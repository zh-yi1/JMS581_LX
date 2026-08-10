#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define HOME_CARD_CNT               3
#define HOME_BTN_CNT                2

#define HOME_ROW_W                  216
#define HOME_ROW_H                  48
#define HOME_ROW0_Y                 108
#define HOME_ROW_GAP                4

#define HOME_CHECK_X_OFS            (-90)
#define HOME_NAME_X_OFS             (-45)
#define HOME_STA_X_OFS              (70)

#define HOME_BTN_W                  104
#define HOME_BTN_H                  48
#define HOME_BTN_Y                  (GUI_SCREEN_HEIGHT - 36)
#define HOME_BTN_X_OFS              56

#define HOME_COLOR_STA              make_color(0x99, 0x99, 0x99)
#define HOME_COLOR_DISABLE          make_color(0x66, 0x66, 0x66)

#define HOME_ICON_Y                 31
#define HOME_SET_X                  (GUI_SCREEN_WIDTH - 20)
#define HOME_BAT_X                  (GUI_SCREEN_WIDTH - 52)

// 卡槽插卡状态（演示：SD/CFA 已就绪，CFB 未插卡）
static const u8 s_card_ready[HOME_CARD_CNT] = {1, 1, 0};
// 已插卡勾选初始状态
static const u8 s_card_checked_init[HOME_CARD_CNT] = {1, 1, 0};
static const char *s_card_name[HOME_CARD_CNT] = {
    "SD", "CFA", "CFB"
};

static const u32 s_bat_level_res[] = {
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_1_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_2_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_3_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_4_BIN,
    UI_BUF_IMAGE_BIN_BATTERY_LEVEL_5_BIN,
};

// 首页私有状态
typedef struct {
    u8 selection;   // 0~2 卡槽选中
    u8 bat_level;   // 1~5
    u8 press_idx;   // 0~2 按下中的卡槽，0xFF=无
    u8 btn_press;   // 0~1 按下中的底部按钮，0xFF=无
    u8 checked[HOME_CARD_CNT];
    compo_picturebox_t *pic_set;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_row[HOME_CARD_CNT];
    compo_picturebox_t *pic_check[HOME_CARD_CNT];
    compo_textbox_t *txt_name[HOME_CARD_CNT];
    compo_textbox_t *txt_sta[HOME_CARD_CNT];
    compo_picturebox_t *pic_btn[HOME_BTN_CNT];
    compo_textbox_t *txt_btn[HOME_BTN_CNT];
} f_home_t;

static u8 home_hit_card(point_t pt)
{
    u8 i;

    for (i = 0; i < HOME_CARD_CNT; i++) {
        s16 y = HOME_ROW0_Y + i * (HOME_ROW_H + HOME_ROW_GAP);
        if (pt.y > (y - HOME_ROW_H / 2) && pt.y < (y + HOME_ROW_H / 2)) {
            return i;
        }
    }
    return 0xFF;
}

static u8 home_hit_btn(point_t pt)
{
    if (pt.y <= (HOME_BTN_Y - 30)) {
        return 0xFF;
    }
    return (pt.x < GUI_SCREEN_CENTER_X) ? 0 : 1;
}

static void home_set_row_press(f_home_t *h, u8 idx)
{
    if (h->press_idx < HOME_CARD_CNT) {
        compo_picturebox_set(h->pic_row[h->press_idx],
                             UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    }
    h->press_idx = idx;
    if (idx < HOME_CARD_CNT) {
        compo_picturebox_set(h->pic_row[idx], UI_BUF_IMAGE_BIN_CLICK_BJ_BIN);
    }
}

static void home_set_btn_press(f_home_t *h, u8 idx)
{
    if (h->btn_press < HOME_BTN_CNT) {
        compo_picturebox_set(h->pic_btn[h->btn_press],
                             UI_BUF_IMAGE_BIN_BOTTON_UNCLICK_BIN);
    }
    h->btn_press = idx;
    if (idx < HOME_BTN_CNT) {
        compo_picturebox_set(h->pic_btn[idx], UI_BUF_IMAGE_BIN_BOTTON_CLICK_BIN);
    }
}

static void home_clear_press(f_home_t *h)
{
    home_set_row_press(h, 0xFF);
    home_set_btn_press(h, 0xFF);
}

static u8 home_bat_level_from_percent(u8 percent)
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

static void home_update_battery(void)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;
    u8 level;

    if (h == NULL || h->pic_bat == NULL) {
        return;
    }

    level = home_bat_level_from_percent(sys_cb.vbat_percent);
    if (level == h->bat_level) {
        return;
    }
    h->bat_level = level;
    compo_picturebox_set(h->pic_bat, s_bat_level_res[level - 1]);
}

static void home_update_display(void)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;
    u8 i;

    for (i = 0; i < HOME_CARD_CNT; i++) {
        bool ready = s_card_ready[i];
        bool checked = ready && h->checked[i];

        // 行背景：按下高亮，松开恢复
        compo_picturebox_set(h->pic_row[i], (h->press_idx == i)
            ? UI_BUF_IMAGE_BIN_CLICK_BJ_BIN
            : UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);

        // 勾选框：仅已插卡可切换；未插卡固定未勾选
        compo_picturebox_set(h->pic_check[i], checked
            ? UI_BUF_IMAGE_BIN_CLICK_BIN
            : UI_BUF_IMAGE_BIN_UNCLICK_BIN);

        // 已就绪 / 未插卡：跟插卡状态，不随点击变化
        if (ready) {
            compo_textbox_set_forecolor(h->txt_name[i], COLOR_WHITE);
            compo_textbox_set_forecolor(h->txt_sta[i], HOME_COLOR_STA);
            compo_textbox_set(h->txt_sta[i], i18n[STR_CARD_READY]);
        } else {
            compo_textbox_set_forecolor(h->txt_name[i], HOME_COLOR_DISABLE);
            compo_textbox_set_forecolor(h->txt_sta[i], HOME_COLOR_DISABLE);
            compo_textbox_set(h->txt_sta[i], i18n[STR_CARD_ABSENT]);
        }
    }

    // 底部按钮：按下高亮，松开恢复
    for (i = 0; i < HOME_BTN_CNT; i++) {
        compo_picturebox_set(h->pic_btn[i], (h->btn_press == i)
            ? UI_BUF_IMAGE_BIN_BOTTON_CLICK_BIN
            : UI_BUF_IMAGE_BIN_BOTTON_UNCLICK_BIN);
    }
}

compo_form_t *func_home_page_form_create(void)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;
    u8 i;

    /* 黑色背景，避免 GPU 误读 UI 头当图解码 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：SSD（固定）+ 剩余（多语言） */
    {
        compo_textbox_t *txt_ssd;
        s16 remain_x;

        txt_ssd = compo_textbox_create(frm, 8);
        compo_textbox_set_location(txt_ssd, 15, 16, 0, 0);
        compo_textbox_set_autosize(txt_ssd, true);
        compo_textbox_set_align_center(txt_ssd, false);
        compo_textbox_set_font(txt_ssd, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(txt_ssd, HOME_COLOR_STA);
        compo_textbox_set(txt_ssd, "SSD");

        remain_x = 15 + compo_textbox_get_wid(txt_ssd);
        txt = compo_textbox_create(frm, 16);
        compo_textbox_set_location(txt, remain_x, 16, 0, 0);
        compo_textbox_set_autosize(txt, true);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set_forecolor(txt, HOME_COLOR_STA);
        compo_textbox_set(txt, i18n[STR_REMAIN]);
    }

    txt = compo_textbox_create(frm, 18);
    compo_textbox_set_location(txt, 16, 29, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_22_BIN);
    compo_textbox_set(txt, "1 .42 TB");

    /* 顶部右侧：设置 + 电量 */
    h->pic_set = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_SETTING_BIN);
    compo_picturebox_set_pos(h->pic_set, HOME_SET_X, HOME_ICON_Y);
  
    h->bat_level = home_bat_level_from_percent(sys_cb.vbat_percent);
    h->pic_bat = compo_picturebox_create(frm, s_bat_level_res[h->bat_level - 1]);
    compo_picturebox_set_pos(h->pic_bat, HOME_BAT_X, HOME_ICON_Y);

    /* 卡槽列表 */
    for (i = 0; i < HOME_CARD_CNT; i++) {
        s16 y = HOME_ROW0_Y + i * (HOME_ROW_H + HOME_ROW_GAP);

        h->pic_row[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
        compo_picturebox_set_pos(h->pic_row[i], GUI_SCREEN_CENTER_X, y);

        h->pic_check[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_CLICK_BIN);
        compo_picturebox_set_pos(h->pic_check[i],
                                 GUI_SCREEN_CENTER_X + HOME_CHECK_X_OFS, y);

        h->txt_name[i] = compo_textbox_create(frm, 16);
        compo_textbox_set_location(h->txt_name[i],
                                   GUI_SCREEN_CENTER_X + HOME_NAME_X_OFS, y, 0, 0);
        compo_textbox_set_autosize(h->txt_name[i], true);
        compo_textbox_set_align_center(h->txt_name[i], true);
        compo_textbox_set_font(h->txt_name[i], UI_BUF_FONT_BIN_FONT_SIZE_17_BIN);
        compo_textbox_set(h->txt_name[i], s_card_name[i]);

        h->txt_sta[i] = compo_textbox_create(frm, 16);
        compo_textbox_set_location(h->txt_sta[i],
                                   GUI_SCREEN_CENTER_X + HOME_STA_X_OFS, y, 0, 0);
        compo_textbox_set_autosize(h->txt_sta[i], true);
        compo_textbox_set_align_center(h->txt_sta[i], true);
        compo_textbox_set_forecolor(h->txt_sta[i], HOME_COLOR_STA);
        compo_textbox_set_font(h->txt_sta[i], UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
        compo_textbox_set(h->txt_sta[i], i18n[STR_CARD_READY]);
    }

    /* 底部按钮：整卡 / 最新 7 日 */
    for (i = 0; i < HOME_BTN_CNT; i++) {
        s16 x = GUI_SCREEN_CENTER_X + (i == 0 ? -HOME_BTN_X_OFS : HOME_BTN_X_OFS);

        h->pic_btn[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BOTTON_UNCLICK_BIN);
        compo_picturebox_set_pos(h->pic_btn[i], x, HOME_BTN_Y);

        h->txt_btn[i] = compo_textbox_create(frm, 16);
        compo_textbox_set_location(h->txt_btn[i], x, HOME_BTN_Y, 0, 0);
        compo_textbox_set_autosize(h->txt_btn[i], true);
        compo_textbox_set_align_center(h->txt_btn[i], true);
        compo_textbox_set_font(h->txt_btn[i], UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        if (i == 0) {
            compo_textbox_set(h->txt_btn[i], i18n[STR_FULL_CARD]);
        } else if (backup_param.latest_days != 0) {
            /* 显示上次在最新N日页选择的天数，如「最新6日」 */
            char buf[24];
            sprintf(buf, "%s%u%s", i18n[STR_LATEST], backup_param.latest_days, i18n[STR_DAY]);
            compo_textbox_set(h->txt_btn[i], buf);
        } else {
            compo_textbox_set(h->txt_btn[i], i18n[STR_LATEST_7D]);
        }
    }

    h->selection = 0;
    h->press_idx = 0xFF;
    h->btn_press = 0xFF;
    if (backup_param.card_checked_set) {
        /* 跨页面返回：恢复上次的勾选状态 */
        for (i = 0; i < HOME_CARD_CNT; i++) {
            h->checked[i] = backup_param.card_checked[i];
        }
    } else {
        /* 首次进入：使用默认勾选，并保存到共享结构体 */
        for (i = 0; i < HOME_CARD_CNT; i++) {
            h->checked[i] = s_card_checked_init[i];
            backup_param.card_checked[i] = s_card_checked_init[i];
        }
        backup_param.card_checked_set = 1;
    }
    home_update_display();

    return frm;
}

static void func_home_page_process(void)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;

    /* 滑动移开或抬起时，恢复按下态 */
    if ((h->press_idx != 0xFF || h->btn_press != 0xFF) && !ctp_is_touch()) {
        home_clear_press(h);
    }
    home_update_battery();
    func_process();
}

static void home_select_next(f_home_t *h, s8 dir)
{
    u8 next = h->selection;
    u8 try_cnt = HOME_CARD_CNT;

    while (try_cnt--) {
        next = (next + dir + HOME_CARD_CNT) % HOME_CARD_CNT;
        if (s_card_ready[next]) {
            h->selection = next;
            return;
        }
    }
}

static void func_home_page_message(size_msg_t msg)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;
    point_t pt;
    u8 idx;

    switch (msg)
    {
    case MSG_QDEC_FORWARD:
        home_select_next(h, 1);
        break;

    case MSG_QDEC_BACKWARD:
        home_select_next(h, -1);
        break;

    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        idx = home_hit_btn(pt);
        if (idx != 0xFF) {
            home_set_row_press(h, 0xFF);
            home_set_btn_press(h, idx);
            break;
        }
        home_set_btn_press(h, 0xFF);
        idx = home_hit_card(pt);
        if (idx != 0xFF && s_card_ready[idx]) {
            home_set_row_press(h, idx);
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG_LIFT:
        home_clear_press(h);
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        home_clear_press(h);
        /* 底部按钮：点击进入对应功能 */
        idx = home_hit_btn(pt);
        if (idx != 0xFF) {
            /* 拼接勾选的卡槽名称传给目标页面 */
            char *p = backup_param.card_sel;
            u8 i;

            p[0] = 0;
            /* 一个都没勾选时，默认选中 SD 卡槽 */
            for (i = 0; i < HOME_CARD_CNT; i++) {
                if (h->checked[i]) {
                    break;
                }
            }
            if (i == HOME_CARD_CNT) {
                h->checked[0] = 1;
                h->selection = 0;
                backup_param.card_checked[0] = 1;
                home_update_display();
            }

            for (i = 0; i < HOME_CARD_CNT; i++) {
                if (h->checked[i]) {
                    p += sprintf(p, " %s", s_card_name[i]);
                }
            }
            if (p == backup_param.card_sel) {
                p += sprintf(p, " %s", s_card_name[h->selection]);
            }

            func_cb.sta = (idx == 0) ? FUNC_CONFIRM_WHOLE_CARD
                                     : FUNC_LATEST_N_DAY_BACKUP;
            break;
        }
        /* 卡槽：仅已插卡可点，每点一下切换勾选；未插卡不可点 */
        idx = home_hit_card(pt);
        if (idx != 0xFF && s_card_ready[idx]) {
            h->checked[idx] = !h->checked[idx];
            backup_param.card_checked[idx] = h->checked[idx];
            h->selection = idx;
            home_update_display();
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_home_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_home_t));
    func_cb.frm_main = func_home_page_form_create();
}

void func_home_page_exit(void)
{
    func_cb.last = FUNC_CLOCK;
}

void func_home_page(void)
{
    printf("%s\n", __func__);
    func_home_page_enter();
    while (func_cb.sta == FUNC_HOME_PAGE)
    {
        func_home_page_process();
        func_home_page_message(msg_dequeue());
    }
    func_home_page_exit();
}
