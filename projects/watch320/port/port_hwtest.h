/*****************************************************************************
 * Module    : 硬件测试临时代码
 * File      : port_hwtest.h
 * Function  : 电源脚 / VBUS_DET / VBUS_OUT 开关机时序验证, 验证完直接删掉本文件
 *****************************************************************************/
#ifndef _PORT_HWTEST_H
#define _PORT_HWTEST_H

#define HWTEST_EN                   1       //硬件测试代码总开关, 0为不编译

void hwtest_early_init(void);               //最早期初始化: 只拉高系统供电PB12, main()第一行调用
void hwtest_init(void);                     //首次开机时序: VBUS_DET/VBUS_OUT同步 + 延时拉高PB11
void hwtest_process(void);                  //主循环调用: VBUS_OUT跟随VBUS_DET, 处理长按开关机请求
void hwtest_key_long_req(void);             //长按事件, 在5ms中断里调用, 只置标志

#endif // _PORT_HWTEST_H
