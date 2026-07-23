#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//备份模式选中状态
static void select_backup_mode(compo_form_t *frm)
{
    compo_textbox_t *txt1;
    compo_textbox_t *txt2;
    /* 上方: 备份模式图标 */
    compo_picturebox_t *pic_backup = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACKUP_MODE_1_BIN);
    compo_picturebox_set_pos(pic_backup, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 24);

    txt1 = compo_textbox_create(frm, 7);
    compo_textbox_set_location(txt1, GUI_SCREEN_CENTER_X - 30, GUI_SCREEN_CENTER_Y - 24, 64, 23);
    compo_textbox_set_align_center(txt1, true);
    compo_textbox_set_forecolor(txt1, COLOR_BLACK);
    compo_textbox_set(txt1, i18n[STR_BACKUP]);

    /* 下方: 设置图标 */
    compo_picturebox_t *pic_set = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_SET_BIN);
    compo_picturebox_set_pos(pic_set, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 24);

    txt2 = compo_textbox_create(frm, 7);
    compo_textbox_set_location(txt2, GUI_SCREEN_CENTER_X - 30, GUI_SCREEN_CENTER_Y + 24, 64, 23);
    compo_textbox_set_align_center(txt2, true);
    compo_textbox_set_forecolor(txt2, COLOR_WHITE);
    compo_textbox_set(txt2, i18n[STR_SET]);
}

// 设置模式选中状态
static void select_set(compo_form_t *frm)
{
    compo_textbox_t *txt1;
    compo_textbox_t *txt2;
    /* 上方: 备份模式图标 */
    compo_picturebox_t *pic_backup = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACKUP_MODE_BIN);
    compo_picturebox_set_pos(pic_backup, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 24);

    txt1 = compo_textbox_create(frm, 7);
    compo_textbox_set_location(txt1, GUI_SCREEN_CENTER_X - 30, GUI_SCREEN_CENTER_Y - 24, 64, 23);
    compo_textbox_set_align_center(txt1, true);
    compo_textbox_set_forecolor(txt1, COLOR_WHITE);
    compo_textbox_set(txt1, i18n[STR_BACKUP]);

    /* 下方: 设置图标 */
    compo_picturebox_t *pic_set = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_SET_1_BIN);
    compo_picturebox_set_pos(pic_set, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 24);

    txt2 = compo_textbox_create(frm, 7);
    compo_textbox_set_location(txt2, GUI_SCREEN_CENTER_X - 30, GUI_SCREEN_CENTER_Y + 24, 64, 23);
    compo_textbox_set_align_center(txt2, true);
    compo_textbox_set_forecolor(txt2, COLOR_BLACK);
    compo_textbox_set(txt2, i18n[STR_SET]);
}

compo_form_t *func_home_page_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    compo_textbox_t *txt1;
    compo_textbox_t *txt2;

    /* 黑色背景，避免 GPU 误读 UI 头当图解码 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    txt = compo_textbox_create(frm, 4);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y-80, 200, 40);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set(txt, i18n[STR_HOME]);

    select_backup_mode(frm);

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
