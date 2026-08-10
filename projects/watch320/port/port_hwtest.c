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

static bool hwtest_on;                              //true: 开机状态
static bool hwtest_key_req;                         //长按请求, 中断置位, 主循环清

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
    LCD_BL_DIS();                                                   //关屏幕背光
    port_gpio_out_level(HWTEST_VBUS_OUT_IO, 0);                     //拉低VBUS_OUT
    port_gpio_out_level(HWTEST_SUB_PWR_IO, 0);                      //拉低副芯片供电
    hwtest_on = false;
    printf("hwtest: power off\n");
}

AT(.text.hwtest)
static void hwtest_power_on(void)
{
    LCD_BL_EN();                                                    //开屏幕背光
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
    }
}

#endif // HWTEST_EN
