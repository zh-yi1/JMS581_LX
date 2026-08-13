#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define BACKUP1_DIVIDER_Y           22
#define BACKUP1_TITLE_Y             22
#define BACKUP1_BAT_X               (GUI_SCREEN_WIDTH - 22)

#define BACKUP1_CARD_ROW_Y          52
#define BACKUP1_CARD_ROW_X          16
#define BACKUP1_CARD_NAME_X         (GUI_SCREEN_WIDTH - 16)

#define BACKUP1_CUR_LABEL_Y         88
#define BACKUP1_PERCENT_Y           120
#define BACKUP1_SIZE_Y              148
#define BACKUP1_BAR_Y               172

#define BACKUP1_PATH_BOX_Y          256
#define BACKUP1_PATH_LINE0_Y        235
#define BACKUP1_PATH_LINE1_Y        264

#define BACKUP1_WARN_Y              (GUI_SCREEN_HEIGHT - 18)

#define BACKUP1_COLOR_LABEL         make_color(0x99, 0x99, 0x99)
#define BACKUP1_COLOR_VOL           make_color(0x4A, 0x9E, 0xFF)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

/* 0%~100% 进度条图 */
static const u32 s_progress_res[101] = {
    UI_BUF_IMAGE_BIN_0_BIN,
    UI_BUF_IMAGE_BIN_1_BIN,
    UI_BUF_IMAGE_BIN_2_BIN,
    UI_BUF_IMAGE_BIN_3_BIN,
    UI_BUF_IMAGE_BIN_4_BIN,
    UI_BUF_IMAGE_BIN_5_BIN,
    UI_BUF_IMAGE_BIN_6_BIN,
    UI_BUF_IMAGE_BIN_7_BIN,
    UI_BUF_IMAGE_BIN_8_BIN,
    UI_BUF_IMAGE_BIN_9_BIN,
    UI_BUF_IMAGE_BIN_10_BIN,
    UI_BUF_IMAGE_BIN_11_BIN,
    UI_BUF_IMAGE_BIN_12_BIN,
    UI_BUF_IMAGE_BIN_13_BIN,
    UI_BUF_IMAGE_BIN_14_BIN,
    UI_BUF_IMAGE_BIN_15_BIN,
    UI_BUF_IMAGE_BIN_16_BIN,
    UI_BUF_IMAGE_BIN_17_BIN,
    UI_BUF_IMAGE_BIN_18_BIN,
    UI_BUF_IMAGE_BIN_19_BIN,
    UI_BUF_IMAGE_BIN_20_BIN,
    UI_BUF_IMAGE_BIN_21_BIN,
    UI_BUF_IMAGE_BIN_22_BIN,
    UI_BUF_IMAGE_BIN_23_BIN,
    UI_BUF_IMAGE_BIN_24_BIN,
    UI_BUF_IMAGE_BIN_25_BIN,
    UI_BUF_IMAGE_BIN_26_BIN,
    UI_BUF_IMAGE_BIN_27_BIN,
    UI_BUF_IMAGE_BIN_28_BIN,
    UI_BUF_IMAGE_BIN_29_BIN,
    UI_BUF_IMAGE_BIN_30_BIN,
    UI_BUF_IMAGE_BIN_31_BIN,
    UI_BUF_IMAGE_BIN_32_BIN,
    UI_BUF_IMAGE_BIN_33_BIN,
    UI_BUF_IMAGE_BIN_34_BIN,
    UI_BUF_IMAGE_BIN_35_BIN,
    UI_BUF_IMAGE_BIN_36_BIN,
    UI_BUF_IMAGE_BIN_37_BIN,
    UI_BUF_IMAGE_BIN_38_BIN,
    UI_BUF_IMAGE_BIN_39_BIN,
    UI_BUF_IMAGE_BIN_40_BIN,
    UI_BUF_IMAGE_BIN_41_BIN,
    UI_BUF_IMAGE_BIN_42_BIN,
    UI_BUF_IMAGE_BIN_43_BIN,
    UI_BUF_IMAGE_BIN_44_BIN,
    UI_BUF_IMAGE_BIN_45_BIN,
    UI_BUF_IMAGE_BIN_46_BIN,
    UI_BUF_IMAGE_BIN_47_BIN,
    UI_BUF_IMAGE_BIN_48_BIN,
    UI_BUF_IMAGE_BIN_49_BIN,
    UI_BUF_IMAGE_BIN_50_BIN,
    UI_BUF_IMAGE_BIN_51_BIN,
    UI_BUF_IMAGE_BIN_52_BIN,
    UI_BUF_IMAGE_BIN_53_BIN,
    UI_BUF_IMAGE_BIN_54_BIN,
    UI_BUF_IMAGE_BIN_55_BIN,
    UI_BUF_IMAGE_BIN_56_BIN,
    UI_BUF_IMAGE_BIN_57_BIN,
    UI_BUF_IMAGE_BIN_58_BIN,
    UI_BUF_IMAGE_BIN_59_BIN,
    UI_BUF_IMAGE_BIN_60_BIN,
    UI_BUF_IMAGE_BIN_61_BIN,
    UI_BUF_IMAGE_BIN_62_BIN,
    UI_BUF_IMAGE_BIN_63_BIN,
    UI_BUF_IMAGE_BIN_64_BIN,
    UI_BUF_IMAGE_BIN_65_BIN,
    UI_BUF_IMAGE_BIN_66_BIN,
    UI_BUF_IMAGE_BIN_67_BIN,
    UI_BUF_IMAGE_BIN_68_BIN,
    UI_BUF_IMAGE_BIN_69_BIN,
    UI_BUF_IMAGE_BIN_70_BIN,
    UI_BUF_IMAGE_BIN_71_BIN,
    UI_BUF_IMAGE_BIN_72_BIN,
    UI_BUF_IMAGE_BIN_73_BIN,
    UI_BUF_IMAGE_BIN_74_BIN,
    UI_BUF_IMAGE_BIN_75_BIN,
    UI_BUF_IMAGE_BIN_76_BIN,
    UI_BUF_IMAGE_BIN_77_BIN,
    UI_BUF_IMAGE_BIN_78_BIN,
    UI_BUF_IMAGE_BIN_79_BIN,
    UI_BUF_IMAGE_BIN_80_BIN,
    UI_BUF_IMAGE_BIN_81_BIN,
    UI_BUF_IMAGE_BIN_82_BIN,
    UI_BUF_IMAGE_BIN_83_BIN,
    UI_BUF_IMAGE_BIN_84_BIN,
    UI_BUF_IMAGE_BIN_85_BIN,
    UI_BUF_IMAGE_BIN_86_BIN,
    UI_BUF_IMAGE_BIN_87_BIN,
    UI_BUF_IMAGE_BIN_88_BIN,
    UI_BUF_IMAGE_BIN_89_BIN,
    UI_BUF_IMAGE_BIN_90_BIN,
    UI_BUF_IMAGE_BIN_91_BIN,
    UI_BUF_IMAGE_BIN_92_BIN,
    UI_BUF_IMAGE_BIN_93_BIN,
    UI_BUF_IMAGE_BIN_94_BIN,
    UI_BUF_IMAGE_BIN_95_BIN,
    UI_BUF_IMAGE_BIN_96_BIN,
    UI_BUF_IMAGE_BIN_97_BIN,
    UI_BUF_IMAGE_BIN_98_BIN,
    UI_BUF_IMAGE_BIN_99_BIN,
    UI_BUF_IMAGE_BIN_100_BIN,
};

typedef struct {
    u8 bat_level;
    u8 percent;             // 0~100
    u8 card_idx;            // 当前第几张，从 1 开始
    u8 card_total;
    char card_name[8];
    char size_done[16];     // 如 "15.0 GB"
    char size_total[16];    // 如 "32 GB"
    char path_dir[32];
    char path_vol[24];
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_bar;
    compo_picturebox_t *pic_path;
    compo_textbox_t *txt_card_row;
    compo_textbox_t *txt_card_name;
    compo_textbox_t *txt_percent;
    compo_textbox_t *txt_size_done;
    compo_textbox_t *txt_size_total;
} f_backup1_t;

static u8 backup1_bat_level_from_percent(u8 percent)
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

static void backup1_update_battery(void)
{
    f_backup1_t *f = (f_backup1_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = backup1_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void backup1_update_display(void)
{
    f_backup1_t *f = (f_backup1_t *)func_cb.f_cb;
    char buf[48];
    s16 card_w;
    s16 done_w;
    s16 total_w;
    s16 start_x;
    u8 pct;

    if (f == NULL) {
        return;
    }

    pct = (f->percent > 100) ? 100 : f->percent;

    if (f->txt_card_row) {
        sprintf(buf, "%s %u / %u %s",
                i18n[STR_CARD_NO], f->card_idx, f->card_total, i18n[STR_CARD_SHEET]);
        compo_textbox_set(f->txt_card_row, buf);
    }

    if (f->txt_card_name) {
        compo_textbox_set(f->txt_card_name, f->card_name);
        card_w = compo_textbox_get_wid(f->txt_card_name);
        compo_textbox_set_location(f->txt_card_name,
                                   BACKUP1_CARD_NAME_X - card_w, BACKUP1_CARD_ROW_Y, 0, 0);
    }

    if (f->txt_percent) {
        sprintf(buf, "%u%%", pct);
        compo_textbox_set(f->txt_percent, buf);
    }

    if (f->txt_size_done && f->txt_size_total) {
        sprintf(buf, "%s / ", f->size_done);
        compo_textbox_set(f->txt_size_done, buf);
        compo_textbox_set(f->txt_size_total, f->size_total);
        done_w = compo_textbox_get_wid(f->txt_size_done);
        total_w = compo_textbox_get_wid(f->txt_size_total);
        start_x = GUI_SCREEN_CENTER_X - (done_w + total_w) / 2;
        compo_textbox_set_location(f->txt_size_done, start_x, BACKUP1_SIZE_Y, 0, 0);
        compo_textbox_set_location(f->txt_size_total, start_x + done_w, BACKUP1_SIZE_Y, 0, 0);
    }

    if (f->pic_bar) {
        compo_picturebox_set(f->pic_bar, s_progress_res[pct]);
    }
}

compo_form_t *func_backing_up_1_page_form_create(void)
{
    f_backup1_t *f = (f_backup1_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider 底 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, BACKUP1_DIVIDER_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, BACKUP1_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_FULL_CARD_BACKUP]);

    f->bat_level = backup1_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, BACKUP1_BAT_X, BACKUP1_TITLE_Y);

    /* 第 N / M 张 + 卡名 */
    f->txt_card_row = compo_textbox_create(frm, 24);
    compo_textbox_set_location(f->txt_card_row, BACKUP1_CARD_ROW_X, BACKUP1_CARD_ROW_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_card_row, true);
    compo_textbox_set_align_center(f->txt_card_row, false);
    compo_textbox_set_font(f->txt_card_row, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    f->txt_card_name = compo_textbox_create(frm, 8);
    compo_textbox_set_location(f->txt_card_name, BACKUP1_CARD_NAME_X, BACKUP1_CARD_ROW_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_card_name, true);
    compo_textbox_set_align_center(f->txt_card_name, false);
    compo_textbox_set_font(f->txt_card_name, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    /* 当前卡 */
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, BACKUP1_CUR_LABEL_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, BACKUP1_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_CURRENT_CARD]);

    /* 百分比 */
    f->txt_percent = compo_textbox_create(frm, 8);
    compo_textbox_set_location(f->txt_percent, GUI_SCREEN_CENTER_X, BACKUP1_PERCENT_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_percent, true);
    compo_textbox_set_align_center(f->txt_percent, true);
    compo_textbox_set_font(f->txt_percent, UI_BUF_FONT_BIN_FONT_SIZE_22_BIN);

    /* 已备份 / 总量 */
    f->txt_size_done = compo_textbox_create(frm, 24);
    compo_textbox_set_autosize(f->txt_size_done, true);
    compo_textbox_set_align_center(f->txt_size_done, false);
    compo_textbox_set_font(f->txt_size_done, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    f->txt_size_total = compo_textbox_create(frm, 16);
    compo_textbox_set_autosize(f->txt_size_total, true);
    compo_textbox_set_align_center(f->txt_size_total, false);
    compo_textbox_set_font(f->txt_size_total, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(f->txt_size_total, BACKUP1_COLOR_LABEL);

    /* 进度条：0.bin ~ 100.bin */
    f->pic_bar = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_0_BIN);
    compo_picturebox_set_pos(f->pic_bar, GUI_SCREEN_CENTER_X, BACKUP1_BAR_Y);

    /* 底部路径卡片 */
    f->pic_path = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_GROUP_3_BIN);
    compo_picturebox_set_pos(f->pic_path, GUI_SCREEN_CENTER_X, BACKUP1_PATH_BOX_Y);

    {
        compo_textbox_t *txt_save;
        s16 path_x;

        txt_save = compo_textbox_create(frm, 16);
        compo_textbox_set_location(txt_save, 20, BACKUP1_PATH_LINE0_Y, 0, 0);
        compo_textbox_set_autosize(txt_save, true);
        compo_textbox_set_align_center(txt_save, false);
        compo_textbox_set_font(txt_save, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set(txt_save, i18n[STR_SAVE_TO]);

        path_x = 20 + compo_textbox_get_wid(txt_save) + 4;
        txt = compo_textbox_create(frm, 32);
        compo_textbox_set_location(txt, path_x, BACKUP1_PATH_LINE0_Y, 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set(txt, f->path_dir);
    }

    {
        char buf[40];

        sprintf(buf, "└─ %s", f->path_vol);
        txt = compo_textbox_create(frm, 32);
        compo_textbox_set_location(txt, 20, BACKUP1_PATH_LINE1_Y, 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(txt, BACKUP1_COLOR_VOL);
        compo_textbox_set(txt, buf);
    }

    /* 底部提示 */
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, BACKUP1_WARN_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set(txt, i18n[STR_DO_NOT_REMOVE_MEDIA]);

    backup1_update_display();

    return frm;
}

static void func_backing_up_1_page_process(void)
{
    backup1_update_battery();
    func_process();
}

static void func_backing_up_1_page_message(size_msg_t msg)
{
    switch (msg)
    {
    case KU_BACK:
        func_cb.sta = FUNC_LOADING_1_PAGE;
        break;

    default:
        //func_message(msg);
        break;
    }
}

void func_backing_up_1_page_enter(void)
{
    f_backup1_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_backup1_t));
    f = (f_backup1_t *)func_cb.f_cb;

    f->percent = 47;
    f->card_idx = 1;
    f->card_total = 2;
    strcpy(f->card_name, "SD");
    strcpy(f->size_done, "15.0 GB");
    strcpy(f->size_total, "32 GB");
    strcpy(f->path_dir, "CARD_BACKUP/CARD_014");
    strcpy(f->path_vol, "SD_128G_A1B2");

    if (backup_param.card_sel[0]) {
        const char *p = backup_param.card_sel;
        u8 i = 0;

        while (*p == ' ') {
            p++;
        }
        while (*p && *p != ' ' && i < sizeof(f->card_name) - 1) {
            f->card_name[i++] = *p++;
        }
        f->card_name[i] = '\0';
    }
    if (backup_param.dir_sel[0]) {
        sprintf(f->path_dir, "CARD_BACKUP/%s", backup_param.dir_sel);
    }

    func_cb.frm_main = func_backing_up_1_page_form_create();
}

void func_backing_up_1_page_exit(void)
{
    func_cb.last = FUNC_LOADING_1_PAGE;
}

void func_backing_up_1_page(void)
{
    printf("%s\n", __func__);
    func_backing_up_1_page_enter();
    while (func_cb.sta == FUNC_BACKING_UP_1_PAGE)
    {
        func_backing_up_1_page_process();
        func_backing_up_1_page_message(msg_dequeue());
    }
    func_backing_up_1_page_exit();
}
