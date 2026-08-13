#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define UPGRADE_DIVIDER_Y           22
#define UPGRADE_TITLE_Y             22
#define UPGRADE_BACK_X              18
#define UPGRADE_BAT_X               (GUI_SCREEN_WIDTH - 22)

#define UPGRADE_ICON_Y              72

#define UPGRADE_STA_Y               118
#define UPGRADE_TIP_Y               138

#define UPGRADE_CARD_Y              205
#define UPGRADE_CARD_ROW0_Y         180
#define UPGRADE_CARD_ROW1_Y         202
#define UPGRADE_CARD_ROW2_Y         222
#define UPGRADE_CARD_LABEL_X        28
#define UPGRADE_CARD_VALUE_X        (GUI_SCREEN_WIDTH - 15)

#define UPGRADE_BTN_Y               (GUI_SCREEN_HEIGHT - 36)

#define UPGRADE_COLOR_LABEL         make_color(0x99, 0x99, 0x99)

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
    char cur_ver[16];       // 如 "v1.2.0"
    char new_ver[16];       // 如 "v1.3.0"
    char file_name[24];     // 如 "NewQ_Update.bin"
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_back;
    compo_picturebox_t *pic_icon;
    compo_picturebox_t *pic_card;
    compo_picturebox_t *pic_btn;
    compo_textbox_t *txt_cur;
    compo_textbox_t *txt_new;
    compo_textbox_t *txt_file;
} f_upgrade_t;

static u8 upgrade_bat_level_from_percent(u8 percent)
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

static void upgrade_update_battery(void)
{
    f_upgrade_t *f = (f_upgrade_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = upgrade_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void upgrade_right_align(compo_textbox_t *txt, s16 y)
{
    s16 w;

    if (txt == NULL) {
        return;
    }
    w = compo_textbox_get_wid(txt);
    compo_textbox_set_location(txt, UPGRADE_CARD_VALUE_X - w, y, 0, 0);
}

static void upgrade_update_display(void)
{
    f_upgrade_t *f = (f_upgrade_t *)func_cb.f_cb;

    if (f == NULL) {
        return;
    }

    if (f->pic_btn) {
        compo_picturebox_set(f->pic_btn, f->btn_press
            ? UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_CLICK_BIN
            : UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    }

    if (f->txt_cur) {
        compo_textbox_set(f->txt_cur, f->cur_ver);
        upgrade_right_align(f->txt_cur, UPGRADE_CARD_ROW0_Y);
    }
    if (f->txt_new) {
        compo_textbox_set(f->txt_new, f->new_ver);
        upgrade_right_align(f->txt_new, UPGRADE_CARD_ROW1_Y);
    }
    if (f->txt_file) {
        compo_textbox_set(f->txt_file, f->file_name);
        upgrade_right_align(f->txt_file, UPGRADE_CARD_ROW2_Y);
    }
}

compo_form_t *func_upgrade_page_form_create(void)
{
    f_upgrade_t *f = (f_upgrade_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider + 返回 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, UPGRADE_DIVIDER_Y);

    f->pic_back = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACK_BIN);
    compo_picturebox_set_pos(f->pic_back, UPGRADE_BACK_X, UPGRADE_TITLE_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, UPGRADE_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_SYS_UPGRADE]);

    f->bat_level = upgrade_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, UPGRADE_BAT_X, UPGRADE_TITLE_Y);

    /* 升级图标 */
    f->pic_icon = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_UPGRADE_BIN);
    compo_picturebox_set_pos(f->pic_icon, GUI_SCREEN_CENTER_X, UPGRADE_ICON_Y);

    /* 发现新版本 */
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, UPGRADE_STA_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_NEW_VERSION_FOUND]);

    /* 升级文件已通过校验 */
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, UPGRADE_TIP_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, UPGRADE_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_UPDATE_VERIFIED]);

    /* 版本信息卡 */
    f->pic_card = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_GROUP_5_BIN);
    compo_picturebox_set_pos(f->pic_card, GUI_SCREEN_CENTER_X, UPGRADE_CARD_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, UPGRADE_CARD_LABEL_X, UPGRADE_CARD_ROW0_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, UPGRADE_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_CUR_VERSION]);

    f->txt_cur = compo_textbox_create(frm, 16);
    compo_textbox_set_location(f->txt_cur, UPGRADE_CARD_VALUE_X, UPGRADE_CARD_ROW0_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_cur, true);
    compo_textbox_set_align_center(f->txt_cur, false);
    compo_textbox_set_font(f->txt_cur, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, UPGRADE_CARD_LABEL_X, UPGRADE_CARD_ROW1_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, UPGRADE_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_NEW_VERSION]);

    f->txt_new = compo_textbox_create(frm, 16);
    compo_textbox_set_location(f->txt_new, UPGRADE_CARD_VALUE_X, UPGRADE_CARD_ROW1_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_new, true);
    compo_textbox_set_align_center(f->txt_new, false);
    compo_textbox_set_font(f->txt_new, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, UPGRADE_CARD_LABEL_X, UPGRADE_CARD_ROW2_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, UPGRADE_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_UPDATE_FILE]);

    f->txt_file = compo_textbox_create(frm, 24);
    compo_textbox_set_location(f->txt_file, UPGRADE_CARD_VALUE_X, UPGRADE_CARD_ROW2_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_file, true);
    compo_textbox_set_align_center(f->txt_file, false);
    compo_textbox_set_font(f->txt_file, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    /* 底部开始升级 */
    f->pic_btn = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_btn, GUI_SCREEN_CENTER_X, UPGRADE_BTN_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, UPGRADE_BTN_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
    compo_textbox_set(txt, i18n[STR_START_UPGRADE]);

    f->btn_press = 0;
    upgrade_update_display();

    return frm;
}

static void func_upgrade_page_process(void)
{
    f_upgrade_t *f = (f_upgrade_t *)func_cb.f_cb;

    if (f->btn_press && !ctp_is_touch()) {
        f->btn_press = 0;
        upgrade_update_display();
    }
    upgrade_update_battery();
    func_process();
}

static void func_upgrade_page_message(size_msg_t msg)
{
    f_upgrade_t *f = (f_upgrade_t *)func_cb.f_cb;
    point_t pt;

    switch (msg)
    {
    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        if (pt.y > (UPGRADE_BTN_Y - 30)) {
            f->btn_press = 1;
            upgrade_update_display();
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG_LIFT:
        if (f->btn_press) {
            f->btn_press = 0;
            upgrade_update_display();
        }
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        if (pt.x < 48 && pt.y < 48) {
            func_cb.sta = FUNC_SETUP_PAGE;
            break;
        }
        if (pt.y > (UPGRADE_BTN_Y - 30)) {
            f->btn_press = 0;
            upgrade_update_display();
            func_cb.sta = FUNC_UPGRADEING_PAGE;
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

void func_upgrade_page_enter(void)
{
    f_upgrade_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_upgrade_t));
    f = (f_upgrade_t *)func_cb.f_cb;

    /* 演示数据，后续由升级检测逻辑填充 */
    strcpy(f->cur_ver, "v1.2.0");
    strcpy(f->new_ver, "v1.3.0");
    strcpy(f->file_name, "NewQ_Update.bin");

    func_cb.frm_main = func_upgrade_page_form_create();
}

void func_upgrade_page_exit(void)
{
    if (func_cb.sta != FUNC_UPGRADEING_PAGE) {
        func_cb.last = FUNC_SETUP_PAGE;
    }
}

void func_upgrade_page(void)
{
    printf("%s\n", __func__);
    func_upgrade_page_enter();
    while (func_cb.sta == FUNC_UPGRADE_PAGE)
    {
        func_upgrade_page_process();
        func_upgrade_page_message(msg_dequeue());
    }
    func_upgrade_page_exit();
}
