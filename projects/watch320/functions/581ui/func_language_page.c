#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define LANG_DIVIDER_Y              22
#define LANG_ICON_Y                 22
#define LANG_BACK_X                 18
#define LANG_BAT_X                  (GUI_SCREEN_WIDTH - 22)

#define LANG_ITEM_CNT               4
#define LANG_ROW0_Y                 84
#define LANG_ROW_H                  55
#define LANG_ROW_GAP                0
#define LANG_NAME_X                 28
#define LANG_CHECK_X                (GUI_SCREEN_WIDTH - 22)

#define LANG_COLOR_SEL              make_color(0x4A, 0x9E, 0xFF)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

/* 语言名保持母语显示，不随界面语言切换 */
static const char *s_lang_name[LANG_ITEM_CNT] = {
    "简体中文",
    "繁體中文",
    "English",
    "日本語",
};

typedef struct {
    u8 bat_level;
    u8 selection;   // 当前选中语言
    u8 press_idx;   // 按下中的行，0xFF=无
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_back;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_row[LANG_ITEM_CNT];
    compo_picturebox_t *pic_check[LANG_ITEM_CNT];
    compo_textbox_t *txt_name[LANG_ITEM_CNT];
} f_lang_t;

static u8 lang_bat_level_from_percent(u8 percent)
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

static void lang_update_battery(void)
{
    f_lang_t *f = (f_lang_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = lang_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void lang_update_display(void)
{
    f_lang_t *f = (f_lang_t *)func_cb.f_cb;
    u8 i;

    if (f == NULL) {
        return;
    }

    for (i = 0; i < LANG_ITEM_CNT; i++) {
        bool selected = (i == f->selection);
        bool pressed  = (i == f->press_idx);

        compo_picturebox_set(f->pic_row[i], (selected || pressed)
            ? UI_BUF_IMAGE_BIN_CONTENTS_CLICK_BIN
            : UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
        compo_textbox_set(f->txt_name[i], s_lang_name[i]);
        compo_textbox_set_forecolor(f->txt_name[i],
            selected ? LANG_COLOR_SEL : COLOR_WHITE);
        compo_picturebox_set_visible(f->pic_check[i], selected);
    }
}

static u8 lang_hit_row(point_t pt)
{
    u8 i;

    for (i = 0; i < LANG_ITEM_CNT; i++) {
        s16 y = LANG_ROW0_Y + i * (LANG_ROW_H + LANG_ROW_GAP);
        if (pt.y > (y - LANG_ROW_H / 2) && pt.y < (y + LANG_ROW_H / 2)) {
            return i;
        }
    }
    return 0xFF;
}

static u8 lang_idx_from_sys(void)
{
    if (sys_cb.lang_id == LANG_EN) {
        return 2;
    }
    return 0;   /* 默认简体中文 */
}

static void lang_apply_selection(u8 idx)
{
    if (idx >= LANG_ITEM_CNT) {
        return;
    }

    if (idx == 2) {
        sys_cb.lang_id = LANG_EN;
        lang_select(LANG_EN);
    } else {
        /* 简体 / 繁体 / 日语：当前仅简体有完整词条，其余先按中文 */
        sys_cb.lang_id = LANG_ZH;
        lang_select(LANG_ZH);
    }
}

compo_form_t *func_language_page_form_create(void)
{
    f_lang_t *f = (f_lang_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    u8 i;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider + 返回 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, LANG_DIVIDER_Y);

    f->pic_back = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACK_BIN);
    compo_picturebox_set_pos(f->pic_back, LANG_BACK_X, LANG_ICON_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LANG_ICON_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_LANGUAGE]);

    f->bat_level = lang_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, LANG_BAT_X, LANG_ICON_Y);

    /* 语言列表 */
    for (i = 0; i < LANG_ITEM_CNT; i++) {
        s16 y = LANG_ROW0_Y + i * (LANG_ROW_H + LANG_ROW_GAP);

        f->pic_row[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
        compo_picturebox_set_pos(f->pic_row[i], GUI_SCREEN_CENTER_X, y);

        f->txt_name[i] = compo_textbox_create(frm, 16);
        compo_textbox_set_location(f->txt_name[i], LANG_NAME_X, y - 10, 0, 0);
        compo_textbox_set_autosize(f->txt_name[i], true);
        compo_textbox_set_align_center(f->txt_name[i], false);
        compo_textbox_set_font(f->txt_name[i], UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(f->txt_name[i], s_lang_name[i]);

        f->pic_check[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_CHECK_MARK_BIN);
        compo_picturebox_set_pos(f->pic_check[i], LANG_CHECK_X, y);
        compo_picturebox_set_visible(f->pic_check[i], false);
    }

    f->selection = lang_idx_from_sys();
    f->press_idx = 0xFF;
    lang_update_display();

    return frm;
}

static void func_language_page_process(void)
{
    f_lang_t *f = (f_lang_t *)func_cb.f_cb;

    if (f->press_idx != 0xFF && !ctp_is_touch()) {
        f->press_idx = 0xFF;
        lang_update_display();
    }
    lang_update_battery();
    func_process();
}

static void func_language_page_message(size_msg_t msg)
{
    f_lang_t *f = (f_lang_t *)func_cb.f_cb;
    point_t pt;
    u8 idx;

    switch (msg)
    {
    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        idx = lang_hit_row(pt);
        if (idx != 0xFF) {
            f->press_idx = idx;
            lang_update_display();
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG_LIFT:
        if (f->press_idx != 0xFF) {
            f->press_idx = 0xFF;
            lang_update_display();
        }
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        if (pt.x < 48 && pt.y < 48) {
            func_cb.sta = FUNC_SETUP_PAGE;
            break;
        }
        idx = lang_hit_row(pt);
        if (idx != 0xFF) {
            f->selection = idx;
            f->press_idx = 0xFF;
            lang_apply_selection(idx);
            lang_update_display();
        }
        break;

    case KU_BACK:
        func_cb.sta = FUNC_SETUP_PAGE;
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_language_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_lang_t));
    func_cb.frm_main = func_language_page_form_create();
}

void func_language_page_exit(void)
{
    func_cb.last = FUNC_SETUP_PAGE;
}

void func_language_page(void)
{
    printf("%s\n", __func__);
    func_language_page_enter();
    while (func_cb.sta == FUNC_LANGUAGE_PAGE)
    {
        func_language_page_process();
        func_language_page_message(msg_dequeue());
    }
    func_language_page_exit();
}
