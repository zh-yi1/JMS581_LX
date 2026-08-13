#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define COMPUTER_ITEM_CNT           2

#define COMPUTER_DIVIDER_Y          22
#define COMPUTER_TITLE_Y            22
#define COMPUTER_BAT_X              (GUI_SCREEN_WIDTH - 22)

#define COMPUTER_ROW0_Y             120
#define COMPUTER_ROW_H              72
#define COMPUTER_ROW_GAP            12

#define COMPUTER_TEXT_X             28
#define COMPUTER_TITLE_Y_OFS        (-12)
#define COMPUTER_SUB_Y_OFS          (10)

#define COMPUTER_COLOR_SUB          make_color(0x99, 0x99, 0x99)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

/* 主标题为固定英文目录名，不随界面语言切换 */
static const char *s_item_name[COMPUTER_ITEM_CNT] = {
    "CARD_BACKUP",
    "RECENT_BACKUP",
};

static const u32 s_item_sub[COMPUTER_ITEM_CNT] = {
    STR_COMPUTER_CARD_TIP,
    STR_COMPUTER_RECENT_TIP,
};

typedef struct {
    u8 bat_level;
    u8 press_idx;   // 0~1 按下中的项，0xFF=无
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_row[COMPUTER_ITEM_CNT];
    compo_textbox_t *txt_name[COMPUTER_ITEM_CNT];
    compo_textbox_t *txt_sub[COMPUTER_ITEM_CNT];
} f_computer_t;

static u8 computer_hit_row(point_t pt)
{
    u8 i;

    for (i = 0; i < COMPUTER_ITEM_CNT; i++) {
        s16 y = COMPUTER_ROW0_Y + i * (COMPUTER_ROW_H + COMPUTER_ROW_GAP);
        if (pt.y > (y - COMPUTER_ROW_H / 2) && pt.y < (y + COMPUTER_ROW_H / 2)) {
            return i;
        }
    }
    return 0xFF;
}

static void computer_set_row_press(f_computer_t *f, u8 idx)
{
    f->press_idx = idx;
}

static void computer_clear_press(f_computer_t *f)
{
    computer_set_row_press(f, 0xFF);
}

static u8 computer_bat_level_from_percent(u8 percent)
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

static void computer_update_battery(void)
{
    f_computer_t *f = (f_computer_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = computer_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void computer_update_display(void)
{
    f_computer_t *f = (f_computer_t *)func_cb.f_cb;
    u8 i;

    if (f == NULL) {
        return;
    }

    for (i = 0; i < COMPUTER_ITEM_CNT; i++) {
        /* 中间卡片背景：computer.bin */
        compo_picturebox_set(f->pic_row[i], UI_BUF_IMAGE_BIN_COMPUTER_BIN);
        compo_textbox_set(f->txt_name[i], s_item_name[i]);
        compo_textbox_set(f->txt_sub[i], i18n[s_item_sub[i]]);
    }
}

compo_form_t *func_computer_page_form_create(void)
{
    f_computer_t *f = (f_computer_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    u8 i;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, COMPUTER_DIVIDER_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, COMPUTER_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_COMPUTER_MODE]);

    f->bat_level = computer_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, COMPUTER_BAT_X, COMPUTER_TITLE_Y);

    /* 中间两项：computer.bin 卡片 + 主/副标题 */
    for (i = 0; i < COMPUTER_ITEM_CNT; i++) {
        s16 y = COMPUTER_ROW0_Y + i * (COMPUTER_ROW_H + COMPUTER_ROW_GAP);

        f->pic_row[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_COMPUTER_BIN);
        compo_picturebox_set_pos(f->pic_row[i], GUI_SCREEN_CENTER_X, y);

        f->txt_name[i] = compo_textbox_create(frm, 24);
        compo_textbox_set_location(f->txt_name[i],
                                   COMPUTER_TEXT_X, y + COMPUTER_TITLE_Y_OFS, 0, 0);
        compo_textbox_set_autosize(f->txt_name[i], true);
        compo_textbox_set_align_center(f->txt_name[i], false);
        compo_textbox_set_font(f->txt_name[i], UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);

        f->txt_sub[i] = compo_textbox_create(frm, 24);
        compo_textbox_set_location(f->txt_sub[i],
                                   COMPUTER_TEXT_X, y + COMPUTER_SUB_Y_OFS, 0, 0);
        compo_textbox_set_autosize(f->txt_sub[i], true);
        compo_textbox_set_align_center(f->txt_sub[i], false);
        compo_textbox_set_font(f->txt_sub[i], UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(f->txt_sub[i], COMPUTER_COLOR_SUB);
    }

    f->press_idx = 0xFF;
    computer_update_display();

    return frm;
}

static void func_computer_page_process(void)
{
    f_computer_t *f = (f_computer_t *)func_cb.f_cb;

    if (f->press_idx != 0xFF && !ctp_is_touch()) {
        computer_clear_press(f);
    }
    computer_update_battery();
    func_process();
}

static void func_computer_page_message(size_msg_t msg)
{
    f_computer_t *f = (f_computer_t *)func_cb.f_cb;
    point_t pt;
    u8 idx;

    switch (msg)
    {
    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        idx = computer_hit_row(pt);
        if (idx != 0xFF) {
            computer_set_row_press(f, idx);
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG_LIFT:
        computer_clear_press(f);
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        computer_clear_press(f);
        idx = computer_hit_row(pt);
        if (idx != 0xFF) {
            TRACE("computer item %u\n", idx);
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

void func_computer_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_computer_t));
    func_cb.frm_main = func_computer_page_form_create();
}

void func_computer_page_exit(void)
{
    func_cb.last = FUNC_HOME_PAGE;
}

void func_computer_page(void)
{
    printf("%s\n", __func__);
    func_computer_page_enter();
    while (func_cb.sta == FUNC_COMPUTER_PAGE)
    {
        func_computer_page_process();
        func_computer_page_message(msg_dequeue());
    }
    func_computer_page_exit();
}
