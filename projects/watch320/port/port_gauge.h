/*****************************************************************************
 * Module    : 电量计接口 (预留)
 * File      : port_gauge.h
 * Function  : 电量/充电状态统一从这里取, 不用SDK的vbat/charge数据。
 *             后续接电量计IC后在port_gauge.c实现, 显示层无需改动
 *****************************************************************************/
#ifndef _PORT_GAUGE_H
#define _PORT_GAUGE_H

///充电状态 (gauge_charge_sta_get返回值)
enum {
    GAUGE_CHG_NONE = 0,                 //未充电
    GAUGE_CHG_CHARGING,                 //充电中
    GAUGE_CHG_FULL,                     //已充满
};

u8 gauge_percent_get(void);             //电量百分比 0~100 (当前桩: 固定值)
u8 gauge_charge_sta_get(void);          //充电状态 GAUGE_CHG_x (当前桩: 未充电)

#endif // _PORT_GAUGE_H
