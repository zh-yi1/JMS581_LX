#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define INSERT_CARD_DIVIDER_Y       22
#define INSERT_CARD_TITLE_Y         22
#define INSERT_CARD_BAT_X           (GUI_SCREEN_WIDTH - 22)

#define INSERT_CARD_ICON_Y          120

#define INSERT_CARD_STA_Y           175
#define INSERT_CARD_TIP_Y           200

#define INSERT_CARD_COLOR_TIP       make_color(0x99, 0x99, 0x99)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

typedef struct {
    u8 bat_level;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_icon;
} f_insert_card_t;

static u8 insert_card_bat_level_from_percent(u8 percent)
{
    if (percent <= 20) {
        return 1;
    }
    if (percent <= 40) {
        return 2;
    }
    if (percent <= 60) {
        return 3;
    }
    if (percent <= 80) {
        return 4;
    }
    return 5;
}

static void insert_card_update_battery(void)
{
    f_insert_card_t *f = (f_insert_card_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = insert_card_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

compo_form_t *func_insert_card_page_form_create(void)
{
    f_insert_card_t *f = (f_insert_card_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, INSERT_CARD_DIVIDER_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, INSERT_CARD_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_PREPARE_BACKUP]);

    f->bat_level = insert_card_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, INSERT_CARD_BAT_X, INSERT_CARD_TITLE_Y);

    /* 中间：插卡图标 */
    f->pic_icon = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_INSERT_CARD_BIN);
    compo_picturebox_set_pos(f->pic_icon, GUI_SCREEN_CENTER_X, INSERT_CARD_ICON_Y);

    /* 插入存储卡 */
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, INSERT_CARD_STA_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_INSERT_CARD]);

    /* 支持 SD、CFA 和 CFB */
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, INSERT_CARD_TIP_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, INSERT_CARD_COLOR_TIP);
    compo_textbox_set(txt, i18n[STR_SUPPORT_CARD_TYPE]);

    return frm;
}

static void func_insert_card_page_process(void)
{
    insert_card_update_battery();
    func_process();
}

static void func_insert_card_page_message(size_msg_t msg)
{
    switch (msg)
    {
    case KU_BACK:
        func_cb.sta = FUNC_HOME_PAGE;
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_insert_card_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_insert_card_t));
    func_cb.frm_main = func_insert_card_page_form_create();
}

void func_insert_card_page_exit(void)
{
    func_cb.last = FUNC_HOME_PAGE;
}

void func_insert_card_page(void)
{
    printf("%s\n", __func__);
    func_insert_card_page_enter();
    while (func_cb.sta == FUNC_INSERT_CARD_PAGE)
    {
        func_insert_card_page_process();
        func_insert_card_page_message(msg_dequeue());
    }
    func_insert_card_page_exit();
}
