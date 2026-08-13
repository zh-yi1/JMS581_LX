#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define UPGRADEING_FRAME_CNT        26

#define UPGRADEING_DIVIDER_Y        22
#define UPGRADEING_TITLE_Y          22
#define UPGRADEING_BAT_X            (GUI_SCREEN_WIDTH - 22)

#define UPGRADEING_RING_Y           130
#define UPGRADEING_RING_SIZE        80

#define UPGRADEING_STA_Y            200
#define UPGRADEING_TIP_Y            225
#define UPGRADEING_WARN_Y           248

#define UPGRADEING_COLOR_LABEL      make_color(0x99, 0x99, 0x99)

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

typedef struct {
    u8 bat_level;
    char new_ver[16];       // 如 "v1.3.0"
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_divider;
    compo_animation_t *anim_loading;
    compo_textbox_t *txt_tip;
} f_upgradeing_t;

static u8 upgradeing_bat_level_from_percent(u8 percent)
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

static void upgradeing_update_battery(void)
{
    f_upgradeing_t *f = (f_upgradeing_t *)func_cb.f_cb;
    u8 level;

    if (f == NULL || f->pic_bat == NULL) {
        return;
    }

    level = upgradeing_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == f->bat_level) {
        return;
    }
    f->bat_level = level;
    compo_picturebox_set(f->pic_bat, s_bat_level_res[level - 1]);
}

static void upgradeing_update_display(void)
{
    f_upgradeing_t *f = (f_upgradeing_t *)func_cb.f_cb;
    char buf[40];

    if (f == NULL || f->txt_tip == NULL) {
        return;
    }

    sprintf(buf, "%s %s", i18n[STR_INSTALLING], f->new_ver);
    compo_textbox_set(f->txt_tip, buf);
}

compo_form_t *func_upgradeing_page_form_create(void)
{
    f_upgradeing_t *f = (f_upgradeing_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：divider 底 + 标题 + 电量 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, UPGRADEING_DIVIDER_Y);

    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, UPGRADEING_TITLE_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_SYS_UPGRADE]);

    f->bat_level = upgradeing_bat_level_from_percent(sys_cb.vbat_percent);
    f->pic_bat = compo_picturebox_create(frm, s_bat_level_res[f->bat_level - 1]);
    compo_picturebox_set_pos(f->pic_bat, UPGRADEING_BAT_X, UPGRADEING_TITLE_Y);

    /* 中间：loading 圆环动画 */
    f->anim_loading = compo_animation_create(frm, UI_BUF_IMAGE_BIN_LOADING_BIN);
    compo_animation_set_radix(f->anim_loading, UPGRADEING_FRAME_CNT);
    compo_animation_set_interval(f->anim_loading, 5);
    compo_animation_set_pos(f->anim_loading, GUI_SCREEN_CENTER_X, UPGRADEING_RING_Y);
    compo_animation_set_size(f->anim_loading, UPGRADEING_RING_SIZE, UPGRADEING_RING_SIZE);

    /* 正在升级 */
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, UPGRADEING_STA_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
    compo_textbox_set(txt, i18n[STR_UPGRADEING]);

    /* 正在安装 vx.x.x */
    f->txt_tip = compo_textbox_create(frm, 32);
    compo_textbox_set_location(f->txt_tip, GUI_SCREEN_CENTER_X, UPGRADEING_TIP_Y, 0, 0);
    compo_textbox_set_autosize(f->txt_tip, true);
    compo_textbox_set_align_center(f->txt_tip, true);
    compo_textbox_set_font(f->txt_tip, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(f->txt_tip, UPGRADEING_COLOR_LABEL);

    /* 请保持设备供电 */
    txt = compo_textbox_create(frm, 32);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, UPGRADEING_WARN_Y, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, UPGRADEING_COLOR_LABEL);
    compo_textbox_set(txt, i18n[STR_KEEP_POWER_ON]);

    upgradeing_update_display();

    return frm;
}

static void func_upgradeing_page_process(void)
{
    upgradeing_update_battery();
    func_process();
}

static void func_upgradeing_page_message(size_msg_t msg)
{
    switch (msg)
    {
    case KU_BACK:
        /* 升级中禁止返回 */
        break;

    default:
        break;
    }
}

void func_upgradeing_page_enter(void)
{
    f_upgradeing_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_upgradeing_t));
    f = (f_upgradeing_t *)func_cb.f_cb;

    /* 演示数据，后续由升级流程填充 */
    strcpy(f->new_ver, "v1.3.0");

    func_cb.frm_main = func_upgradeing_page_form_create();
}

void func_upgradeing_page_exit(void)
{
    func_cb.last = FUNC_UPGRADE_PAGE;
}

void func_upgradeing_page(void)
{
    printf("%s\n", __func__);
    func_upgradeing_page_enter();
    while (func_cb.sta == FUNC_UPGRADEING_PAGE)
    {
        func_upgradeing_page_process();
        func_upgradeing_page_message(msg_dequeue());
    }
    func_upgradeing_page_exit();
}
