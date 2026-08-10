/*****************************************************************************
 * Module    : KEY_ON按键 (USER_KEY_ON按键模式的按键驱动)
 * File      : port_key_on.c
 * Function  : 消抖 + 单击/双击/长按, 状态机识别, 识别到就调对应回调
 *
 * 状态转移:
 *   WAIT_RELEASE --抬起--> IDLE --按下--> PRESS_FILTER --消抖OK--> PRESSED
 *                                              |抖动                  |
 *                                              |                      |按满3秒: 长按回调
 *                                              v                      v
 *                  IDLE <--窗口超时: 单击-- MULTI_WAIT               LONG
 *                                              ^                      |
 *                                              |第1次抬起              |抬起
 *                             RELEASE_FILTER --+                      |
 *                                   ^          |第2次抬起: 双击        |
 *                                   +----------+-- 抬起 --------------+--> IDLE
 *
 * 说明: 长按按满KEY_ON_LONG_MS当场回调, 不等抬起, 之后这一次按下不再产生单击/双击;
 *       key_init()调用key_on_init()初始化, 状态从WAIT_RELEASE开始,
 *       上电时按键还按着的话先等抬起, 不会被当成一次新按键。
 *****************************************************************************/
#include "include.h"
#include "port_hwtest.h"

#if USER_KEY_ON

//本文件调试打印开关: 1打开, 0关闭。打印在5ms中断里执行, 调完记得关掉
#define KEY_ON_DEBUG_EN         1

#if KEY_ON_DEBUG_EN
#define key_on_printf(...)      printf(__VA_ARGS__)
#else
#define key_on_printf(...)
#endif

/*----------------------------------------------------------------------------
 * 按键回调, 按键动作填在这里。运行在5ms定时中断上下文, 只做轻量操作。
 *--------------------------------------------------------------------------*/
AT(.com_text.port.key)
static void key_on_click_cb(void)                  //单击
{
    key_on_printf("key_on: click\n");
}

AT(.com_text.port.key)
static void key_on_double_cb(void)                 //双击
{
    key_on_printf("key_on: double\n");
}

AT(.com_text.port.key)
static void key_on_long_cb(void)                   //长按
{
    key_on_printf("key_on: long\n");
#if HWTEST_EN
    hwtest_key_long_req();                         //硬件测试: 长按开关机, 时序在主循环做
#endif
}

/*--------------------------------------------------------------------------*/

#define KEY_ON_MS2CNT(ms)       ((u16)((ms) / KEY_ON_TICK_MS))

enum {
    KEY_ON_WAIT_RELEASE = 0,    //等待抬起(bss清零后的默认状态)
    KEY_ON_IDLE,                //空闲, 等待按下
    KEY_ON_PRESS_FILTER,        //按下消抖
    KEY_ON_PRESSED,             //已按下, 等待长按/抬起
    KEY_ON_LONG,                //长按已上报, 等待抬起
    KEY_ON_RELEASE_FILTER,      //抬起消抖
    KEY_ON_MULTI_WAIT,          //双击窗口, 等待第二次按下
};

typedef struct {
    u8  sta;                    //状态机状态
    u8  filter_cnt;             //消抖计数
    u8  click_cnt;              //已完成的短按次数
    u8  long_flag;              //本次按下已判定为长按
    u8  pressed;                //消抖后的按下状态
    u16 timer;                  //按下时长 / 双击窗口计时
} key_on_cb_t;

static key_on_cb_t key_on_cb;

AT(.text.key.init)
void key_on_init(void)
{
    RTCCON1 |= BIT(4) | BIT(0);                                     //WKO脚使能, 只用数字电平
    memset(&key_on_cb, 0, sizeof(key_on_cb));
    key_on_cb.sta = KEY_ON_WAIT_RELEASE;
    key_on_printf("key_on: init, level=%d\n", KEY_ON_IS_PRESS());
}

///状态机, 每KEY_ON_TICK_MS调用一次
AT(.com_text.port.key)
void key_on_process(void)
{
    bool level = KEY_ON_IS_PRESS();                                 //true: 按键按下(低电平)

    switch (key_on_cb.sta) {
    case KEY_ON_WAIT_RELEASE:
        if (level) {
            key_on_cb.filter_cnt = 0;                               //一直按着, 消抖计数清零
            break;
        }
        if (++key_on_cb.filter_cnt >= KEY_ON_MS2CNT(KEY_ON_FILTER_MS)) {
            key_on_cb.filter_cnt = 0;
            key_on_cb.pressed = 0;
            key_on_cb.sta = KEY_ON_IDLE;
            key_on_printf("key_on: idle\n");
        }
        break;

    case KEY_ON_IDLE:
        if (level) {
            key_on_cb.filter_cnt = 0;
            key_on_cb.sta = KEY_ON_PRESS_FILTER;
        }
        break;

    case KEY_ON_PRESS_FILTER:
        if (!level) {                                               //抖动, 返回原状态, 双击窗口继续计时
            key_on_cb.sta = key_on_cb.click_cnt ? KEY_ON_MULTI_WAIT : KEY_ON_IDLE;
            break;
        }
        if (++key_on_cb.filter_cnt >= KEY_ON_MS2CNT(KEY_ON_FILTER_MS)) {
            key_on_cb.pressed   = 1;
            key_on_cb.long_flag = 0;
            key_on_cb.timer     = 0;
            key_on_cb.sta       = KEY_ON_PRESSED;
            key_on_printf("key_on: press, click_cnt=%d\n", key_on_cb.click_cnt);
        }
        break;

    case KEY_ON_PRESSED:
        if (!level) {
            key_on_cb.filter_cnt = 0;
            key_on_cb.sta = KEY_ON_RELEASE_FILTER;
            break;
        }
        if (++key_on_cb.timer >= KEY_ON_MS2CNT(KEY_ON_LONG_MS)) {   //按满即长按, 不等抬起
            key_on_cb.long_flag = 1;
            key_on_cb.click_cnt = 0;                                //长按不参与单击/双击
            key_on_cb.sta       = KEY_ON_LONG;
            key_on_long_cb();
        }
        break;

    case KEY_ON_LONG:
        if (!level) {
            key_on_cb.filter_cnt = 0;
            key_on_cb.sta = KEY_ON_RELEASE_FILTER;
        }
        break;

    case KEY_ON_RELEASE_FILTER:
        if (level) {                                                //抖动, 恢复按下状态
            key_on_cb.sta = key_on_cb.long_flag ? KEY_ON_LONG : KEY_ON_PRESSED;
            break;
        }
        if (++key_on_cb.filter_cnt >= KEY_ON_MS2CNT(KEY_ON_FILTER_MS)) {
            key_on_cb.pressed = 0;
            key_on_printf("key_on: release, long=%d\n", key_on_cb.long_flag);
            if (key_on_cb.long_flag) {                              //长按抬起, 无回调
                key_on_cb.long_flag = 0;
                key_on_cb.click_cnt = 0;
                key_on_cb.sta = KEY_ON_IDLE;
            } else if (++key_on_cb.click_cnt >= 2) {                //第二次抬起: 双击
                key_on_cb.click_cnt = 0;
                key_on_cb.sta = KEY_ON_IDLE;
                key_on_double_cb();
            } else {
                key_on_cb.timer = 0;
                key_on_cb.sta = KEY_ON_MULTI_WAIT;
            }
        }
        break;

    case KEY_ON_MULTI_WAIT:
        if (level) {
            key_on_cb.filter_cnt = 0;
            key_on_cb.sta = KEY_ON_PRESS_FILTER;
            break;
        }
        if (++key_on_cb.timer >= KEY_ON_MS2CNT(KEY_ON_MULTI_MS)) {  //窗口结束, 没有第二击: 单击
            key_on_cb.click_cnt = 0;
            key_on_cb.sta = KEY_ON_IDLE;
            key_on_click_cb();
        }
        break;

    default:
        key_on_cb.sta = KEY_ON_IDLE;
        break;
    }
}

#endif // USER_KEY_ON
