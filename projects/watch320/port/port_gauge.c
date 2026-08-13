/*****************************************************************************
 * Module    : 电量计接口 (预留桩)
 * File      : port_gauge.c
 * Function  : TODO: 接电量计IC后实现真实读取(I2C), 当前返回占位值。
 *             不使用SDK的sys_cb.vbat_percent / bsp_charge_sta_get
 *****************************************************************************/
#include "include.h"

//TODO 电量计: 读IC电量寄存器
u8 gauge_percent_get(void)
{
    return 100;                         //占位: 固定100%
}

//TODO 电量计: 读IC充电状态
u8 gauge_charge_sta_get(void)
{
    return GAUGE_CHG_NONE;              //占位: 未充电
}
