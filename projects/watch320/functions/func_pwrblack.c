/*****************************************************************************
 * Module    : 关机模式界面 (JMS581流程, 黑屏待机)
 * File      : func_pwrblack.c
 * Function  : 纯显示: 纯黑界面 + 短按临时显示电量3秒(请求来自主状态机bat_show标志)
 *             按键/USB事件由主状态机轮询采集, 本任务不处理
 *****************************************************************************/
#include "include.h"
#include "func.h"

#if JMS581_MODE_EN

#define PWRBLACK_BAT_SHOW_MS        3000    //短按电量显示时长

typedef struct f_pwrblack_t_ {
    u8  show_bat;                           //电量文字显示中
    u32 show_tick;                          //电量显示计时
} f_pwrblack_t;

enum {
    COMPO_ID_PWRBLACK_BAT = 1,              //电量文字
};

//纯黑窗体 + 隐藏的电量文字(短按时临时显示)
compo_form_t *func_pwrblack_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_22_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                               GUI_SCREEN_WIDTH, 50);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set_visible(txt, false);
    compo_setid(txt, COMPO_ID_PWRBLACK_BAT);
    return frm;
}

//显示"电量% 充电状态" (数据源: port_gauge电量计接口, 不用SDK vbat/charge)
static void func_pwrblack_bat_show(void)
{
    f_pwrblack_t *f = (f_pwrblack_t *)func_cb.f_cb;
    compo_textbox_t *txt = compo_getobj_byid(COMPO_ID_PWRBLACK_BAT);
    char buf[32];
    u8 chg = gauge_charge_sta_get();
    const char *chg_str = (chg == GAUGE_CHG_CHARGING) ? i18n[STR_JMS_CHG_ING] :
                          (chg == GAUGE_CHG_FULL) ? i18n[STR_JMS_CHG_FULL] : i18n[STR_JMS_CHG_NONE];
    sprintf(buf, "%d%% %s", gauge_percent_get(), chg_str);
    compo_textbox_set(txt, buf);
    compo_textbox_set_visible(txt, true);
    f->show_bat  = 1;
    f->show_tick = tick_get();
}

void func_pwrblack_process(void)
{
    f_pwrblack_t *f = (f_pwrblack_t *)func_cb.f_cb;
    func_process();

    if (jms581_mode_bat_show_take()) {      //主状态机收到短按, UI取走请求做显示
        func_pwrblack_bat_show();
    }
    if (f->show_bat && tick_check_expire(f->show_tick, PWRBLACK_BAT_SHOW_MS)) {
        f->show_bat = 0;
        compo_textbox_set_visible(compo_getobj_byid(COMPO_ID_PWRBLACK_BAT), false);
    }
}

void func_pwrblack_enter(void)
{
    printf("%s\n", __func__);
    func_cb.f_cb = func_zalloc(sizeof(f_pwrblack_t));
    func_cb.frm_main = func_pwrblack_form_create();
}

void func_pwrblack_exit(void)
{
    func_cb.last = FUNC_PWRBLACK;
}

void func_pwrblack(void)
{
    printf("%s\n", __func__);
    func_pwrblack_enter();
    while (func_cb.sta == FUNC_PWRBLACK) {
        func_pwrblack_process();
        msg_dequeue();                      //排空消息队列(主状态机不走消息, 事件在func_process内轮询)
    }
    func_pwrblack_exit();
}

#endif // JMS581_MODE_EN
