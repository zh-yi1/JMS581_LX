#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define DONE_DIVIDER_Y              22
#define DONE_TITLE_Y                22
#define DONE_BAT_X                  (GUI_SCREEN_WIDTH - 22)

#define DONE_ICON_Y                 100

#define DONE_STA_Y                  150
#define DONE_SIZE_Y                 180
#define DONE_FILE_Y                 200
#define DONE_PATH_Y                 220

#define DONE_BTN_Y                  (GUI_SCREEN_HEIGHT - 36)

#define DONE_COLOR_LABEL            make_color(0x99, 0x99, 0x99)

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
    u8 card_total;
    u32 file_cnt;
    char size_str[16];      // 如 "32 GB"
    char dir_name[JMS581_DIR_NAME_MAX];     // 如 "CARD_014"
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_ok;
    compo_picturebox_t *pic_btn;
    compo_textbox_t *txt_size;
    compo_textbox_t *txt_file;
    compo_textbox_t *txt_path;
} f_done_t;

/* 字节 → "1.42 TB"/"512.00 GB"/"800 MB" 字符串 */
static void done_fmt_bytes(u64 bytes, char *buf)
{
    if (bytes >= ((u64)1 << 40)) {
        sprintf(buf, "%u.%02u TB", (unsigned int)(bytes >> 40),
                (unsigned int)(((bytes & (((u64)1 << 40) - 1)) * 100) >> 40));
    } else if (bytes >= ((u64)1 << 30)) {
        sprintf(buf, "%u.%02u GB", (unsigned int)(bytes >> 30),
                (unsigned int)(((bytes & (((u64)1 << 30) - 1)) * 100) >> 30));
    } else if (bytes >= ((u64)1 << 20)) {
        sprintf(buf, "%u.%02u MB", (unsigned int)(bytes >> 20),
                (unsigned int)(((bytes & (((u64)1 << 20) - 1)) * 100) >> 20));
    } else {
        sprintf(buf, "%u KB", (unsigned int)(bytes >> 10));
    }
}

static u8 done_bat_level_from_percent(u8 percent)
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

static void done_update_battery(void)
{
    f_done_t *f = (f_done_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = done_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void done_update_display(void)
{
    f_done_t *f = (f_done_t *)func_cb.f_cb;
    char buf[48];

    if (f == NULL) {
        return;
    }

    if (f->pic_btn) {
        compo_picturebox_set(f->pic_btn, f->btn_press
            ? UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_CLICK_BIN
            : UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    }

    if (f->txt_size) {
        sprintf(buf, "%s %s", i18n[STR_BACKED_UP], f->size_str);
        compo_textbox_set(f->txt_size, buf);
    }

    if (f->txt_file) {
        if (f->file_cnt >= 1000) {
            sprintf(buf, "%lu,%03lu %s · %u %s",
                    (unsigned long)(f->file_cnt / 1000),
                    (unsigned long)(f->file_cnt % 1000),
                    i18n[STR_FILE_UNIT],
                    f->card_total, i18n[STR_CARD_UNIT]);
        } else {
            sprintf(buf, "%lu %s · %u %s",
                    (unsigned long)f->file_cnt, i18n[STR_FILE_UNIT],
                    f->card_total, i18n[STR_CARD_UNIT]);
        }
        compo_textbox_set(f->txt_file, buf);
    }

    if (f->txt_path) {
        sprintf(buf, "%s %s", i18n[STR_SAVE_TO], f->dir_name);
        compo_textbox_set(f->txt_path, buf);
    }
}

compo_form_t *func_whole_card_done_page_form_create(void)
{
    f_done_t *f = (f_done_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider 底 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, DONE_DIVIDER_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, DONE_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_FULL_CARD_BACKUP]);

    f->bat_level = done_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, DONE_BAT_X, DONE_TITLE_Y);

    /* 中间：完成勾选图标 */
    f->pic_ok = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_CHECK_OUT_CORRECT_BIN);
    compo_picturebox_set_pos(f->pic_ok, GUI_SCREEN_CENTER_X, DONE_ICON_Y);

    /* 备份完成 */
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, DONE_STA_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_BACKUP_DONE]);

    /* 统计信息 */
    f->txt_size = compo_textbox_create(frm, 24);
    compo_textbox_set_location(f->txt_size, GUI_SCREEN_CENTER_X, DONE_SIZE_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_size, true);
    compo_textbox_set_align_center(f->txt_size, true);
    compo_textbox_set_font(f->txt_size, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    f->txt_file = compo_textbox_create(frm, 32);
    compo_textbox_set_location(f->txt_file, GUI_SCREEN_CENTER_X, DONE_FILE_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_file, true);
    compo_textbox_set_align_center(f->txt_file, true);
    compo_textbox_set_font(f->txt_file, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    f->txt_path = compo_textbox_create(frm, 32);
    compo_textbox_set_location(f->txt_path, GUI_SCREEN_CENTER_X, DONE_PATH_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_path, true);
    compo_textbox_set_align_center(f->txt_path, true);
    compo_textbox_set_font(f->txt_path, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(f->txt_path, DONE_COLOR_LABEL);

    /* 底部完成按钮 */
    f->pic_btn = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_LONG_BOTTON_BLUE_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_btn, GUI_SCREEN_CENTER_X, DONE_BTN_Y);

    txt = compo_textbox_create(frm, 8);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, DONE_BTN_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
    compo_textbox_set(txt, i18n[STR_DONE]);

    f->btn_press = 0;
    done_update_display();

    return frm;
}

static void func_whole_card_done_page_process(void)
{
    f_done_t *f = (f_done_t *)func_cb.f_cb;

    if (f->btn_press && !ctp_is_touch()) {
        f->btn_press = 0;
        done_update_display();
    }
    done_update_battery();
    func_process();
}

static void func_whole_card_done_page_message(size_msg_t msg)
{
    f_done_t *f = (f_done_t *)func_cb.f_cb;
    point_t pt;

    switch (msg)
    {
    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        if (pt.y > (DONE_BTN_Y - 30)) {
            f->btn_press = 1;
            done_update_display();
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG_LIFT:
        if (f->btn_press) {
            f->btn_press = 0;
            done_update_display();
        }
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        if (pt.y > (DONE_BTN_Y - 30)) {
            f->btn_press = 0;
            done_update_display();
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

void func_whole_card_done_page_enter(void)
{
    f_done_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_done_t));
    f = (f_done_t *)func_cb.f_cb;

    f->card_total = backup_param.bk_card_cnt;
    f->file_cnt = backup_param.bk_file_total;
    done_fmt_bytes(backup_param.bk_size_bytes, f->size_str);
    strcpy(f->dir_name, backup_param.dir_sel);

    func_cb.frm_main = func_whole_card_done_page_form_create();
}

void func_whole_card_done_page_exit(void)
{
    func_cb.last = FUNC_HOME_PAGE;
}

void func_whole_card_done_page(void)
{
    printf("%s\n", __func__);
    func_whole_card_done_page_enter();
    while (func_cb.sta == FUNC_WHOLE_CARD_DONE_PAGE)
    {
        func_whole_card_done_page_process();
        func_whole_card_done_page_message(msg_dequeue());
    }
    func_whole_card_done_page_exit();
}
