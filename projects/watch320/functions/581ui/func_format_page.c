#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define FORMAT_DIVIDER_Y            22
#define FORMAT_TITLE_Y              22
#define FORMAT_BACK_X               18
#define FORMAT_BAT_X                (GUI_SCREEN_WIDTH - 22)

#define FORMAT_ICON_Y               80

#define FORMAT_STA_Y                122
#define FORMAT_TIP0_Y               142
#define FORMAT_TIP1_Y               158

#define FORMAT_CARD_Y               210
#define FORMAT_CARD_ROW0_Y          190
#define FORMAT_CARD_ROW1_Y          215
#define FORMAT_CARD_LABEL_X         28
#define FORMAT_CARD_VALUE_X         (GUI_SCREEN_WIDTH - 28)

#define FORMAT_BTN_Y                (GUI_SCREEN_HEIGHT - 36)

#define FORMAT_COLOR_LABEL          make_color(0x99, 0x99, 0x99)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

typedef struct {
    u8 bat_level;
    u8 btn_press;
    char cap_str[16];       // 如 "2.00 TB"
    char avail_str[16];     // 如 "1.42 TB"
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_back;
    compo_picturebox_t *pic_info;
    compo_picturebox_t *pic_card;
    compo_picturebox_t *pic_btn;
    compo_textbox_t *txt_cap;
    compo_textbox_t *txt_avail;
} f_format_t;

static u8 format_bat_level_from_percent(u8 percent)
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

static void format_update_battery(void)
{
    f_format_t *f = (f_format_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = format_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void format_update_display(void)
{
    f_format_t *f = (f_format_t *)func_cb.f_cb;

    if (f == NULL) {
        return;
    }

    if (f->pic_btn) {
        compo_picturebox_set(f->pic_btn, f->btn_press
            ? UI_BUF_IMAGE_BIN_LONG_BOTTON_RED_CLICK_BIN
            : UI_BUF_IMAGE_BIN_LONG_BOTTON_RED_UNCLICK_BIN);
    }

    if (f->txt_cap) {
        compo_textbox_set(f->txt_cap, f->cap_str);
    }
    if (f->txt_avail) {
        compo_textbox_set(f->txt_avail, f->avail_str);
    }
}

compo_form_t *func_format_page_form_create(void)
{
    f_format_t *f = (f_format_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider + 返回 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, FORMAT_DIVIDER_Y);

    f->pic_back = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACK_BIN);
    compo_picturebox_set_pos(f->pic_back, FORMAT_BACK_X, FORMAT_TITLE_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, FORMAT_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_FORMAT_SSD]);

    f->bat_level = format_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, FORMAT_BAT_X, FORMAT_TITLE_Y);

    /* 警告图标 */
    f->pic_info = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_INFO_BIN);
    compo_picturebox_set_pos(f->pic_info, GUI_SCREEN_CENTER_X, FORMAT_ICON_Y);

    /* 清除 SSD 全部内容 */
    txt = compo_textbox_create(frm, 24);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, FORMAT_STA_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_CLEAR_SSD_ALL]);

    /* 警告说明 */
    txt = compo_textbox_create(frm, 40);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, FORMAT_TIP0_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, FORMAT_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_FORMAT_WARN_FILES]);

    txt = compo_textbox_create(frm, 24);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, FORMAT_TIP1_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, FORMAT_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_FORMAT_WARN_UNDO]);

    /* 容量信息卡 */
    f->pic_card = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_GROUP_4_BIN);
    compo_picturebox_set_pos(f->pic_card, GUI_SCREEN_CENTER_X, FORMAT_CARD_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, FORMAT_CARD_LABEL_X, FORMAT_CARD_ROW0_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, FORMAT_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_SSD_CAPACITY]);

    f->txt_cap = compo_textbox_create(frm, 16);
    compo_textbox_set_location(f->txt_cap, FORMAT_CARD_VALUE_X, FORMAT_CARD_ROW0_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_cap, true);
    compo_textbox_set_align_center(f->txt_cap, false);
    compo_textbox_set_font(f->txt_cap, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, FORMAT_CARD_LABEL_X, FORMAT_CARD_ROW1_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, FORMAT_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_CUR_AVAIL]);

    f->txt_avail = compo_textbox_create(frm, 16);
    compo_textbox_set_location(f->txt_avail, FORMAT_CARD_VALUE_X, FORMAT_CARD_ROW1_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_avail, true);
    compo_textbox_set_align_center(f->txt_avail, false);
    compo_textbox_set_font(f->txt_avail, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);

    /* 底部确认格式化（红色） */
    f->pic_btn = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_LONG_BOTTON_RED_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_btn, GUI_SCREEN_CENTER_X, FORMAT_BTN_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, FORMAT_BTN_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
    compo_textbox_set(txt, i18n[STR_CONFIRM_FORMAT]);

    f->btn_press = 0;
    format_update_display();

    /* 右对齐容量数值：先设文案再按宽度回推 */
    {
        s16 w;

        w = compo_textbox_get_wid(f->txt_cap);
        compo_textbox_set_location(f->txt_cap, FORMAT_CARD_VALUE_X - w, FORMAT_CARD_ROW0_Y, 0, 0);
        w = compo_textbox_get_wid(f->txt_avail);
        compo_textbox_set_location(f->txt_avail, FORMAT_CARD_VALUE_X - w, FORMAT_CARD_ROW1_Y, 0, 0);
    }

    return frm;
}

static void func_format_page_process(void)
{
    f_format_t *f = (f_format_t *)func_cb.f_cb;

    if (f->btn_press && !ctp_is_touch()) {
        f->btn_press = 0;
        format_update_display();
    }
    format_update_battery();
    func_process();
}

static void func_format_page_message(size_msg_t msg)
{
    f_format_t *f = (f_format_t *)func_cb.f_cb;
    point_t pt;

    switch (msg)
    {
    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        if (pt.y > (FORMAT_BTN_Y - 30)) {
            f->btn_press = 1;
            format_update_display();
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG_LIFT:
        if (f->btn_press) {
            f->btn_press = 0;
            format_update_display();
        }
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        if (pt.x < 48 && pt.y < 48) {
            func_cb.sta = FUNC_SETUP_PAGE;
            break;
        }
        if (pt.y > (FORMAT_BTN_Y - 30)) {
            f->btn_press = 0;
            format_update_display();
            func_cb.sta = FUNC_FORMATING_PAGE;
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

void func_format_page_enter(void)
{
    f_format_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_format_t));
    f = (f_format_t *)func_cb.f_cb;

    /* 演示数据，后续由容量检测逻辑填充 */
    strcpy(f->cap_str, "2.00 TB");
    strcpy(f->avail_str, "1.42 TB");

    func_cb.frm_main = func_format_page_form_create();
}

void func_format_page_exit(void)
{
    if (func_cb.sta != FUNC_FORMATING_PAGE) {
        func_cb.last = FUNC_SETUP_PAGE;
    }
}

void func_format_page(void)
{
    printf("%s\n", __func__);
    func_format_page_enter();
    while (func_cb.sta == FUNC_FORMAT_PAGE)
    {
        func_format_page_process();
        func_format_page_message(msg_dequeue());
    }
    func_format_page_exit();
}
