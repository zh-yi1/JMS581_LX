#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

compo_form_t *func_home_page_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    /* 黑色背景，避免 GPU 误读 UI 头当图解码 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    txt = compo_textbox_create(frm, 4);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y-150, 200, 40);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set(txt, i18n[STR_HOME]);

    /* 上方: 设置图标 */
    compo_picturebox_t *pic_set = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_SET_BIN);
    compo_picturebox_set_pos(pic_set, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 50);

    /* 下方: 备份模式图标 */
    compo_picturebox_t *pic_backup = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACKUP_MODE_BIN);
    compo_picturebox_set_pos(pic_backup, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 50);

    return frm;
}

static void func_home_page_process(void)
{
    func_process();
}

static void func_home_page_message(size_msg_t msg)
{
    switch (msg)
    {
    default:
        func_message(msg);
        break;
    }
}

void func_home_page_enter(void)
{
    func_cb.frm_main = func_home_page_form_create();
}

void func_home_page_exit(void)
{
    func_cb.last = FUNC_CLOCK;
}

void func_home_page(void)
{
    printf("%s\n", __func__);
    func_home_page_enter();
    while (func_cb.sta == FUNC_HOME_PAGE)
    {
        func_home_page_process();
        func_home_page_message(msg_dequeue());
    }
    func_home_page_exit();
}
