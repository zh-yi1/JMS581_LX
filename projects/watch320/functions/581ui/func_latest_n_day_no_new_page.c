#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define LNONEW_DIVIDER_Y            22
#define LNONEW_TITLE_Y              22
#define LNONEW_BAT_X                (GUI_SCREEN_WIDTH - 22)

#define LNONEW_ICON_Y               100

#define LNONEW_STA_Y                150
#define LNONEW_TIP0_Y               180
#define LNONEW_TIP1_Y               200
#define LNONEW_DIR_Y                230

#define LNONEW_BTN_Y                (GUI_SCREEN_HEIGHT - 36)

#define LNONEW_COLOR_LABEL          make_color(0x99, 0x99, 0x99)

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
    u8 days;
    char dir_name[16];      // 如 "RECENT_003"
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_ok;
    compo_picturebox_t *pic_btn;
    compo_textbox_t *txt_title;
    compo_textbox_t *txt_dir;
} f_lnonew_t;

static u8 lnonew_bat_level_from_percent(u8 percent)
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

static void lnonew_update_battery(void)
{
    f_lnonew_t *f = (f_lnonew_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = lnonew_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void lnonew_update_display(void)
{
    f_lnonew_t *f = (f_lnonew_t *)func_cb.f_cb;
    char buf[48];

    if (f == NULL) {
        return;
    }

    if (f->pic_btn) {
        compo_picturebox_set(f->pic_btn, f->btn_press
            ? UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_CLICK_BIN
            : UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    }

    if (f->txt_title) {
        sprintf(buf, "%s %u %s", i18n[STR_LATEST], f->days, i18n[STR_DAYS_BACKUP]);
        compo_textbox_set(f->txt_title, buf);
    }

    if (f->txt_dir) {
        sprintf(buf, "%s %s", i18n[STR_CHECKED], f->dir_name);
        compo_textbox_set(f->txt_dir, buf);
    }
}

compo_form_t *func_latest_n_day_no_new_page_form_create(void)
{
    f_lnonew_t *f = (f_lnonew_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider 底 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, LNONEW_DIVIDER_Y);

    f->txt_title = compo_textbox_create(frm, 24);
    compo_textbox_set_location(f->txt_title, GUI_SCREEN_CENTER_X, LNONEW_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_title, true);
    compo_textbox_set_align_center(f->txt_title, true);
    compo_textbox_set_font(f->txt_title, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);

    f->bat_level = lnonew_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, LNONEW_BAT_X, LNONEW_TITLE_Y);

    /* 中间：勾选图标 */
    f->pic_ok = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_CHECK_OUT_CORRECT_BIN);
    compo_picturebox_set_pos(f->pic_ok, GUI_SCREEN_CENTER_X, LNONEW_ICON_Y);

    /* 没有新内容 */
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LNONEW_STA_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_NO_NEW_CONTENT]);

    /* 说明文案 */
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LNONEW_TIP0_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, LNONEW_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_NO_NEW_FILES]);

    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LNONEW_TIP1_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, LNONEW_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_DUP_SKIPPED]);

    /* 已检查目录 */
    f->txt_dir = compo_textbox_create(frm, 32);
    compo_textbox_set_location(f->txt_dir, GUI_SCREEN_CENTER_X, LNONEW_DIR_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_dir, true);
    compo_textbox_set_align_center(f->txt_dir, true);
    compo_textbox_set_font(f->txt_dir, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(f->txt_dir, LNONEW_COLOR_LABEL);

    /* 底部返回首页按钮 */
    f->pic_btn = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_btn, GUI_SCREEN_CENTER_X, LNONEW_BTN_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LNONEW_BTN_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
    compo_textbox_set(txt, i18n[STR_BACK_HOME]);

    f->btn_press = 0;
    lnonew_update_display();

    return frm;
}

static void func_latest_n_day_no_new_page_process(void)
{
    f_lnonew_t *f = (f_lnonew_t *)func_cb.f_cb;

    if (f->btn_press && !ctp_is_touch()) {
        f->btn_press = 0;
        lnonew_update_display();
    }
    lnonew_update_battery();
    func_process();
}

static void func_latest_n_day_no_new_page_message(size_msg_t msg)
{
    f_lnonew_t *f = (f_lnonew_t *)func_cb.f_cb;
    point_t pt;

    switch (msg)
    {
    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        if (pt.y > (LNONEW_BTN_Y - 30)) {
            f->btn_press = 1;
            lnonew_update_display();
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG_LIFT:
        if (f->btn_press) {
            f->btn_press = 0;
            lnonew_update_display();
        }
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        if (pt.y > (LNONEW_BTN_Y - 30)) {
            f->btn_press = 0;
            lnonew_update_display();
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

void func_latest_n_day_no_new_page_enter(void)
{
    f_lnonew_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_lnonew_t));
    f = (f_lnonew_t *)func_cb.f_cb;

    f->days = backup_param.latest_days ? backup_param.latest_days : 1;
    strcpy(f->dir_name, "RECENT_003");
    if (backup_param.dir_sel[0]) {
        strcpy(f->dir_name, backup_param.dir_sel);
    }

    func_cb.frm_main = func_latest_n_day_no_new_page_form_create();
}

void func_latest_n_day_no_new_page_exit(void)
{
    func_cb.last = FUNC_HOME_PAGE;
}

void func_latest_n_day_no_new_page(void)
{
    printf("%s\n", __func__);
    func_latest_n_day_no_new_page_enter();
    while (func_cb.sta == FUNC_LATEST_N_DAY_NO_NEW_PAGE)
    {
        func_latest_n_day_no_new_page_process();
        func_latest_n_day_no_new_page_message(msg_dequeue());
    }
    func_latest_n_day_no_new_page_exit();
}
