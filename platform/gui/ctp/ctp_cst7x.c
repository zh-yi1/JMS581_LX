#include "include.h"

#if (CTP_SELECT == CTP_CST7X)

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * Hynitron CST7XX 系列 (CST726 / CST826 / CST836U)
 * 协议参考厂家驱动 hyn_driver_release_mtk/hyn_chips/hyn_cst7xx.c
 * IIC地址: 主固件 TP_IIC_ADDR(0x15)  BOOT TP_IIC_UPDATE_ADDR(0x6A)
 */

//CTP升级相关
#define CST7X_UPDATE_EN          0                  //打开TP升级功能

#define BIN_SIZE                 (15 * 1024)        //固件长度(不含6字节文件头)
#define PER_LEN                  512                //FLASH单页长度
#define TANS_LIMT                6                  //单次IIC写的数据字节数, 受ctp_iic_update_write内部缓冲限制, 最大6

//上报数据: 0x00[手势] 0x01[保留] 0x02[手指数] 0x03~0x06[P1: XH XL YH YL]
#define CTP_READ_ADDR            0x00
#define CTP_READ_SIZE            7

//芯片信息: 0xA6[fw_ver] 0xA7 0xA8[module_id] 0xA9[project_id] 0xAA[ic_type] 0xAB
#define CTP_INFO_ADDR            0xA6
#define CTP_INFO_SIZE            6

//命令: 高字节为寄存器地址, 低字节为数据
#define CMD_NORMAL_MODE          0xFE00             //正常工作模式
#define CMD_DEEPSLEEP            0xA503             //深度睡眠

static u8 ctp_cst7x_buf[CTP_READ_SIZE];

bool ctp_iic_update_write(u8 dev_addr, u16 addr, u8 *cmd, int len);
bool ctp_iic_update_read(u8 dev_addr, void *rbuf, int rlen, u16 w_addr, u8 *w_cmd, int wlen);
void ctp_reset(void);

//发送单字节命令(reg = cmd高字节, data = cmd低字节)
static bool ctp_cst7x_write_cmd(u16 cmd)
{
    return ctp_iic_update_write(TP_IIC_ADDR, cmd, NULL, 1);
}

#if CST7X_UPDATE_EN

extern i2c_t *CTP_IIC;

static bool cst7x_boot_pass;

//BOOT复位: 拉低10ms后释放, 再等待delay_time毫秒的BOOT窗口
static void cst7x_rst(u8 delay_time)
{
    PORT_CTP_RST_L();
    delay_5ms(2);
    PORT_CTP_RST_H();
    delay_ms(delay_time);
}

//退出BOOT模式
static void cst7x_exit_boot(void)
{
    u8 cmd = 0xEE;
    ctp_iic_update_write(TP_IIC_UPDATE_ADDR, 0xA006, &cmd, 2);      //A0 06 EE
}

//进入BOOT模式, BOOT窗口只有复位后的几毫秒, 需要逐档试探
static bool cst7x_enter_boot(void)
{
    u8 t;

    for (t = 5; t < 15; t += 2) {
        u8 cmd = 0xAA;
        u8 res = 0;

        cst7x_rst(t);
        WDT_CLR();

        if (!ctp_iic_update_write(TP_IIC_UPDATE_ADDR, 0xA001, &cmd, 2)) {   //A0 01 AA
            continue;
        }
        if (!ctp_iic_update_read(TP_IIC_UPDATE_ADDR, &res, 1, 0xA003, NULL, 1)) {
            continue;
        }
        if (res != 0x55) {
            continue;
        }
        return true;
    }

    TRACE("%s fail!\n", __func__);
    return false;
}

//读固件checksum, 同时判断BOOT是否正常(空片时boot_pass为0)
static u16 cst7x_read_checksum(void)
{
    u8 buf[2];
    u8 cmd;
    int retry, time_out;

    cst7x_boot_pass = false;

    for (retry = 3; retry > 0; retry--) {
        cmd = 0x00;
        if (!ctp_iic_update_write(TP_IIC_UPDATE_ADDR, 0xA003, &cmd, 2)) {   //A0 03 00, 启动校验
            continue;
        }
        delay_ms(100);

        for (time_out = 100; time_out > 0; time_out--) {
            delay_ms(10);
            WDT_CLR();
            buf[0] = 0;
            ctp_iic_update_read(TP_IIC_UPDATE_ADDR, buf, 1, 0xA000, NULL, 1);
            if (buf[0] == 0x01) {
                cst7x_boot_pass = true;
                break;
            }
        }
        if (!cst7x_boot_pass) {
            continue;
        }

        if (ctp_iic_update_read(TP_IIC_UPDATE_ADDR, buf, 2, 0xA008, NULL, 1)) {
            return ((u16)buf[1] << 8) | buf[0];
        }
    }
    return 0;
}

//写一页数据到芯片RAM(0xA018起), 单次IIC长度受限, 分包写且寄存器地址自增
static bool cst7x_write_page(u8 *src, u16 len)
{
    u16 reg = 0xA018;

    while (len) {
        u16 step = (len > TANS_LIMT) ? TANS_LIMT : len;

        if (!ctp_iic_update_write(TP_IIC_UPDATE_ADDR, reg, src, step + 1)) {
            return false;
        }
        reg += step;
        src += step;
        len -= step;
        WDT_CLR();
    }
    return true;
}

//逐页烧写, 进入时必须已经在BOOT模式
static bool cst7x_update_do(u8 *src, u32 len)
{
    u32 offset;
    u8 cmd[2];
    bool ret = false;

    TRACE("CTP update start\n");

    for (offset = 0; offset < len; offset += PER_LEN) {
        int retry;

        //1. 指定FLASH页地址
        cmd[0] = offset & 0xFF;
        cmd[1] = (offset >> 8) & 0xFF;
        if (!ctp_iic_update_write(TP_IIC_UPDATE_ADDR, 0xA014, cmd, 3)) {    //A0 14 lo hi
            goto update_end;
        }

        //2. 整页数据搬到芯片RAM
        if (!cst7x_write_page(&src[offset], PER_LEN)) {
            goto update_end;
        }

        //3. 启动烧写
        cmd[0] = 0xEE;
        if (!ctp_iic_update_write(TP_IIC_UPDATE_ADDR, 0xA004, cmd, 2)) {    //A0 04 EE
            goto update_end;
        }
        delay_ms(100);

        //4. 等待烧写完成
        for (retry = 50; retry > 0; retry--) {
            u8 res = 0;
            delay_ms(5);
            WDT_CLR();
            ctp_iic_update_read(TP_IIC_UPDATE_ADDR, &res, 1, 0xA005, NULL, 1);
            if (res == 0x55) {
                break;
            }
        }
        if (retry == 0) {
            goto update_end;
        }

        //预留升级百分比方便UI做显示
        TRACE("%d%%\n", (int)((offset + PER_LEN) * 100 / len));
    }
    ret = true;

update_end:
    cst7x_exit_boot();
    return ret;
}

bool ctp_cst7x_update(void)
{
    u8 *update_ptr = (u8 *)(RES_BUF_CTP_CTP_UPDATE_BIN);
    u32 update_len = RES_LEN_CTP_CTP_UPDATE_BIN;
    u16 bin_len, bin_checksum, ic_checksum;

    //文件头6字节: [0:2]起始地址 [2:4]数据长度 [4:6]checksum, 均为小端
    if (update_len <= 6) {
        TRACE("CTP no update bin\n");
        return false;
    }
    bin_len      = ((u16)update_ptr[3] << 8) | update_ptr[2];
    bin_checksum = ((u16)update_ptr[5] << 8) | update_ptr[4];

    if (bin_len != BIN_SIZE || update_len < (u32)bin_len + 6) {
        TRACE("CTP bin len err bin:%x res:%x\n", bin_len, (unsigned)update_len);
        return false;
    }

    if (!cst7x_enter_boot()) {
        return false;
    }

    ic_checksum = cst7x_read_checksum();
    TRACE("CTP checksum ic:%04x bin:%04x boot_pass:%d\n", ic_checksum, bin_checksum, cst7x_boot_pass);

    if (cst7x_boot_pass && ic_checksum == bin_checksum) {
        TRACE("CTP fw is already the latest version!\n");
        cst7x_exit_boot();
        return true;
    }

    //空片时读checksum会失败, 重新进BOOT后直接烧
    if (!cst7x_enter_boot()) {
        return false;
    }
    if (!cst7x_update_do(&update_ptr[6], BIN_SIZE)) {
        TRACE("CTP update do fail\n");
        return false;
    }

    if (!cst7x_enter_boot()) {
        return false;
    }
    ic_checksum = cst7x_read_checksum();
    cst7x_exit_boot();

    if (ic_checksum != bin_checksum) {
        TRACE("CTP update checksum err:%04x correct:%04x\n", ic_checksum, bin_checksum);
        return false;
    }
    TRACE("CTP update success!\n");
    return true;
}

void ctp_cst7x_update_check(void)
{
    u32 iic_con0 = CTP_IIC->sfr->IICxCON0;
    u32 iic_con1 = CTP_IIC->sfr->IICxCON1;

    //50KHZ 低速配置IIC模块
    CTP_IIC->sfr->IICxCON0 = BIT(10) | (63 << 4) | BIT(0) | (0 << 2);       //WSCL_OPT, POSDIV, IIC EN

    if (!ctp_cst7x_update()) {
        TRACE("CTP update failed!\n");
    }

    CTP_IIC->sfr->IICxCON0 = iic_con0;
    CTP_IIC->sfr->IICxCON1 = iic_con1;

    ctp_reset();
}

#endif //CST7X_UPDATE_EN

bool ctp_cst7x_init(void)
{
    u8 info[CTP_INFO_SIZE];

#if CST7X_UPDATE_EN
    //Only check once after power on
    static bool check_flag = false;
    if (!check_flag) {
        ctp_cst7x_update_check();
        check_flag = true;
    }
#endif

    if (!ctp_iic_read(info, CTP_INFO_ADDR, CTP_INFO_SIZE)) {
        TRACE("CTP IIC TIMEOUT!\n");
        return false;
    }

    TRACE("CTP A6: %02x %02x %02x %02x %02x %02x\n",
          info[0], info[1], info[2], info[3], info[4], info[5]);
    TRACE("CTP VER: %02x, PROJ: %02x, ICTYPE: %02x\n", info[0], info[3], info[4]);

    //IIC无应答时总线维持空闲电平, 读回全FF; SDA被拉死则读回全00
    if ((info[0] == 0xFF && info[3] == 0xFF && info[4] == 0xFF) ||
        (info[0] == 0x00 && info[3] == 0x00 && info[4] == 0x00)) {
        TRACE("CTP NO ACK!\n");
        return false;
    }

    ctp_cst7x_write_cmd(CMD_NORMAL_MODE);
    return true;
}

AT(.com_text.ctp)
void ctp_cst7x_readkick(void)
{
    ctp_iic_readkick(ctp_cst7x_buf, CTP_READ_ADDR, CTP_READ_SIZE);
}

AT(.com_text.ctp)
bool ctp_cst7x_get_point(s32 *x, s32 *y)
{
    u8 *buf = ctp_cst7x_buf;
    u8 finger = buf[2];
    u8 event = buf[3] & 0xC0;               //0x00按下 0x40抬起 0x80接触 0xC0无效

    *x = ((s32)(buf[3] & 0x0F) << 8) | buf[4];
    *y = ((s32)(buf[5] & 0x0F) << 8) | buf[6];

    if (finger == 0 || finger > 2) {        //手指数非法, 丢弃该帧
        return false;
    }
    return (event != 0x40 && event != 0xC0);
}

#endif //CTP_SELECT == CTP_CST7X
