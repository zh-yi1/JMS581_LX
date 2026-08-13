#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define VERINFO_ITEM_CNT            5

#define VERINFO_DIVIDER_Y           22
#define VERINFO_ICON_Y              22
#define VERINFO_BACK_X              18
#define VERINFO_BAT_X               (GUI_SCREEN_WIDTH - 22)

#define VERINFO_ROW0_Y              80
#define VERINFO_ROW_H               48
#define VERINFO_ROW_GAP             0
#define VERINFO_TEXT_Y_OFS          (-12)   /* 相对行中心上移 */

#define VERINFO_LABEL_X             16
#define VERINFO_VALUE_RIGHT_X       (GUI_SCREEN_WIDTH - 16)

#define VERINFO_COLOR_LABEL         make_color(0x99, 0x99, 0x99)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

static const u32 s_item_label[VERINFO_ITEM_CNT] = {
    STR_PRODUCT_MODEL,
    STR_FW_VERSION,
    STR_SERIAL_NUM,
    STR_SSD_CAPACITY,
    STR_UI_LANGUAGE,
};

typedef struct {
    u8 bat_level;
    char model[16];
    char fw_ver[16];
    char serial[24];
    char ssd_cap[16];
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_back;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_row[VERINFO_ITEM_CNT];
    compo_textbox_t *txt_label[VERINFO_ITEM_CNT];
    compo_textbox_t *txt_value[VERINFO_ITEM_CNT];
} f_verinfo_t;

static u8 verinfo_bat_level_from_percent(u8 percent)
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

static void verinfo_update_battery(void)
{
    f_verinfo_t *f = (f_verinfo_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = verinfo_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static const char *verinfo_value_str(f_verinfo_t *f, u8 idx)
{
    switch (idx) {
    case 0: return f->model;
    case 1: return f->fw_ver;
    case 2: return f->serial;
    case 3: return f->ssd_cap;
    case 4: return i18n[STR_LANG_ZH_CN];
    default: return "";
    }
}

static void verinfo_update_display(void)
{
    f_verinfo_t *f = (f_verinfo_t *)func_cb.f_cb;
    u8 i;
    s16 y;
    s16 w;

    if (f == NULL) {
        return;
    }

    for (i = 0; i < VERINFO_ITEM_CNT; i++) {
        y = VERINFO_ROW0_Y + i * (VERINFO_ROW_H + VERINFO_ROW_GAP);

        compo_picturebox_set(f->pic_row[i], UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
        compo_textbox_set(f->txt_label[i], i18n[s_item_label[i]]);
        compo_textbox_set_location(f->txt_label[i],
                                   VERINFO_LABEL_X, y + VERINFO_TEXT_Y_OFS, 0, 0);

        compo_textbox_set(f->txt_value[i], verinfo_value_str(f, i));
        w = compo_textbox_get_wid(f->txt_value[i]);
        compo_textbox_set_location(f->txt_value[i],
                                   VERINFO_VALUE_RIGHT_X - w, y + VERINFO_TEXT_Y_OFS, 0, 0);
    }
}

compo_form_t *func_verinfo_page_form_create(void)
{
    f_verinfo_t *f = (f_verinfo_t *)func_cb.f_cb;
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
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, VERINFO_DIVIDER_Y);

    f->pic_back = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACK_BIN);
    compo_picturebox_set_pos(f->pic_back, VERINFO_BACK_X, VERINFO_ICON_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, VERINFO_ICON_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_DEVICE_INFO]);

    f->bat_level = verinfo_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, VERINFO_BAT_X, VERINFO_ICON_Y);

    /* 信息列表：左标签 / 右数值 */
    for (i = 0; i < VERINFO_ITEM_CNT; i++) {
        s16 y = VERINFO_ROW0_Y + i * (VERINFO_ROW_H + VERINFO_ROW_GAP);

        f->pic_row[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
        compo_picturebox_set_pos(f->pic_row[i], GUI_SCREEN_CENTER_X, y);

        f->txt_label[i] = compo_textbox_create(frm, 16);
        compo_textbox_set_location(f->txt_label[i],
                                   VERINFO_LABEL_X, y + VERINFO_TEXT_Y_OFS, 0, 0);
        compo_textbox_set_autosize(f->txt_label[i], true);
        compo_textbox_set_align_center(f->txt_label[i], false);
        compo_textbox_set_font(f->txt_label[i], UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set_forecolor(f->txt_label[i], VERINFO_COLOR_LABEL);

        f->txt_value[i] = compo_textbox_create(frm, 24);
        compo_textbox_set_location(f->txt_value[i],
                                   VERINFO_VALUE_RIGHT_X, y + VERINFO_TEXT_Y_OFS, 0, 0);
        compo_textbox_set_autosize(f->txt_value[i], true);
        compo_textbox_set_align_center(f->txt_value[i], false);
        compo_textbox_set_font(f->txt_value[i], UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
    }

    verinfo_update_display();

    return frm;
}

static void func_verinfo_page_process(void)
{
    verinfo_update_battery();
    func_process();
}

static void func_verinfo_page_message(size_msg_t msg)
{
    point_t pt;

    switch (msg)
    {
    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        if (pt.x < 48 && pt.y < 48) {
            func_cb.sta = FUNC_SETUP_PAGE;
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

void func_verinfo_page_enter(void)
{
    f_verinfo_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_verinfo_t));
    f = (f_verinfo_t *)func_cb.f_cb;

    /* 演示数据，后续由设备信息接口填充 */
    strcpy(f->model, "NewQ");
    strcpy(f->fw_ver, "v1.2.0");
    strcpy(f->serial, "NQ26-07A1B2");
    strcpy(f->ssd_cap, "2.00 TB");

    func_cb.frm_main = func_verinfo_page_form_create();
}

void func_verinfo_page_exit(void)
{
    func_cb.last = FUNC_SETUP_PAGE;
}

void func_verinfo_page(void)
{
    printf("%s\n", __func__);
    func_verinfo_page_enter();
    while (func_cb.sta == FUNC_VERINFO_PAGE)
    {
        func_verinfo_page_process();
        func_verinfo_page_message(msg_dequeue());
    }
    func_verinfo_page_exit();
}
