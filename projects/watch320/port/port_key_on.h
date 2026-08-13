/*****************************************************************************
 * Module    : KEY_ON按键 (USER_KEY_ON按键模式的按键驱动)
 * File      : port_key_on.h
 * Function  : 参数配置见config.h的KEY_ON按键配置, 由bsp_key.c的KEY_ON分支调用
 *             事件上报: 裸机标志位, 不走消息队列; 主循环轮询key_on_event_get()取走
 *****************************************************************************/
#ifndef _PORT_KEY_ON_H
#define _PORT_KEY_ON_H

///按键事件 (key_on_event_get返回值)
enum {
    KEY_ON_EVT_NONE = 0,
    KEY_ON_EVT_CLICK,                       //单击
    KEY_ON_EVT_DOUBLE,                      //双击
    KEY_ON_EVT_LONG,                        //长按(按满即报, 不等抬起)
};

void key_on_init(void);                     //初始化, key_init()调用
void key_on_process(void);                  //状态机, 每KEY_ON_TICK_MS调用一次

/**
 * @brief 取一次按键事件, 取走即清, 无事件返回KEY_ON_EVT_NONE
 *        5ms中断置标志, 主循环轮询消费; 每次只返回一个事件
 **/
u8 key_on_event_get(void);

#endif // _PORT_KEY_ON_H
