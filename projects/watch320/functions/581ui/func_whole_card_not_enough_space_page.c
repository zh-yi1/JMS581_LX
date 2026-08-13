#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define WCSPACE_DIVIDER_Y           22
#define WCSPACE_TITLE_Y             22
#define WCSPACE_BAT_X               (GUI_SCREEN_WIDTH - 22)

#define WCSPACE_ICON_Y              100

#define WCSPACE_STA_Y               150
#define WCSPACE_SHORT_Y             180
#define WCSPACE_DETAIL_Y            200
#define WCSPACE_TIP_Y               230

#define WCSPACE_BTN_Y               (GUI_SCREEN_HEIGHT - 36)

#define WCSPACE_COLOR_LABEL         make_color(0x99, 0x99, 0x99)

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
    char short_str[16];     // 如 "6.8 GB"
    char avail_str[16];     // 如 "1.2 GB"
    char need_str[16];      // 如 "8.0 GB"
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_info;
    compo_picturebox_t *pic_btn;
    compo_textbox_t *txt_short;
    compo_textbox_t *txt_detail;
} f_wcspace_t;

static u8 wcspace_bat_level_from_percent(u8 percent)
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

static void wcspace_update_battery(void)
{
    f_wcspace_t *f = (f_wcspace_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = wcspace_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void wcspace_update_display(void)
{
    f_wcspace_t *f = (f_wcspace_t *)func_cb.f_cb;
    char buf[48];

    if (f == NULL) {
        return;
    }

    if (f->pic_btn) {
        compo_picturebox_set(f->pic_btn, f->btn_press
            ? UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_CLICK_BIN
            : UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    }

    if (f->txt_short) {
        sprintf(buf, "%s %s", i18n[STR_SHORT_OF], f->short_str);
        compo_textbox_set(f->txt_short, buf);
    }

    if (f->txt_detail) {
        sprintf(buf, "%s %s · %s %s",
                i18n[STR_AVAIL], f->avail_str,
                i18n[STR_NEED], f->need_str);
        compo_textbox_set(f->txt_detail, buf);
    }
}

compo_form_t *func_whole_card_not_enough_space_page_form_create(void)
{
    f_wcspace_t *f = (f_wcspace_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider 底 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, WCSPACE_DIVIDER_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, WCSPACE_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_FULL_CARD_BACKUP]);

    f->bat_level = wcspace_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, WCSPACE_BAT_X, WCSPACE_TITLE_Y);

    /* 中间：警告图标 */
    f->pic_info = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_INFO_BIN);
    compo_picturebox_set_pos(f->pic_info, GUI_SCREEN_CENTER_X, WCSPACE_ICON_Y);

    /* SSD 空间不足 */
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, WCSPACE_STA_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_SSD_SPACE_LOW]);

    /* 还差 x GB */
    f->txt_short = compo_textbox_create(frm, 24);
    compo_textbox_set_location(f->txt_short, GUI_SCREEN_CENTER_X, WCSPACE_SHORT_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_short, true);
    compo_textbox_set_align_center(f->txt_short, true);
    compo_textbox_set_font(f->txt_short, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    /* 可用 · 需要 */
    f->txt_detail = compo_textbox_create(frm, 40);
    compo_textbox_set_location(f->txt_detail, GUI_SCREEN_CENTER_X, WCSPACE_DETAIL_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_detail, true);
    compo_textbox_set_align_center(f->txt_detail, true);
    compo_textbox_set_font(f->txt_detail, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    /* 请释放空间后重试 */
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, WCSPACE_TIP_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, WCSPACE_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_FREE_SPACE_RETRY]);

    /* 底部返回首页按钮 */
    f->pic_btn = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_btn, GUI_SCREEN_CENTER_X, WCSPACE_BTN_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, WCSPACE_BTN_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
    compo_textbox_set(txt, i18n[STR_BACK_HOME]);

    f->btn_press = 0;
    wcspace_update_display();

    return frm;
}

static void func_whole_card_not_enough_space_page_process(void)
{
    f_wcspace_t *f = (f_wcspace_t *)func_cb.f_cb;

    if (f->btn_press && !ctp_is_touch()) {
        f->btn_press = 0;
        wcspace_update_display();
    }
    wcspace_update_battery();
    func_process();
}

static void func_whole_card_not_enough_space_page_message(size_msg_t msg)
{
    f_wcspace_t *f = (f_wcspace_t *)func_cb.f_cb;
    point_t pt;

    switch (msg)
    {
    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        if (pt.y > (WCSPACE_BTN_Y - 30)) {
            f->btn_press = 1;
            wcspace_update_display();
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG_LIFT:
        if (f->btn_press) {
            f->btn_press = 0;
            wcspace_update_display();
        }
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        if (pt.y > (WCSPACE_BTN_Y - 30)) {
            f->btn_press = 0;
            wcspace_update_display();
            func_cb.sta = FUNC_HOME_PAGE;
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

void func_whole_card_not_enough_space_page_enter(void)
{
    f_wcspace_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_wcspace_t));
    f = (f_wcspace_t *)func_cb.f_cb;

    /* 演示数据，后续由容量检测逻辑填充 */
    strcpy(f->short_str, "6.8 GB");
    strcpy(f->avail_str, "1.2 GB");
    strcpy(f->need_str, "8.0 GB");

    func_cb.frm_main = func_whole_card_not_enough_space_page_form_create();
}

void func_whole_card_not_enough_space_page_exit(void)
{
    func_cb.last = FUNC_HOME_PAGE;
}

void func_whole_card_not_enough_space_page(void)
{
    printf("%s\n", __func__);
    func_whole_card_not_enough_space_page_enter();
    while (func_cb.sta == FUNC_WHOLE_CARD_NOT_ENOUGH_SPACE_PAGE)
    {
        func_whole_card_not_enough_space_page_process();
        func_whole_card_not_enough_space_page_message(msg_dequeue());
    }
    func_whole_card_not_enough_space_page_exit();
}
