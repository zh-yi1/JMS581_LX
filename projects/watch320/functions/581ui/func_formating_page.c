#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define FORMATING_FRAME_CNT         26

#define FORMATING_DIVIDER_Y         22
#define FORMATING_TITLE_Y           22
#define FORMATING_BAT_X             (GUI_SCREEN_WIDTH - 22)

#define FORMATING_RING_Y            130
#define FORMATING_RING_SIZE         80

#define FORMATING_STA_Y             200
#define FORMATING_TIP_Y             225
#define FORMATING_WARN_Y            248

#define FORMATING_COLOR_LABEL       make_color(0x99, 0x99, 0x99)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

typedef struct {
    u8 bat_level;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_divider;
    compo_animation_t *anim_loading;
} f_formating_t;

static u8 formating_bat_level_from_percent(u8 percent)
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

static void formating_update_battery(void)
{
    f_formating_t *f = (f_formating_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = formating_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

compo_form_t *func_formating_page_form_create(void)
{
    f_formating_t *f = (f_formating_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider 底 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, FORMATING_DIVIDER_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, FORMATING_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_FORMAT_SSD]);

    f->bat_level = formating_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, FORMATING_BAT_X, FORMATING_TITLE_Y);

    /* 中间：loading 圆环动画 */
    f->anim_loading = compo_animation_create(frm, UI_BUF_IMAGE_BIN_LOADING_BIN);
    compo_animation_set_radix(f->anim_loading, FORMATING_FRAME_CNT);
    compo_animation_set_interval(f->anim_loading, 5);
    compo_animation_set_pos(f->anim_loading, GUI_SCREEN_CENTER_X, FORMATING_RING_Y);
    compo_animation_set_size(f->anim_loading, FORMATING_RING_SIZE, FORMATING_RING_SIZE);

    /* 正在格式化 SSD */
    txt = compo_textbox_create(frm, 24);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, FORMATING_STA_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_FORMATING_SSD]);

    /* 正在重建存储空间 */
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, FORMATING_TIP_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, FORMATING_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_REBUILD_STORAGE]);

    /* 请勿关机或断开 SSD */
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, FORMATING_WARN_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, FORMATING_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_DO_NOT_POWER_OFF_SSD]);

    return frm;
}

static void func_formating_page_process(void)
{
    formating_update_battery();
    func_process();
}

static void func_formating_page_message(size_msg_t msg)
{
    switch (msg)
    {
    case KU_BACK:
        /* 格式化中禁止返回 */
        break;

    default:
        break;
    }
}

void func_formating_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_formating_t));
    func_cb.frm_main = func_formating_page_form_create();
}

void func_formating_page_exit(void)
{
    func_cb.last = FUNC_FORMAT_PAGE;
}

void func_formating_page(void)
{
    printf("%s\n", __func__);
    func_formating_page_enter();
    while (func_cb.sta == FUNC_FORMATING_PAGE)
    {
        func_formating_page_process();
        func_formating_page_message(msg_dequeue());
    }
    func_formating_page_exit();
}
