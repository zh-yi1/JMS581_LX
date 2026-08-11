#ifndef _JMS581_PROTO_H
#define _JMS581_PROTO_H

// JMS581存储带屏串口协议 协议层: 命令组包/解析/分发/回调
// 协议依据: doc/JMS581存储带屏串口通讯协议 §3~§13 (存储业务0x8000~0x8008)
// 仅API+回调: 不做状态缓存/超时重发, 请求-应答闭环由上层"发送API+记tick+等回调"实现

// 是否打开协议层注入测试代码 (jms581_test_run)
#ifndef JMS581_TEST_EN
#define JMS581_TEST_EN              1       //串口模拟联调中, 出货/联调结束改回0
#endif

///命令组
#define JMS581_CMD_STORAGE          0x80    //存储业务
#define JMS581_CMD_UPGRADE          0x81    //MCU固件升级 (本次未实现, 走unknown回调)

///子命令 (存储业务组)
#define JMS581_SUB_DEV_STATUS       0x00    //获取/主动上报设备状态
#define JMS581_SUB_BACKUP_START     0x01    //开始备份
#define JMS581_SUB_BACKUP_REPORT    0x02    //获取/主动上报备份报告
#define JMS581_SUB_FORMAT           0x03    //开始格式化/格式化进度上报
#define JMS581_SUB_PC_IDLE          0x04    //获取PC端无读写空闲时长
#define JMS581_SUB_BACKUP_CANCEL    0x05    //取消备份
#define JMS581_SUB_CAPACITY         0x06    //获取卡容量
#define JMS581_SUB_FW_VER           0x07    //获取581固件版本号
#define JMS581_SUB_DIR_LIST         0x08    //列出备份目录列表

///协议设备ID (§3.1)
#define JMS581_DEV_CFA              0
#define JMS581_DEV_CFB              1
#define JMS581_DEV_SD               2
#define JMS581_DEV_SATA             3
#define JMS581_DEV_PCIE             4       //PCIe/M.2
#define JMS581_DEV_MAX              4

///dev_list在位bit (§5.3)
#define JMS581_DEVBIT_CFA           BIT(0)
#define JMS581_DEVBIT_CFB           BIT(1)
#define JMS581_DEVBIT_SD            BIT(2)
#define JMS581_DEVBIT_SATA          BIT(3)
#define JMS581_DEVBIT_PCIE          BIT(4)

///备份模式 (§6)
#define JMS581_MODE_FULL            0       //完全备份
#define JMS581_MODE_INC             1       //增量备份
#define JMS581_MODE_RECENT          3       //最新N日备份 (仅长帧, 须携带days 1~31)

///0x8008 根目录类型 (§13.2)
#define JMS581_ROOT_CARD_BACKUP     0
#define JMS581_ROOT_RECENT_BACKUP   1

///错误码 (§18)
#define JMS581_ERR_NONE             0x00
#define JMS581_ERR_NOT_READY        0x01
#define JMS581_ERR_LENGTH           0x02
#define JMS581_ERR_CHECK_SUM        0x03
#define JMS581_ERR_UNSUPPORTED      0x04
#define JMS581_ERR_PARAMETER        0x05
#define JMS581_ERR_NOT_FOUND        0x07
#define JMS581_ERR_DIRECTION        0x08
#define JMS581_ERR_BACKUP_FAILED    0x09
#define JMS581_ERR_FORMAT_FAILED    0x0A
#define JMS581_ERR_BACKUP_CANCELLED 0x0B
#define JMS581_ERR_CANCEL_LAST_BUSY 0x0C
#define JMS581_ERR_OTHER            0xFF

#define JMS581_NAME_LEN_MAX         64      //目录名UTF-16LE最大字节数 (0x8001/l3_name共用上限)
#define JMS581_CURSOR_LEN           6       //0x8008 cursor字节数
#define JMS581_DIR_ENTRY_MAX        16      //0x8008 单页解析条目上限 (请求count自动钳制)

///0x8000 设备状态 (13B应答/主动上报; 11B短ACK时status/dev_list为0)
typedef struct {
    u8 err_code;
    u8 status;                  //0=空闲/充电器/脱机, 4=PC模式, 6=判定中
    u8 dev_list;                //设备在位bit字段, JMS581_DEVBIT_x
} jms581_dev_status_t;

///0x8002 备份报告 (36B=309兼容 / >=39B=315扩展 / 11B短ACK时统计为0)
typedef struct {
    u8  err_code;               //0=成功, 0x09=备份失败, 0x0B=用户取消
    u32 file_done_cnt;          //已处理文件数
    u32 folder_done_cnt;        //已处理文件夹数
    u8  total_unit;             //0=B 1=KB 2=MB 3=GB
    u32 total_size;             //文件总大小
    u8  speed_unit;             //0=B/s 1=KB/s 2=MB/s 3=GB/s
    u32 speed_val;              //速度值
    u32 time_sec;               //耗时, 秒
    u8  sub_err;                //备份引擎SubErrorCode
    u8  src_dev;                //源盘协议ID, 0xFF=未知
    u8  dst_dev;                //目标盘协议ID, 0xFF=未知
    u8  is_ext;                 //1=含315扩展(以下4项才有效)
    u8  phase;                  //0=终态 1=挂载盘 2=扫描文件 3=拷贝
    u8  progress;               //0~100, 仅phase=3有效
    u8  l3_name_len;            //l3_name的UTF-16LE字节数, 0=无
    const u8 *l3_name;          //L3目录名, 指向帧缓冲, 回调返回即失效, 需自行拷贝
} jms581_backup_report_t;

///0x8003 格式化状态 (15B进度/终态上报; 11B启动ACK走format_ack回调)
typedef struct {
    u8 err_code;                //进行中=0; 失败如0x0A
    u8 phase;                   //0=终态 1=进行中
    u8 progress;                //0~100, 仅phase=1有效
    u8 result;                  //仅终态有效: 0=成功 1=失败
    u8 dev_id;                  //正在格式化的设备
} jms581_format_status_t;

///0x8004 PC空闲时长 (13B)
typedef struct {
    u8  err_code;               //0=有效, 1=当前非PC模式
    u16 idle_sec;               //空闲秒数
} jms581_pc_idle_t;


///0x8006 容量 (92B成功包; 11B短ACK时slot_num=0仅err_code有效)
///槽序固定 M.2/CFA/CFB/SD, 与协议设备ID数值序不同, 勿用下标当dev_id
#define JMS581_SLOT_M2              0
#define JMS581_SLOT_CFA             1
#define JMS581_SLOT_CFB             2
#define JMS581_SLOT_SD              3

///0x8006 单槽容量 (20B)
typedef struct {
    u8  present;                //1=在位且容量可查询
    u8  err_code;               //Slot错误码
    u8  total_unit;             //当前固定0=KB
    u8  used_valid;             //1=used_size有效
    u64 total_size;             //总容量, KB
    u64 used_size;              //已用容量, KB
} jms581_slot_cap_t;

typedef struct {
    u8 err_code;                //rsp_err_code整批错误码
    u8 slot_num;                //成功包固定4; 短ACK时0
    jms581_slot_cap_t slot[4];
} jms581_capacity_t;

///0x8007 581固件版本 (15B; 11B短ACK时fw_ver为0)
typedef struct {
    u8 err_code;
    u8 fw_ver[4];               //显示为A.B.C.D
} jms581_fw_ver_t;

///0x8008 目录列表页信息
typedef struct {
    u8 err_code;                //0=成功; 0x05=参数/cursor失效需从首页重拉; 0x07=根目录不存在
    u8 return_count;            //本页实际解析出的条目数
    u8 has_more;                //1=还有下一页
    u8 next_cursor[JMS581_CURSOR_LEN];  //下一页令牌, 581生成, 续页时原样回传
} jms581_dir_list_t;

///0x8008 单条目录项
typedef struct {
    u8  file_type;              //0=目录 (当前仅返回目录)
    u16 name_len;               //目录名UTF-16LE字节数
    const u8 *name;             //目录名, 指向帧缓冲, 回调返回即失效, 需自行拷贝
} jms581_dir_entry_t;

///应答/主动上报回调表: NULL项自动忽略; 回调运行在主循环上下文
typedef struct {
    void (*dev_status)(const jms581_dev_status_t *sta);         //0x8000 应答+主动上报共用
    void (*backup_ack)(u8 err_code);                            //0x8001 启动ACK(0=接受 1=未就绪 5=参数错)
    void (*backup_report)(const jms581_backup_report_t *rpt);   //0x8002 应答+主动上报共用
    void (*format_ack)(u8 err_code);                            //0x8003 11B启动ACK
    void (*format_status)(const jms581_format_status_t *sta);   //0x8003 15B进度/终态(多为主动上报)
    void (*pc_idle)(const jms581_pc_idle_t *idle);              //0x8004
    void (*cancel_ack)(u8 err_code);                            //0x8005 (0=已接受 1=无可取消 0x0C=收尾拒绝)
    void (*capacity)(const jms581_capacity_t *cap);             //0x8006
    void (*fw_ver)(const jms581_fw_ver_t *ver);                 //0x8007
    void (*dir_list)(const jms581_dir_list_t *info,
                     const jms581_dir_entry_t *entries, u8 cnt);//0x8008 整页一次回调
    void (*unknown)(u8 cmd, u8 sub_cmd, const u8 *payload, u16 payload_len);    //未实现命令兜底(0x81xx等)
} jms581_cb_t;

/**
 * @brief 注册应答/上报回调表 (表可为const驻flash; 传NULL注销)
 **/
void jms581_proto_cb_reg(const jms581_cb_t *cb);

/**
 * @brief 帧层唯一上交入口: 解析payload并分发回调 (帧层调用, 上层勿直接调用)
 * @param[in] payload: 帧Byte10起负载
 * @param[in] len: 负载字节数 (=data_len-10)
 **/
void jms581_proto_frame_input(u8 cmd, u8 sub_cmd, const u8 *payload, u16 len);

///请求发送API: 返回1=已交驱动发送, 0=参数非法或驱动错误
u8 jms581_dev_status_req(void);                                 //0x8000 查询设备状态
u8 jms581_backup_start_req(u8 src_dev, u8 dst_dev, u8 mode);    //0x8001 短帧(309兼容, 581自管目录名)
u8 jms581_backup_start_ex_req(u8 src_dev, u8 dst_dev, u8 mode,
        const u8 *name_utf16, u8 name_len, u8 days);            //0x8001 长帧(315): name_len 2~64偶数;
                                                                //mode=3时days必填1~31, 其余mode传0
u8 jms581_backup_start_ascii_req(u8 src_dev, u8 dst_dev, u8 mode,
        const char *name, u8 days);                             //0x8001 长帧便捷包装, 内部ASCII转UTF-16LE
u8 jms581_backup_report_req(void);                              //0x8002 查询备份报告
u8 jms581_format_start_req(u8 dev_id);                          //0x8003 开始格式化
u8 jms581_format_status_req(void);                              //0x8003 被动查询格式化状态
u8 jms581_pc_idle_req(void);                                    //0x8004 查询PC空闲时长
u8 jms581_backup_cancel_req(void);                              //0x8005 取消备份
u8 jms581_capacity_req(void);                                   //0x8006 查询容量
u8 jms581_fw_ver_req(void);                                     //0x8007 查询581固件版本
u8 jms581_dir_list_req(u8 root_type, u8 count, const u8 *cursor);   //0x8008: cursor传NULL=首页;
                                                                //续页原样回传上页next_cursor; count钳到JMS581_DIR_ENTRY_MAX

///UTF-16LE辅助 (目录名编解码, 供上层复用)
/**
 * @brief ASCII转UTF-16LE
 * @return UTF-16LE字节数(=strlen*2, 可直接作name_len); 空串或out_size不足返回0
 **/
u8 jms581_ascii_to_utf16le(const char *ascii, u8 *out, u8 out_size);

/**
 * @brief UTF-16LE转ASCII, 自动补'\0', 码点>0x7F或0填'?'
 * @return ASCII字符数
 **/
u8 jms581_utf16le_to_ascii(const u8 *utf16, u8 utf16_len, char *out, u8 out_size);

#if JMS581_TEST_EN
/**
 * @brief 注入测试: 构造帧经bsp_uart1_isr注入环形缓冲, 穿透全链路, 结果printf输出
 *        开机调用一次; 同时注册printf调试回调表, 之后串口真实收到的帧也会打印
 **/
void jms581_test_run(void);

/**
 * @brief 串口模拟联调轮询: 每3s发一条0x8000查询, 供PC端观察请求帧/回应答
 *        主循环调用, 需先跑过jms581_test_run()注册回调表
 **/
void jms581_test_poll(void);
#endif
#endif
