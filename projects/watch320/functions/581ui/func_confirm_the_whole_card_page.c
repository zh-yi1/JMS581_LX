#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define CONFIRM_TIP_CNT             3

#define CONFIRM_DIVIDER_Y           22
#define CONFIRM_ICON_Y              22
#define CONFIRM_BACK_X              18
#define CONFIRM_BAT_X               (GUI_SCREEN_WIDTH - 22)

#define CONFIRM_CARD_Y              90
#define CONFIRM_CARD_LABEL_Y        70
#define CONFIRM_CARD_NAME_Y         88

#define CONFIRM_TIP0_Y              155
#define CONFIRM_TIP_GAP             28
#define CONFIRM_CHECK_X             28
#define CONFIRM_TIP_TXT_X           44

#define CONFIRM_BTN_Y               (GUI_SCREEN_HEIGHT - 36)

#define CONFIRM_COLOR_LABEL         make_color(0x99, 0x99, 0x99)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

static const u32 s_tip_str[CONFIRM_TIP_CNT] = {
    STR_KEEP_DIR_STRUCT,
    STR_SKIP_DUP_FILES,
    STR_NO_MODIFY_SRC,
};

typedef struct {
    u8 bat_level;
    u8 btn_press;
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_back;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_card;
    compo_picturebox_t *pic_check[CONFIRM_TIP_CNT];
    compo_picturebox_t *pic_btn;
} f_confirm_t;

static u8 confirm_bat_level_from_percent(u8 percent)
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

static void confirm_update_battery(void)
{
    f_confirm_t *f = (f_confirm_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = confirm_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void confirm_update_display(void)
{
    f_confirm_t *f = (f_confirm_t *)func_cb.f_cb;

    if (f == NULL || f->pic_btn == NULL) {
        return;
    }

    compo_picturebox_set(f->pic_btn, f->btn_press
        ? UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_CLICK_BIN
        : UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
}

compo_form_t *func_confirm_the_whole_card_page_form_create(void)
{
    f_confirm_t *f = (f_confirm_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    u8 i;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider 底 + 返回 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, CONFIRM_DIVIDER_Y);

    f->pic_back = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACK_BIN);
    compo_picturebox_set_pos(f->pic_back, CONFIRM_BACK_X, CONFIRM_ICON_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, CONFIRM_ICON_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_FULL_CARD_BACKUP]);

    f->bat_level = confirm_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, CONFIRM_BAT_X, CONFIRM_ICON_Y);

    /* 目标路径卡片 */
    f->pic_card = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_GROUP_1_BIN);
    compo_picturebox_set_pos(f->pic_card, GUI_SCREEN_CENTER_X, CONFIRM_CARD_Y);

    {
        compo_textbox_t *txt_save;
        s16 path_x;

        txt_save = compo_textbox_create(frm, 16);
        compo_textbox_set_location(txt_save, 20, CONFIRM_CARD_LABEL_Y, 0, 0);
        compo_textbox_set_autosize(txt_save, true);
        compo_textbox_set_align_center(txt_save, false);
        compo_textbox_set_font(txt_save, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(txt_save, CONFIRM_COLOR_LABEL);
        compo_textbox_set(txt_save, i18n[STR_SAVE_TO]);

        path_x = 20 + compo_textbox_get_wid(txt_save);
        txt = compo_textbox_create(frm, 32);
        compo_textbox_set_location(txt, path_x, CONFIRM_CARD_LABEL_Y, 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(txt, CONFIRM_COLOR_LABEL);
        compo_textbox_set(txt, "SSD·CARD_BACKUP");
    }

    txt = compo_textbox_create(frm, 24);
    compo_textbox_set_location(txt, 20, CONFIRM_CARD_NAME_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
    compo_textbox_set(txt, "CARD_011");

    /* 特性说明 */
    for (i = 0; i < CONFIRM_TIP_CNT; i++) {
        s16 y = CONFIRM_TIP0_Y + i * CONFIRM_TIP_GAP;

        f->pic_check[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_CHECK_MARK_BIN);
        compo_picturebox_set_pos(f->pic_check[i], CONFIRM_CHECK_X, y);

        txt = compo_textbox_create(frm, 32);
        compo_textbox_set_location(txt, CONFIRM_TIP_TXT_X, y - 6, 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set(txt, i18n[s_tip_str[i]]);
    }

    /* 底部开始备份：多语言「开始备份」+ 固定「 SD CFA」 */
    f->pic_btn = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_btn, GUI_SCREEN_CENTER_X, CONFIRM_BTN_Y);

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
        compo_textbox_set_location(txt_start, start_x, CONFIRM_BTN_Y - 10, 0, 0);
        compo_textbox_set_location(txt_cards, start_x + w1, CONFIRM_BTN_Y - 10, 0, 0);
    }

    f->btn_press = 0;
    confirm_update_display();

    return frm;
}

static void func_confirm_the_whole_card_page_process(void)
{
    confirm_update_battery();
    func_process();
}

static void func_confirm_the_whole_card_page_message(size_msg_t msg)
{
    f_confirm_t *f = (f_confirm_t *)func_cb.f_cb;
    point_t pt;

    switch (msg)
    {
    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        /* 点击返回：左上角区域 */
        if (pt.x < 48 && pt.y < 48) {
            func_cb.sta = FUNC_HOME_PAGE;
        } else if (pt.y > (CONFIRM_CARD_Y - 30) && pt.y < (CONFIRM_CARD_Y + 30)) {
            func_cb.last = FUNC_CONFIRM_WHOLE_CARD;
            func_cb.sta = FUNC_CONTENTS_PAGE;
        } else if (pt.y > (CONFIRM_BTN_Y - 30)) {
            f->btn_press = 1;
            confirm_update_display();
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

void func_confirm_the_whole_card_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_confirm_t));
    func_cb.frm_main = func_confirm_the_whole_card_page_form_create();
}

void func_confirm_the_whole_card_page_exit(void)
{
    func_cb.last = FUNC_HOME_PAGE;
}

void func_confirm_the_whole_card_page(void)
{
    printf("%s\n", __func__);
    func_confirm_the_whole_card_page_enter();
    while (func_cb.sta == FUNC_CONFIRM_WHOLE_CARD)
    {
        func_confirm_the_whole_card_page_process();
        func_confirm_the_whole_card_page_message(msg_dequeue());
    }
    func_confirm_the_whole_card_page_exit();
}
