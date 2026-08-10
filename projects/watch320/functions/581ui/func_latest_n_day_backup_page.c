#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define LATEST_DIVIDER_Y            22
#define LATEST_ICON_Y               22
#define LATEST_BACK_X               18
#define LATEST_BAT_X                (GUI_SCREEN_WIDTH - 22)

#define LATEST_CARD_Y               78
#define LATEST_CARD_LABEL_Y         62
#define LATEST_CARD_NAME_Y          88

#define LATEST_STEP_Y               154
#define LATEST_MINUS_X              40
#define LATEST_PLUS_X               (GUI_SCREEN_WIDTH - 40)

#define LATEST_TIP0_Y               194
#define LATEST_TIP_GAP              20
#define LATEST_TIP_X                16

#define LATEST_BTN_Y                (GUI_SCREEN_HEIGHT - 36)

#define LATEST_DAYS_MIN             1
#define LATEST_DAYS_MAX             9
#define LATEST_DAYS_DEFAULT         6

#define LATEST_COLOR_LABEL          make_color(0x99, 0x99, 0x99)
#define LATEST_COLOR_STEP           make_color(0x00, 0x7A, 0xFF)

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
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_back;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_card;
    compo_picturebox_t *pic_step;
    compo_picturebox_t *pic_plus;
    compo_picturebox_t *pic_btn;
    compo_textbox_t *txt_title_num;
    compo_textbox_t *txt_days_num;
    compo_textbox_t *txt_tip1;
    compo_textbox_t *txt_tip2;
} f_latest_t;

static u8 latest_bat_level_from_percent(u8 percent)
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

static void latest_update_battery(void)
{
    f_latest_t *f = (f_latest_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = latest_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void latest_update_days_text(void)
{
    f_latest_t *f = (f_latest_t *)func_cb.f_cb;
    char buf[48];

    if (f == NULL) {
        return;
    }

    sprintf(buf, "%u", f->days);
    if (f->txt_title_num) {
        compo_textbox_set(f->txt_title_num, buf);
    }
    if (f->txt_days_num) {
        compo_textbox_set(f->txt_days_num, buf);
    }

    /* 向前连续 N 个自然日（含当天） */
    if (f->txt_tip1) {
        sprintf(buf, "%s %u %s", i18n[STR_FORWARD_CONTINUOUS], f->days, i18n[STR_NATURAL_DAYS]);
        compo_textbox_set(f->txt_tip1, buf);
    }

    /* 示例范围：以 7/21 为最新日 */
    if (f->txt_tip2) {
        u8 start_day = (21 >= f->days) ? (21 - f->days + 1) : 1;
        sprintf(buf, "%s 7/21%s 7/%u-7/21", i18n[STR_EXAMPLE_LATEST], i18n[STR_EXAMPLE_RANGE], start_day);
        compo_textbox_set(f->txt_tip2, buf);
    }
}

static void latest_update_display(void)
{
    f_latest_t *f = (f_latest_t *)func_cb.f_cb;

    if (f == NULL || f->pic_btn == NULL) {
        return;
    }

    compo_picturebox_set(f->pic_btn, f->btn_press
        ? UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_CLICK_BIN
        : UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    latest_update_days_text();
}

static void latest_change_days(s8 delta)
{
    f_latest_t *f = (f_latest_t *)func_cb.f_cb;
    s16 next;

    if (f == NULL) {
        return;
    }

    next = (s16)f->days + delta;
    if (next < LATEST_DAYS_MIN) {
        next = LATEST_DAYS_MIN;
    } else if (next > LATEST_DAYS_MAX) {
        next = LATEST_DAYS_MAX;
    }
    if ((u8)next == f->days) {
        return;
    }
    f->days = (u8)next;
    backup_param.latest_days = f->days;
    latest_update_days_text();
}

compo_form_t *func_latest_n_day_backup_page_form_create(void)
{
    f_latest_t *f = (f_latest_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    f->days = backup_param.latest_days ? backup_param.latest_days : LATEST_DAYS_DEFAULT;

    /* 顶部：divider + 返回 + 标题(最新 N 日备份) + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, LATEST_DIVIDER_Y);

    f->pic_back = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACK_BIN);
    compo_picturebox_set_pos(f->pic_back, LATEST_BACK_X, LATEST_ICON_Y);

    {
        compo_textbox_t *txt_latest;
        compo_textbox_t *txt_day_bak;
        s16 title_x;
        s16 w1, w2, w3;

        txt_latest = compo_textbox_create(frm, 16);
        compo_textbox_set_autosize(txt_latest, true);
        compo_textbox_set_align_center(txt_latest, false);
        compo_textbox_set_font(txt_latest, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(txt_latest, i18n[STR_LATEST]);
        w1 = compo_textbox_get_wid(txt_latest);

        f->txt_title_num = compo_textbox_create(frm, 4);
        compo_textbox_set_autosize(f->txt_title_num, true);
        compo_textbox_set_align_center(f->txt_title_num, false);
        compo_textbox_set_font(f->txt_title_num, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(f->txt_title_num, "6");
        w2 = compo_textbox_get_wid(f->txt_title_num) + 8; /* 预留数字宽度 */

        txt_day_bak = compo_textbox_create(frm, 16);
        compo_textbox_set_autosize(txt_day_bak, true);
        compo_textbox_set_align_center(txt_day_bak, false);
        compo_textbox_set_font(txt_day_bak, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(txt_day_bak, i18n[STR_DAYS_BACKUP]);
        w3 = compo_textbox_get_wid(txt_day_bak);

        title_x = GUI_SCREEN_CENTER_X - (w1 + w2 + w3) / 2;
        compo_textbox_set_location(txt_latest, title_x, LATEST_ICON_Y - 8, 0, 0);
        compo_textbox_set_location(f->txt_title_num, title_x + w1 + 4, LATEST_ICON_Y - 8, 0, 0);
        compo_textbox_set_location(txt_day_bak, title_x + w1 + w2, LATEST_ICON_Y - 8, 0, 0);
    }

    f->bat_level = latest_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, LATEST_BAT_X, LATEST_ICON_Y);

    /* 目标路径卡片 */
    f->pic_card = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_GROUP_1_BIN);
    compo_picturebox_set_pos(f->pic_card, GUI_SCREEN_CENTER_X, LATEST_CARD_Y + 10);

    {
        compo_textbox_t *txt_save;
        s16 path_x;

        txt_save = compo_textbox_create(frm, 16);
        compo_textbox_set_location(txt_save, 20, LATEST_CARD_LABEL_Y + 3, 0, 0);
        compo_textbox_set_autosize(txt_save, true);
        compo_textbox_set_align_center(txt_save, false);
        compo_textbox_set_font(txt_save, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(txt_save, LATEST_COLOR_LABEL);
        compo_textbox_set(txt_save, i18n[STR_SAVE_TO]);

        path_x = 20 + compo_textbox_get_wid(txt_save);
        txt = compo_textbox_create(frm, 32);
        compo_textbox_set_location(txt, path_x, LATEST_CARD_LABEL_Y + 3, 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(txt, LATEST_COLOR_LABEL);
        compo_textbox_set(txt, "SSD·RECENT_BACKUP");
    }

    txt = compo_textbox_create(frm, 24);
    compo_textbox_set_location(txt, 20, LATEST_CARD_NAME_Y - 5, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
    compo_textbox_set(txt, "RECENT_004");

    /* 天数步进：Group_2 + 减/加 + 数字日 */
    f->pic_step = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_GROUP_2_BIN);
    compo_picturebox_set_pos(f->pic_step, GUI_SCREEN_CENTER_X, LATEST_STEP_Y);

    txt = compo_textbox_create(frm, 4);
    compo_textbox_set_location(txt, LATEST_MINUS_X, LATEST_STEP_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_22_BIN);
    compo_textbox_set_forecolor(txt, LATEST_COLOR_STEP);

    {
        compo_textbox_t *txt_day;
        s16 num_x;
        s16 w_num, w_day;

        f->txt_days_num = compo_textbox_create(frm, 4);
        compo_textbox_set_autosize(f->txt_days_num, true);
        compo_textbox_set_align_center(f->txt_days_num, false);
        compo_textbox_set_font(f->txt_days_num, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
        compo_textbox_set(f->txt_days_num, "6");
        w_num = compo_textbox_get_wid(f->txt_days_num);

        txt_day = compo_textbox_create(frm, 4);
        compo_textbox_set_autosize(txt_day, true);
        compo_textbox_set_align_center(txt_day, false);
        compo_textbox_set_font(txt_day, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(txt_day, i18n[STR_DAY]);
        w_day = compo_textbox_get_wid(txt_day);

        num_x = GUI_SCREEN_CENTER_X - (w_num + w_day) / 2;
        compo_textbox_set_location(f->txt_days_num, num_x, LATEST_STEP_Y - 10, 0, 0);
        compo_textbox_set_location(txt_day, num_x + w_num + 2, LATEST_STEP_Y - 8, 0, 0);
    }

    f->pic_plus = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_ADD_BIN);
    compo_picturebox_set_pos(f->pic_plus, LATEST_PLUS_X, LATEST_STEP_Y);

    /* 说明文字 */
    txt = compo_textbox_create(frm, 48);
    compo_textbox_set_location(txt, LATEST_TIP_X, LATEST_TIP0_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set(txt, i18n[STR_BASED_ON_LATEST_FILE]);

    f->txt_tip1 = compo_textbox_create(frm, 48);
    compo_textbox_set_location(f->txt_tip1, LATEST_TIP_X, LATEST_TIP0_Y + LATEST_TIP_GAP, 0, 0);
    compo_textbox_set_autosize(f->txt_tip1, true);
    compo_textbox_set_align_center(f->txt_tip1, false);
    compo_textbox_set_font(f->txt_tip1, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    f->txt_tip2 = compo_textbox_create(frm, 48);
    compo_textbox_set_location(f->txt_tip2, LATEST_TIP_X, LATEST_TIP0_Y + LATEST_TIP_GAP * 2, 0, 0);
    compo_textbox_set_autosize(f->txt_tip2, true);
    compo_textbox_set_align_center(f->txt_tip2, false);
    compo_textbox_set_font(f->txt_tip2, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    /* 底部开始备份 */
    f->pic_btn = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_btn, GUI_SCREEN_CENTER_X, LATEST_BTN_Y);

    {
        compo_textbox_t *txt_start;
        compo_textbox_t *txt_cards;
        s16 start_x;
        s16 w1, w2;

        txt_start = compo_textbox_create(frm, 16);
        compo_textbox_set_autosize(txt_start, true);
        compo_textbox_set_align_center(txt_start, false);
        compo_textbox_set_font(txt_start, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(txt_start, i18n[STR_START_BACKUP]);
        w1 = compo_textbox_get_wid(txt_start);

        txt_cards = compo_textbox_create(frm, 16);
        compo_textbox_set_autosize(txt_cards, true);
        compo_textbox_set_align_center(txt_cards, false);
        compo_textbox_set_font(txt_cards, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(txt_cards, backup_param.card_sel[0] ? backup_param.card_sel : " SD");
        w2 = compo_textbox_get_wid(txt_cards);

        start_x = GUI_SCREEN_CENTER_X - (w1 + w2) / 2;
        compo_textbox_set_location(txt_start, start_x, LATEST_BTN_Y - 10, 0, 0);
        compo_textbox_set_location(txt_cards, start_x + w1, LATEST_BTN_Y - 10, 0, 0);
    }

    f->btn_press = 0;
    latest_update_display();

    return frm;
}

static void func_latest_n_day_backup_page_process(void)
{
    f_latest_t *f = (f_latest_t *)func_cb.f_cb;

    /* 触摸抬起或滑动移开时，恢复按钮未按下状态 */
    if (f->btn_press && !ctp_is_touch()) {
        f->btn_press = 0;
        latest_update_display();
    }
    latest_update_battery();
    func_process();
}

static void func_latest_n_day_backup_page_message(size_msg_t msg)
{
    f_latest_t *f = (f_latest_t *)func_cb.f_cb;
    point_t pt;

    switch (msg)
    {
    case MSG_QDEC_FORWARD:
        latest_change_days(1);
        break;

    case MSG_QDEC_BACKWARD:
        latest_change_days(-1);
        break;

    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        /* 按下底部按钮：切换为按下图 */
        if (pt.y > (LATEST_BTN_Y - 30)) {
            f->btn_press = 1;
            latest_update_display();
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG_LIFT:
        /* 滑动移开或抬起：恢复未按下图 */
        if (f->btn_press) {
            f->btn_press = 0;
            latest_update_display();
        }
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        if (pt.x < 48 && pt.y < 48) {
            func_cb.sta = FUNC_HOME_PAGE;
        } else if (pt.y > (LATEST_CARD_Y - 30) && pt.y < (LATEST_CARD_Y + 30)) {
            func_cb.last = FUNC_LATEST_N_DAY_BACKUP;
            func_cb.sta = FUNC_CONTENTS_PAGE;
        } else if (pt.y > (LATEST_STEP_Y - 30) && pt.y < (LATEST_STEP_Y + 30)) {
            if (pt.x < GUI_SCREEN_CENTER_X) {
                latest_change_days(-1);
            } else {
                latest_change_days(1);
            }
        } else if (pt.y > (LATEST_BTN_Y - 30)) {
            /* 开始备份：后续接业务 */
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

void func_latest_n_day_backup_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_latest_t));
    func_cb.frm_main = func_latest_n_day_backup_page_form_create();
}

void func_latest_n_day_backup_page_exit(void)
{
    func_cb.last = FUNC_HOME_PAGE;
}

void func_latest_n_day_backup_page(void)
{
    printf("%s\n", __func__);
    func_latest_n_day_backup_page_enter();
    while (func_cb.sta == FUNC_LATEST_N_DAY_BACKUP)
    {
        func_latest_n_day_backup_page_process();
        func_latest_n_day_backup_page_message(msg_dequeue());
    }
    func_latest_n_day_backup_page_exit();
}
