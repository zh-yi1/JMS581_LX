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

// 已插卡勾选初始状态
static const u8 s_card_checked_init[HOME_CARD_CNT] = {1, 1, 0};
static const char *s_card_name[HOME_CARD_CNT] = {
    "SD", "CFA", "CFB"
};
// 与 s_card_name 一一对应的协议在位 bit（0x8000 的 dev_list）
// 注意是 JMS581_DEVBIT_x，不是 0x8006 的 JMS581_SLOT_x，两套编号顺序不同
static const u8 s_card_devbit[HOME_CARD_CNT] = {
    JMS581_DEVBIT_SD, JMS581_DEVBIT_CFA, JMS581_DEVBIT_CFB
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
    // jms581_model 的更新计数：建表时用真值画完并记下，之后每圈比对，变了才重画
    u32 ver_sta;    // 卡槽在位（0x8000）
    u32 ver_cap;    // SSD 容量（0x8006）
    // 点了整卡备份后在本页等目录首帧，拿到才跳，让确认页首帧就是真目录名
    u8 goto_sta;    // 数据到齐后要切的界面，0=没在等
    compo_textbox_t *txt_cap;
    compo_picturebox_t *pic_set;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_row[HOME_CARD_CNT];
    compo_picturebox_t *pic_check[HOME_CARD_CNT];
    compo_textbox_t *txt_name[HOME_CARD_CNT];
    compo_textbox_t *txt_sta[HOME_CARD_CNT];
    compo_picturebox_t *pic_btn[HOME_BTN_CNT];
    compo_textbox_t *txt_btn[HOME_BTN_CNT];
} f_home_t;

// 卡槽是否在位：查 jms581_model 缓存的 0x8000 dev_list
// 开机门闸保证进本页时数据已到齐，取不到值只可能是 581 异常，一律按未插卡处理
static u8 home_card_ready(u8 idx)
{
    jms581_dev_status_t sta;

    if (idx >= HOME_CARD_CNT || !jms581_model_dev_status(&sta)) {
        return 0;
    }
    return (sta.dev_list & s_card_devbit[idx]) ? 1 : 0;
}

// 第一个在位的卡槽，全都没插返回 0xFF
static u8 home_first_ready(void)
{
    u8 i;

    for (i = 0; i < HOME_CARD_CNT; i++) {
        if (home_card_ready(i)) {
            return i;
        }
    }
    return 0xFF;
}

// KB → "1.42 TB" / "512.00 GB" / "800 MB"
// 先把 u64 的 KB 降成 u32 的 MB，之后全在 32 位里做；1TB/1GB 都是 2 的幂，
// 用移位和掩码取整数部分与两位小数，不引浮点、也不引 64 位除法
static void home_fmt_size(u64 kb, char *buf)
{
    // u32 在本工具链是 long unsigned int，sprintf 的 %u 对不上，统一转 unsigned int
    u32 mb = (u32)(kb >> 10);                   // 4PB 以内不会溢出 u32

    if (mb >= (1024u * 1024u)) {                // >=1TB
        sprintf(buf, "%u.%02u TB", (unsigned int)(mb >> 20),
                (unsigned int)(((mb & 0xFFFFFu) * 100) >> 20));
    } else if (mb >= 1024u) {                   // >=1GB
        sprintf(buf, "%u.%02u GB", (unsigned int)(mb >> 10),
                (unsigned int)(((mb & 0x3FFu) * 100) >> 10));
    } else {
        sprintf(buf, "%u MB", (unsigned int)mb);
    }
}

// SSD 剩余容量文本：取 0x8006 的 M.2 槽（协议槽序固定 M.2/CFA/CFB/SD）
static void home_fmt_ssd_remain(char *buf)
{
    jms581_capacity_t cap;
    const jms581_slot_cap_t *m2;

    if (!jms581_model_capacity(&cap)) {
        strcpy(buf, "0 MB");                    // 581 异常，门闸本不该放行到这里
        return;
    }
    m2 = &cap.slot[JMS581_SLOT_M2];
    if (!m2->present) {
        strcpy(buf, "0 MB");
        return;
    }
    if (!m2->used_valid || m2->used_size > m2->total_size) {
        home_fmt_size(m2->total_size, buf);     // 已用量无效：退化显示总容量
        return;
    }
    home_fmt_size(m2->total_size - m2->used_size, buf);
}

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

    level = home_bat_level_from_percent(gauge_percent_get());
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
        bool ready = home_card_ready(i);
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

    {
        char cap_buf[16];

        h->txt_cap = compo_textbox_create(frm, 18);
        compo_textbox_set_location(h->txt_cap, 16, 29, 0, 0);
        compo_textbox_set_autosize(h->txt_cap, true);
        compo_textbox_set_align_center(h->txt_cap, false);
        compo_textbox_set_font(h->txt_cap, UI_BUF_FONT_BIN_FONT_SIZE_22_BIN);
        home_fmt_ssd_remain(cap_buf);
        compo_textbox_set(h->txt_cap, cap_buf);
    }

    /* 顶部右侧：设置 + 电量 */
    h->pic_set = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_SETTING_BIN);
    compo_picturebox_set_pos(h->pic_set, HOME_SET_X, HOME_ICON_Y);
  
    h->bat_level = home_bat_level_from_percent(gauge_percent_get());
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

    h->selection = home_first_ready();          /* 选中落在在位的卡槽上 */
    if (h->selection == 0xFF) {
        h->selection = 0;                       /* 一张卡都没插，随便落一个 */
    }
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
    /* 首帧已用真值画完，记下当前计数；此后只有 581 上报新数据才重画 */
    h->ver_sta = jms581_model_ver(JMS581_VER_STATUS);
    h->ver_cap = jms581_model_ver(JMS581_VER_CAP);

    return frm;
}

/* 581 数据刷新：比对 jms581_model 的更新计数，变了才重画。
   本页不注册任何协议回调，所以退出时无需注销 */
static void home_update_581(f_home_t *h)
{
    u32 ver;

    ver = jms581_model_ver(JMS581_VER_CAP);
    if (ver != h->ver_cap) {
        char cap_buf[16];

        h->ver_cap = ver;
        home_fmt_ssd_remain(cap_buf);
        compo_textbox_set(h->txt_cap, cap_buf);
    }

    ver = jms581_model_ver(JMS581_VER_STATUS);
    if (ver != h->ver_sta) {
        h->ver_sta = ver;
        home_update_display();          /* 插拔卡：在位/勾选态整体重画 */
    }
}

static void func_home_page_process(void)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;

    /* 滑动移开或抬起时，恢复按下态 */
    if ((h->press_idx != 0xFF || h->btn_press != 0xFF) && !ctp_is_touch()) {
        home_clear_press(h);
    }
    home_update_battery();
    home_update_581(h);

    /* 等目录首帧：拿到才跳，跳过去首帧就是真目录名，不出现占位符 */
    if (h->goto_sta && jms581_model_dir_ready()) {
        func_cb.sta = h->goto_sta;
        h->goto_sta = 0;
    }
    func_process();
}

static void home_select_next(f_home_t *h, s8 dir)
{
    u8 next = h->selection;
    u8 try_cnt = HOME_CARD_CNT;

    while (try_cnt--) {
        next = (next + dir + HOME_CARD_CNT) % HOME_CARD_CNT;
        if (home_card_ready(next)) {
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
        if (idx != 0xFF && home_card_ready(idx)) {
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
        /* 右上角设置图标 */
        if (pt.y < 48 && pt.x > (HOME_SET_X - 24)) {
            func_cb.sta = FUNC_SETUP_PAGE;
            break;
        }
        /* 底部按钮：点击进入对应功能 */
        idx = home_hit_btn(pt);
        if (idx != 0xFF) {
            /* 拼接勾选的卡槽名称传给目标页面：只认「在位且勾选」的，
               勾选状态是跨页面保留的，可能停留在一个后来被拔掉的卡槽上 */
            char *p = backup_param.card_sel;
            u8 i, first = home_first_ready();

            if (first == 0xFF) {
                TRACE("home: no card present, backup ignored\n");
                break;                          /* 一张卡都没插，不进备份流程 */
            }

            p[0] = 0;
            for (i = 0; i < HOME_CARD_CNT; i++) {
                if (home_card_ready(i) && h->checked[i]) {
                    p += sprintf(p, " %s", s_card_name[i]);
                }
            }
            if (p == backup_param.card_sel) {   /* 在位的一个都没勾选：默认勾第一个在位的 */
                h->checked[first] = 1;
                h->selection = first;
                backup_param.card_checked[first] = 1;
                home_update_display();
                p += sprintf(p, " %s", s_card_name[first]);
            }

            if (idx == 0) {
                /* 整卡：先取目录列表，等首帧回来再跳，确认页首帧就是真目录名。
                   最新 N 日那条路暂未接目录，保持原样直接跳 */
                jms581_model_dir_open(JMS581_ROOT_CARD_BACKUP);
                h->goto_sta = FUNC_CONFIRM_WHOLE_CARD;
            } else {
                func_cb.sta = FUNC_LATEST_N_DAY_BACKUP;
            }
            break;
        }
        /* 卡槽：仅已插卡可点，每点一下切换勾选；未插卡不可点 */
        idx = home_hit_card(pt);
        if (idx != 0xFF && home_card_ready(idx)) {
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
