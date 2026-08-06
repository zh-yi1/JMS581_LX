#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

// 首页私有状态
typedef struct {
    u8 selection;  // 0 = 备份模式, 1 = 设置
    compo_picturebox_t *pic_backup;
    compo_picturebox_t *pic_set;
    compo_textbox_t *txt_backup;
    compo_textbox_t *txt_set;
} f_home_t;

static void home_update_display(void)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;
    bool is_backup = (h->selection == 0);

    // 备份模式：选中态 vs 普通态
    compo_picturebox_set(h->pic_backup, is_backup
        ? UI_BUF_IMAGE_BIN_BACKUP_MODE_1_BIN
        : UI_BUF_IMAGE_BIN_BACKUP_MODE_BIN);
    compo_textbox_set_forecolor(h->txt_backup, is_backup ? COLOR_BLACK : COLOR_WHITE);

    // 设置：选中态 vs 普通态
    compo_picturebox_set(h->pic_set, is_backup
        ? UI_BUF_IMAGE_BIN_SET_BIN
        : UI_BUF_IMAGE_BIN_SET_1_BIN);
    compo_textbox_set_forecolor(h->txt_set, is_backup ? COLOR_WHITE : COLOR_BLACK);
}

compo_form_t *func_home_page_form_create(void)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    /* 黑色背景，避免 GPU 误读 UI 头当图解码 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    txt = compo_textbox_create(frm, 18);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 80, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_F18_BIN);
    compo_textbox_set(txt, i18n[STR_HOME]);

    /* 上方：备份模式图标 */
    h->pic_backup = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACKUP_MODE_1_BIN);
    compo_picturebox_set_pos(h->pic_backup, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 24);

    h->txt_backup = compo_textbox_create(frm, 16);
    compo_textbox_set_location(h->txt_backup, GUI_SCREEN_CENTER_X - 60, GUI_SCREEN_CENTER_Y - 24, 0, 0);
    compo_textbox_set_autosize(h->txt_backup, true);
    compo_textbox_set_align_center(h->txt_backup, true);
    compo_textbox_set(h->txt_backup, i18n[STR_BACKUP]);

    /* 下方：设置图标 */
    h->pic_set = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_SET_BIN);
    compo_picturebox_set_pos(h->pic_set, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 24);

    h->txt_set = compo_textbox_create(frm, 16);
    compo_textbox_set_location(h->txt_set, GUI_SCREEN_CENTER_X - 78, GUI_SCREEN_CENTER_Y + 24, 0, 0);
    compo_textbox_set_autosize(h->txt_set, true);
    compo_textbox_set_align_center(h->txt_set, true);
    compo_textbox_set(h->txt_set, i18n[STR_SET]);

    /* 初始选中备份模式 */
    compo_textbox_set_forecolor(h->txt_backup, COLOR_BLACK);
    compo_textbox_set_forecolor(h->txt_set, COLOR_WHITE);

    return frm;
}

static void func_home_page_process(void)
{
    func_process();
}

static void func_home_page_message(size_msg_t msg)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;

    switch (msg)
    {
    case MSG_QDEC_FORWARD:
    case MSG_CTP_SHORT_DOWN:
        if (h->selection != 1) {
            h->selection = 1;
            home_update_display();
        }
        break;

    case MSG_QDEC_BACKWARD:
    case MSG_CTP_SHORT_UP:
        if (h->selection != 0) {
            h->selection = 0;
            home_update_display();
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_home_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_home_t));
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
