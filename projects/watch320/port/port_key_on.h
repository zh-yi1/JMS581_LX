/*****************************************************************************
 * Module    : KEY_ON按键 (USER_KEY_ON按键模式的按键驱动)
 * File      : port_key_on.h
 * Function  : 参数配置见config.h的KEY_ON按键配置, 由bsp_key.c的KEY_ON分支调用
 *****************************************************************************/
#ifndef _PORT_KEY_ON_H
#define _PORT_KEY_ON_H

void key_on_init(void);                     //初始化, key_init()调用
void key_on_process(void);                  //状态机, 每KEY_ON_TICK_MS调用一次

#endif // _PORT_KEY_ON_H
