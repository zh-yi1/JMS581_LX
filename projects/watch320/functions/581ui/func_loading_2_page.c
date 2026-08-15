#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define LOADING2_FRAME_CNT          26

#define LOADING2_DIVIDER_Y          22
#define LOADING2_TITLE_Y            22
#define LOADING2_BAT_X              (GUI_SCREEN_WIDTH - 22)

#define LOADING2_PROGRESS_Y         52
#define LOADING2_PROGRESS_X         16
#define LOADING2_CARD_X             (GUI_SCREEN_WIDTH - 16)

#define LOADING2_RING_Y             118
#define LOADING2_RING_SIZE          80

#define LOADING2_STA_Y              168
#define LOADING2_TIP_Y              188

#define LOADING2_PATH_BOX_Y         256
#define LOADING2_PATH_LINE0_Y       235
#define LOADING2_PATH_LINE1_Y       264

#define LOADING2_WARN_Y             (GUI_SCREEN_HEIGHT - 18)

#define LOADING2_COLOR_LABEL        make_color(0x99, 0x99, 0x99)
#define LOADING2_COLOR_VOL          make_color(0x4A, 0x9E, 0xFF)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

typedef struct {
    u8 bat_level;
    u8 days;
    u8 is_whole;        // 1=整卡流程 0=最新N日流程(决定标题/路径/跳转)
    u8 card_idx;        // 当前第几张，从 1 开始
    u8 card_total;
    char card_name[8];
    char path_dir[48];
    char path_vol[24];
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_divider;
    compo_animation_t *anim_loading;
    compo_picturebox_t *pic_path;
    compo_textbox_t *txt_title;
    compo_textbox_t *txt_progress;
    compo_textbox_t *txt_card;
} f_loading2_t;

/* 协议设备 ID → 卡名 */
static const char *loading2_card_name(u8 dev_id)
{
    switch (dev_id) {
    case JMS581_DEV_SD:  return "SD";
    case JMS581_DEV_CFA: return "CFA";
    case JMS581_DEV_CFB: return "CFB";
    default:             return "";
    }
}

static u8 loading2_bat_level_from_percent(u8 percent)
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

static void loading2_update_battery(void)
{
    f_loading2_t *f = (f_loading2_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = loading2_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void loading2_update_text(void)
{
    f_loading2_t *f = (f_loading2_t *)func_cb.f_cb;
    char buf[48];
    s16 card_w;

    if (f == NULL) {
        return;
    }

    if (f->txt_title) {
        if (f->is_whole) {
            compo_textbox_set(f->txt_title, i18n[STR_FULL_CARD_BACKUP]);
        } else {
            sprintf(buf, "%s %u %s", i18n[STR_LATEST], f->days, i18n[STR_DAYS_BACKUP]);
            compo_textbox_set(f->txt_title, buf);
        }
    }

    if (f->txt_progress) {
        sprintf(buf, "%s %u / %u %s",
                i18n[STR_CARD_NO], f->card_idx, f->card_total, i18n[STR_CARD_SHEET]);
        compo_textbox_set(f->txt_progress, buf);
    }

    if (f->txt_card) {
        compo_textbox_set(f->txt_card, f->card_name);
        card_w = compo_textbox_get_wid(f->txt_card);
        compo_textbox_set_location(f->txt_card,
                                   LOADING2_CARD_X - card_w, LOADING2_PROGRESS_Y, 0, 0);
    }
}

compo_form_t *func_loading_2_page_form_create(void)
{
    f_loading2_t *f = (f_loading2_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider 底 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, LOADING2_DIVIDER_Y);

    f->txt_title = compo_textbox_create(frm, 24);
    compo_textbox_set_location(f->txt_title, GUI_SCREEN_CENTER_X, LOADING2_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_title, true);
    compo_textbox_set_align_center(f->txt_title, true);
    compo_textbox_set_font(f->txt_title, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);

    f->bat_level = loading2_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, LOADING2_BAT_X, LOADING2_TITLE_Y);

    /* 进度：第 N / M 张 + 卡名 */
    f->txt_progress = compo_textbox_create(frm, 24);
    compo_textbox_set_location(f->txt_progress, LOADING2_PROGRESS_X, LOADING2_PROGRESS_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_progress, true);
    compo_textbox_set_align_center(f->txt_progress, false);
    compo_textbox_set_font(f->txt_progress, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    f->txt_card = compo_textbox_create(frm, 8);
    compo_textbox_set_location(f->txt_card, LOADING2_CARD_X, LOADING2_PROGRESS_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_card, true);
    compo_textbox_set_align_center(f->txt_card, false);
    compo_textbox_set_font(f->txt_card, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);

    /* 中间：loading 动画 */
    f->anim_loading = compo_animation_create(frm, UI_BUF_IMAGE_BIN_LOADING_BIN);
    compo_animation_set_radix(f->anim_loading, LOADING2_FRAME_CNT);
    compo_animation_set_interval(f->anim_loading, 5);
    compo_animation_set_pos(f->anim_loading, GUI_SCREEN_CENTER_X, LOADING2_RING_Y);
    compo_animation_set_size(f->anim_loading, LOADING2_RING_SIZE, LOADING2_RING_SIZE);

    /* 状态文案 */
    txt = compo_textbox_create(frm, 24);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LOADING2_STA_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_CHECKING_FILES]);

    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LOADING2_TIP_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, LOADING2_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_CALC_BACKUP_SIZE]);

    /* 底部路径卡片（坐标与 loading_1 一致） */
    f->pic_path = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_GROUP_3_BIN);
    compo_picturebox_set_pos(f->pic_path, GUI_SCREEN_CENTER_X, LOADING2_PATH_BOX_Y);

    {
        compo_textbox_t *txt_save;
        s16 path_x;

        txt_save = compo_textbox_create(frm, 16);
        compo_textbox_set_location(txt_save, 20, LOADING2_PATH_LINE0_Y, 0, 0);
        compo_textbox_set_autosize(txt_save, true);
        compo_textbox_set_align_center(txt_save, false);
        compo_textbox_set_font(txt_save, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set(txt_save, i18n[STR_SAVE_TO]);

        path_x = 20 + compo_textbox_get_wid(txt_save) + 4;
        txt = compo_textbox_create(frm, 32);
        compo_textbox_set_location(txt, path_x, LOADING2_PATH_LINE0_Y, 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set(txt, f->path_dir);
    }

    {
        char buf[40];

        sprintf(buf, "└─ %s", f->path_vol);
        txt = compo_textbox_create(frm, 32);
        compo_textbox_set_location(txt, 20, LOADING2_PATH_LINE1_Y, 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(txt, LOADING2_COLOR_VOL);
        compo_textbox_set(txt, buf);
    }

    /* 底部提示 */
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, LOADING2_WARN_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set(txt, i18n[STR_DO_NOT_REMOVE_MEDIA]);

    loading2_update_text();

    return frm;
}

static void func_loading_2_page_process(void)
{
    jms581_backup_sta_t bk;

    loading2_update_battery();

    jms581_model_backup_sta(&bk);
    if (bk.sta == JMS581_BK_FAILED || bk.sta == JMS581_BK_CANCELLED) {
        func_cb.sta = FUNC_HOME_PAGE;               /* 备份失败/取消，回首页 */
        return;
    }
    if (bk.phase >= 3) {
        /* 扫描完成，进入拷贝：整卡 → backing_up_1，最新N日 → backing_up_2 */
        f_loading2_t *f = (f_loading2_t *)func_cb.f_cb;
        func_cb.sta = f->is_whole ? FUNC_BACKING_UP_1_PAGE : FUNC_BACKING_UP_2_PAGE;
        return;
    }
    func_process();
}

static void func_loading_2_page_message(size_msg_t msg)
{
    switch (msg)
    {
    case KU_BACK:
        // 备份中禁止返回，等阶段推进或终态
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_loading_2_page_enter(void)
{
    f_loading2_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_loading2_t));
    f = (f_loading2_t *)func_cb.f_cb;

    f->days = backup_param.latest_days ? backup_param.latest_days : 1;
    f->is_whole = (func_cb.last == FUNC_CONFIRM_WHOLE_CARD);
    f->card_idx = backup_param.bk_card_idx + 1;
    f->card_total = backup_param.bk_card_cnt;
    strcpy(f->card_name, loading2_card_name(
               backup_param.bk_card_dev[backup_param.bk_card_idx]));
    strcpy(f->path_vol, "");                        /* L3 卡身份目录 phase=3 才返回 */

    strcpy(f->path_dir, f->is_whole ? "CARD_BACKUP/" : "RECENT_BACKUP/");
    if (backup_param.dir_sel[0]) {
        sprintf(f->path_dir, "%s%s", f->path_dir, backup_param.dir_sel);
    }

    func_cb.frm_main = func_loading_2_page_form_create();
}

void func_loading_2_page_exit(void)
{
    func_cb.last = FUNC_LATEST_N_DAY_BACKUP;
}

void func_loading_2_page(void)
{
    printf("%s\n", __func__);
    func_loading_2_page_enter();
    while (func_cb.sta == FUNC_LOADING_2_PAGE)
    {
        func_loading_2_page_process();
        func_loading_2_page_message(msg_dequeue());
    }
    func_loading_2_page_exit();
}
