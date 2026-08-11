#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define SETUP_ITEM_CNT              4

#define SETUP_DIVIDER_Y             22
#define SETUP_ICON_Y                22
#define SETUP_BACK_X                18
#define SETUP_BAT_X                 (GUI_SCREEN_WIDTH - 22)

#define SETUP_ROW0_Y                75
#define SETUP_ROW_H                 60
#define SETUP_ROW_GAP               0

#define SETUP_ICON_X                28
#define SETUP_TITLE_X               52
#define SETUP_ARROW_X               (GUI_SCREEN_WIDTH - 16)
#define SETUP_VALUE_X               (GUI_SCREEN_WIDTH - 36)

#define SETUP_COLOR_SUB             make_color(0x99, 0x99, 0x99)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

static const u32 s_item_icon[SETUP_ITEM_CNT] = {
    UI_BUF_IMAGE_BIN_SETUP_LANGUAGE_BIN,
    UI_BUF_IMAGE_BIN_SETUP_SSD_BIN,
    UI_BUF_IMAGE_BIN_SETUP_UP_BIN,
    UI_BUF_IMAGE_BIN_SETUP_INFO_BIN,
};

static const u32 s_item_title[SETUP_ITEM_CNT] = {
    STR_LANGUAGE,
    STR_FORMAT_SSD,
    STR_SYS_UPGRADE,
    STR_DEVICE_INFO,
};

static const u32 s_item_sub[SETUP_ITEM_CNT] = {
    STR_LANGUAGE_TIP,
    STR_FORMAT_SSD_TIP,
    STR_SYS_UPGRADE_TIP,
    STR_DEVICE_INFO_TIP,
};

typedef struct {
    u8 bat_level;
    u8 press_idx;   // 0~3 按下中的行，0xFF=无
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_back;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_row[SETUP_ITEM_CNT];
    compo_picturebox_t *pic_icon[SETUP_ITEM_CNT];
    compo_picturebox_t *pic_arrow[SETUP_ITEM_CNT];
    compo_textbox_t *txt_title[SETUP_ITEM_CNT];
    compo_textbox_t *txt_sub[SETUP_ITEM_CNT];
    compo_textbox_t *txt_value[SETUP_ITEM_CNT];
} f_setup_t;

static u8 setup_hit_row(point_t pt)
{
    u8 i;

    for (i = 0; i < SETUP_ITEM_CNT; i++) {
        s16 y = SETUP_ROW0_Y + i * (SETUP_ROW_H + SETUP_ROW_GAP);
        if (pt.y > (y - SETUP_ROW_H / 2) && pt.y < (y + SETUP_ROW_H / 2)) {
            return i;
        }
    }
    return 0xFF;
}

static void setup_set_row_press(f_setup_t *f, u8 idx)
{
    if (f->press_idx < SETUP_ITEM_CNT) {
        compo_picturebox_set(f->pic_row[f->press_idx],
                             UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    }
    f->press_idx = idx;
    if (idx < SETUP_ITEM_CNT) {
        compo_picturebox_set(f->pic_row[idx], UI_BUF_IMAGE_BIN_CLICK_BJ_BIN);
    }
}

static void setup_clear_press(f_setup_t *f)
{
    setup_set_row_press(f, 0xFF);
}

static u8 setup_bat_level_from_percent(u8 percent)
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

static void setup_update_battery(void)
{
    f_setup_t *f = (f_setup_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = setup_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void setup_update_display(void)
{
    f_setup_t *f = (f_setup_t *)func_cb.f_cb;
    u8 i;

    if (f == NULL) {
        return;
    }

    for (i = 0; i < SETUP_ITEM_CNT; i++) {
        compo_picturebox_set(f->pic_row[i], (f->press_idx == i)
            ? UI_BUF_IMAGE_BIN_CLICK_BJ_BIN
            : UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);

        compo_textbox_set(f->txt_title[i], i18n[s_item_title[i]]);
        compo_textbox_set(f->txt_sub[i], i18n[s_item_sub[i]]);
    }

    /* 语言当前值 / 系统版本 */
    if (f->txt_value[0]) {
        compo_textbox_set(f->txt_value[0], i18n[STR_LANG_ZH_CN]);
    }
    if (f->txt_value[2]) {
        compo_textbox_set(f->txt_value[2], "v1.2.0");
    }
}

compo_form_t *func_setup_page_form_create(void)
{
    f_setup_t *f = (f_setup_t *)func_cb.f_cb;
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
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, SETUP_DIVIDER_Y);

    f->pic_back = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACK_BIN);
    compo_picturebox_set_pos(f->pic_back, SETUP_BACK_X, SETUP_ICON_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, SETUP_ICON_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_SET]);

    f->bat_level = setup_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, SETUP_BAT_X, SETUP_ICON_Y);

    /* 设置列表 */
    for (i = 0; i < SETUP_ITEM_CNT; i++) {
        s16 y = SETUP_ROW0_Y + i * (SETUP_ROW_H + SETUP_ROW_GAP);

        f->pic_row[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
        compo_picturebox_set_pos(f->pic_row[i], GUI_SCREEN_CENTER_X, y);

        f->pic_icon[i] = compo_picturebox_create(frm, s_item_icon[i]);
        compo_picturebox_set_pos(f->pic_icon[i], SETUP_ICON_X, y);

        f->txt_title[i] = compo_textbox_create(frm, 16);
        compo_textbox_set_location(f->txt_title[i], SETUP_TITLE_X, y - 22, 0, 0);
        compo_textbox_set_autosize(f->txt_title[i], true);
        compo_textbox_set_align_center(f->txt_title[i], false);
        compo_textbox_set_font(f->txt_title[i], UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);

        f->txt_sub[i] = compo_textbox_create(frm, 24);
        compo_textbox_set_location(f->txt_sub[i], SETUP_TITLE_X, y, 0, 0);
        compo_textbox_set_autosize(f->txt_sub[i], true);
        compo_textbox_set_align_center(f->txt_sub[i], false);
        compo_textbox_set_font(f->txt_sub[i], UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(f->txt_sub[i], SETUP_COLOR_SUB);

        f->txt_value[i] = compo_textbox_create(frm, 16);
        compo_textbox_set_location(f->txt_value[i], SETUP_VALUE_X - 15, y, 0, 0);
        compo_textbox_set_autosize(f->txt_value[i], true);
        compo_textbox_set_align_center(f->txt_value[i], true);
        compo_textbox_set_font(f->txt_value[i], UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(f->txt_value[i], SETUP_COLOR_SUB);
        if (i != 0 && i != 2) {
            compo_textbox_set_visible(f->txt_value[i], false);
        }

        f->pic_arrow[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_SETUP_LEFT_BIN);
        compo_picturebox_set_pos(f->pic_arrow[i], SETUP_ARROW_X, y);
    }

    f->press_idx = 0xFF;
    setup_update_display();

    return frm;
}

static void func_setup_page_process(void)
{
    f_setup_t *f = (f_setup_t *)func_cb.f_cb;

    if (f->press_idx != 0xFF && !ctp_is_touch()) {
        setup_clear_press(f);
    }
    setup_update_battery();
    func_process();
}

static void func_setup_page_message(size_msg_t msg)
{
    f_setup_t *f = (f_setup_t *)func_cb.f_cb;
    point_t pt;
    u8 idx;

    switch (msg)
    {
    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        if (pt.x < 48 && pt.y < 48) {
            setup_clear_press(f);
            break;
        }
        idx = setup_hit_row(pt);
        if (idx != 0xFF) {
            setup_set_row_press(f, idx);
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG_LIFT:
        setup_clear_press(f);
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        setup_clear_press(f);
        /* 返回首页 */
        if (pt.x < 48 && pt.y < 48) {
            func_cb.sta = FUNC_HOME_PAGE;
            break;
        }
        /* 列表项：子页面待接 */
        idx = setup_hit_row(pt);
        if (idx != 0xFF) {
            TRACE("setup item %u\n", idx);
        }
        break;

    case KU_BACK:
        func_cb.sta = FUNC_HOME_PAGE;
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_setup_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_setup_t));
    func_cb.frm_main = func_setup_page_form_create();
}

void func_setup_page_exit(void)
{
    func_cb.last = FUNC_HOME_PAGE;
}

void func_setup_page(void)
{
    printf("%s\n", __func__);
    func_setup_page_enter();
    while (func_cb.sta == FUNC_SETUP_PAGE)
    {
        func_setup_page_process();
        func_setup_page_message(msg_dequeue());
    }
    func_setup_page_exit();
}
