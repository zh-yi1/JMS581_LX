/*****************************************************************************
 * Module    : 硬件测试临时代码
 * File      : port_hwtest.c
 * Function  : 验证电源脚和VBUS通路的开关机时序, 状态只放在RAM里, 不写flash,
 *             掉电再上电即视为首次开机。验证完直接删掉本文件和它的调用点。
 *
 * 引脚:
 *   PB12  系统供电,     芯片一启动就拉高, 全程保持
 *   PE7   VBUS_DET,     输入, 内部下拉, USB插入后被外部分压拉高
 *   PE4   VBUS_OUT,     输出, 开机状态下实时跟随VBUS_DET
 *   PB11  副芯片供电,   拉高开供电; 副芯片上电后会去读VBUS_OUT, 所以要先给
 *                       VBUS_OUT置好电平, 等30ms再拉高PB11
 *
 * 时序:
 *   1. 首次开机(USB供电): 拉高PB12 -> 读VBUS_DET同步输出VBUS_OUT -> 等30ms -> 拉高PB11
 *   2. 长按关机: 关背光 -> 拉低VBUS_OUT -> 拉低PB11
 *   3. 长按开机: 开背光 -> VBUS_OUT同步VBUS_DET -> 等30ms -> 拉高PB11
 *****************************************************************************/
#include "include.h"
#include "port_hwtest.h"

#if HWTEST_EN

#define HWTEST_SYS_PWR_IO           IO_PB12         //系统供电
#define HWTEST_SUB_PWR_IO           IO_PB11         //副芯片供电
#define HWTEST_VBUS_DET_IO          IO_PE7          //USB插入检测输入
#define HWTEST_VBUS_OUT_IO          IO_PE4          //VBUS_OUT输出
#define HWTEST_SUB_PWR_DELAY_MS     30              //VBUS_OUT稳定到拉高副芯片供电的间隔

//背光控制方式: HWTEST_BL_IO 填实际背光IO则直接用GPIO开关, 填IO_NONE则用
//config.h那套 power gate + PWM(PORT_TFT_BL = PG_BL_TMR4)
#define HWTEST_BL_IO                IO_NONE         //背光IO, 按原理图填, 例: IO_PA14
#define HWTEST_BL_IO_ON_LEVEL       1               //背光IO点亮电平, 0低有效 1高有效

static bool hwtest_on;                              //true: 开机状态
static bool hwtest_key_req;                         //长按请求, 中断置位, 主循环清
static u8   hwtest_bl_duty;                         //关机前保存的背光占空比

//GUI每刷一帧, TE中断会置 tft_bglight_kick / te_bglight_cnt / tft_bglight_first_set,
//主循环的 tft_bglight_frist_set_check() 看到 tft_bglight_duty==0 就当成"还没初始化",
//直接改成 GUI_DEFAULT_BK 再点亮(tft.c:157), 所以这几个恢复标志要一起清掉
AT(.text.hwtest)
static void hwtest_bl_off(void)
{
    tft_cb_t *tft_get_tft_cb(void);
    tft_cb_t *cb = tft_get_tft_cb();

    cb->tft_bglight_first_set = false;                              //堵住GUI刷新后的背光恢复
    cb->tft_bglight_kick      = false;
    cb->te_bglight_cnt        = 0;

#if (HWTEST_BL_IO != IO_NONE)
    port_gpio_set_out(HWTEST_BL_IO, !HWTEST_BL_IO_ON_LEVEL);        //背光IO直接关
#else
    LCD_BL_DIS();                                                   //关power gate
    lcd_drv_set_brightness(0);                                      //走驱动层压占空比
    bsp_pwm_duty_set(PORT_TFT_BL, 0, false);                        //绕过last_duty判断直接写PWM寄存器
#endif
}

AT(.text.hwtest)
static void hwtest_bl_on(u8 duty)
{
#if (HWTEST_BL_IO != IO_NONE)
    port_gpio_set_out(HWTEST_BL_IO, HWTEST_BL_IO_ON_LEVEL);
    (void)duty;
#else
    tft_cb_t *tft_get_tft_cb(void);

    LCD_BL_EN();
    tft_get_tft_cb()->tft_bglight_last_duty = 0;                    //强制下一次真正写PWM
    lcd_drv_set_brightness(duty);
    bsp_pwm_duty_set(PORT_TFT_BL, duty, false);
#endif
}

AT(.text.hwtest)
static u8 hwtest_bl_get(void)
{
    tft_cb_t *tft_get_tft_cb(void);
    u8 duty = tft_get_tft_cb()->tft_bglight_duty;

    return duty ? duty : GUI_DEFAULT_BK;
}

//最早期初始化, 只做系统供电, main()第一行调用
AT(.text.hwtest)
void hwtest_early_init(void)
{
    port_gpio_set_out(HWTEST_SYS_PWR_IO, 1);
}

//首次开机时序, bsp_sys_init()之后调用(避免IO配置被系统初始化覆盖)
AT(.text.hwtest)
void hwtest_init(void)
{
    port_gpio_set_out(HWTEST_SYS_PWR_IO, 1);                        //保险起见再拉一次
    port_gpio_set_in(HWTEST_VBUS_DET_IO, GPIOxPD);                  //VBUS_DET输入, 内部下拉
    port_gpio_set_out(HWTEST_VBUS_OUT_IO, bsp_gpio_get_sta(HWTEST_VBUS_DET_IO));
    port_gpio_set_out(HWTEST_SUB_PWR_IO, 0);
    delay_ms(HWTEST_SUB_PWR_DELAY_MS);
    port_gpio_out_level(HWTEST_SUB_PWR_IO, 1);
    hwtest_on   = true;
    hwtest_key_req = false;
    printf("hwtest: boot, vbus_det=%d\n", bsp_gpio_get_sta(HWTEST_VBUS_DET_IO));
}

//长按事件, 5ms中断里调用, 只置标志, 时序放主循环做
AT(.com_text.hwtest)
void hwtest_key_long_req(void)
{
    hwtest_key_req = true;
}

AT(.text.hwtest)
static void hwtest_power_off(void)
{
    hwtest_bl_duty = hwtest_bl_get();                               //保存当前亮度, 开机时恢复
    hwtest_bl_off();                                                //关屏幕背光
    port_gpio_out_level(HWTEST_VBUS_OUT_IO, 0);                     //拉低VBUS_OUT
    port_gpio_out_level(HWTEST_SUB_PWR_IO, 0);                      //拉低副芯片供电
    hwtest_on = false;
    printf("hwtest: power off, save bl duty=%d\n", hwtest_bl_duty);
}

AT(.text.hwtest)
static void hwtest_power_on(void)
{
    hwtest_bl_on(hwtest_bl_duty ? hwtest_bl_duty : GUI_DEFAULT_BK); //开屏幕背光
    port_gpio_out_level(HWTEST_VBUS_OUT_IO, bsp_gpio_get_sta(HWTEST_VBUS_DET_IO));
    delay_ms(HWTEST_SUB_PWR_DELAY_MS);
    port_gpio_out_level(HWTEST_SUB_PWR_IO, 1);                      //拉高副芯片供电
    hwtest_on = true;
    printf("hwtest: power on, vbus_det=%d\n", bsp_gpio_get_sta(HWTEST_VBUS_DET_IO));
}

//主循环调用
AT(.text.hwtest)
void hwtest_process(void)
{
    if (hwtest_key_req) {
        hwtest_key_req = false;
        if (hwtest_on) {
            hwtest_power_off();
        } else {
            hwtest_power_on();
        }
    }

    if (hwtest_on) {                                                //开机状态下VBUS_OUT实时跟随VBUS_DET
        port_gpio_out_level(HWTEST_VBUS_OUT_IO, bsp_gpio_get_sta(HWTEST_VBUS_DET_IO));
    } else {
        hwtest_bl_off();                                            //TE中断会重新置起恢复标志, 关机态每轮都压一次
    }
}

#endif // HWTEST_EN
