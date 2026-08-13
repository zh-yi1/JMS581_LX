/*****************************************************************************
 * Module    : 脱机模式界面 (JMS581流程)
 * File      : func_offline.c
 * Function  : 纯显示: 黑底文字"脱机"; 转移逻辑全在jms581_mode模块大switch
 *****************************************************************************/
#include "include.h"
#include "func.h"

#if JMS581_MODE_EN

compo_form_t *func_offline_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    compo_textbox_t *txt = compo_textbox_create(frm, 8);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_22_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                               GUI_SCREEN_WIDTH, 50);
    compo_textbox_set_forecolor(txt, COLOR_WHITE);
    compo_textbox_set(txt, i18n[STR_JMS_OFFLINE]);
    return frm;
}

void func_offline_process(void)
{
    func_process();
}

void func_offline_enter(void)
{
    printf("%s\n", __func__);
    func_cb.f_cb = func_zalloc(4);          //无私有状态, 占位保持func_exit释放逻辑一致
    func_cb.frm_main = func_offline_form_create();
}

void func_offline_exit(void)
{
    func_cb.last = FUNC_OFFLINE;
}

void func_offline(void)
{
    printf("%s\n", __func__);
    func_offline_enter();
    while (func_cb.sta == FUNC_OFFLINE) {
        func_offline_process();
        msg_dequeue();                      //排空消息队列(主状态机不走消息, 事件在func_process内轮询)
    }
    func_offline_exit();
}

#endif // JMS581_MODE_EN
