/*****************************************************************************
 * Module    : 充电模式界面 (JMS581流程)
 * File      : func_jmscharge.c
 * Function  : 纯显示: 黑底文字"充电模式"+电量%+充电状态, 1秒刷新;
 *             转移逻辑全在jms581_mode模块大switch
 *****************************************************************************/
#include "include.h"
#include "func.h"

#if JMS581_MODE_EN

#define JMSCHARGE_REFRESH_MS        1000    //电量文字刷新周期

typedef struct f_jmscharge_t_ {
    u32 refresh_tick;                       //刷新计时
} f_jmscharge_t;

enum {
    COMPO_ID_JMSCHARGE_BAT = 1,             //电量文字
};

//电量+充电状态文字 (数据源: port_gauge电量计接口, 不用SDK vbat/charge)
static void func_jmscharge_bat_update(void)
{
    compo_textbox_t *txt = compo_getobj_byid(COMPO_ID_JMSCHARGE_BAT);
    char buf[32];
    u8 chg = gauge_charge_sta_get();
    const char *chg_str = (chg == GAUGE_CHG_CHARGING) ? i18n[STR_JMS_CHG_ING] :
                          (chg == GAUGE_CHG_FULL) ? i18n[STR_JMS_CHG_FULL] : i18n[STR_JMS_CHG_NONE];
    sprintf(buf, "%d%% %s", gauge_percent_get(), chg_str);
    compo_textbox_set(txt, buf);
}

compo_form_t *func_jmscharge_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    compo_textbox_t *txt = compo_textbox_create(frm, 12);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_22_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 30,
                               GUI_SCREEN_WIDTH, 50);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, i18n[STR_JMS_CHARGE_MODE]);

    compo_textbox_t *bat = compo_textbox_create(frm, 16);
    compo_textbox_set_font(bat, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set_location(bat, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 30,
                               GUI_SCREEN_WIDTH, 40);
    compo_textbox_set_forecolor(bat, COLOR_WHITE);
    compo_setid(bat, COMPO_ID_JMSCHARGE_BAT);
    return frm;
}

void func_jmscharge_process(void)
{
    f_jmscharge_t *f = (f_jmscharge_t *)func_cb.f_cb;
    func_process();

    if (tick_check_expire(f->refresh_tick, JMSCHARGE_REFRESH_MS)) {
        f->refresh_tick = tick_get();
        func_jmscharge_bat_update();
    }
}

void func_jmscharge_enter(void)
{
    printf("%s\n", __func__);
    func_cb.f_cb = func_zalloc(sizeof(f_jmscharge_t));
    func_cb.frm_main = func_jmscharge_form_create();
    func_jmscharge_bat_update();
}

void func_jmscharge_exit(void)
{
    func_cb.last = FUNC_JMSCHARGE;
}

void func_jmscharge(void)
{
    printf("%s\n", __func__);
    func_jmscharge_enter();
    while (func_cb.sta == FUNC_JMSCHARGE) {
        func_jmscharge_process();
        msg_dequeue();                      //排空消息队列(主状态机不走消息, 事件在func_process内轮询)
    }
    func_jmscharge_exit();
}

#endif // JMS581_MODE_EN
