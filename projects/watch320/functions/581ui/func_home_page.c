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
#define HOME_NAME_X_OFS             (-55)
#define HOME_STA_X_OFS              (70)

#define HOME_BTN_W                  104
#define HOME_BTN_H                  48
#define HOME_BTN_Y                  (GUI_SCREEN_HEIGHT - 36)
#define HOME_BTN_X_OFS              56

#define HOME_COLOR_STA              make_color(0x99, 0x99, 0x99)
#define HOME_COLOR_DISABLE          make_color(0x66, 0x66, 0x66)

// 卡槽就绪状态（演示：SD/CFA 已就绪，CFB 未插卡）
static const u8 s_card_ready[HOME_CARD_CNT] = {1, 1, 0};
static const u16 s_card_name_id[HOME_CARD_CNT] = {
    STR_CARD_SD, STR_CARD_CFA, STR_CARD_CFB
};

// 首页私有状态
typedef struct {
    u8 selection;   // 0~2 卡槽选中
    u8 btn_sel;     // 0=整卡, 1=最新7日
    compo_picturebox_t *pic_set;
    compo_picturebox_t *pic_row[HOME_CARD_CNT];
    compo_picturebox_t *pic_check[HOME_CARD_CNT];
    compo_textbox_t *txt_name[HOME_CARD_CNT];
    compo_textbox_t *txt_sta[HOME_CARD_CNT];
    compo_picturebox_t *pic_btn[HOME_BTN_CNT];
    compo_textbox_t *txt_btn[HOME_BTN_CNT];
} f_home_t;

static void home_update_display(void)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;
    u8 i;

    for (i = 0; i < HOME_CARD_CNT; i++) {
        bool selected = (h->selection == i);
        bool ready = s_card_ready[i];

        // 行背景：选中 / 未选中
        compo_picturebox_set(h->pic_row[i], selected
            ? UI_BUF_IMAGE_BIN_CLICK_BJ_BIN
            : UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);

        // 勾选框：已就绪 / 未插卡
        compo_picturebox_set(h->pic_check[i], ready
            ? UI_BUF_IMAGE_BIN_CLICK_BIN
            : UI_BUF_IMAGE_BIN_UNCLICK_BIN);

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

    // 底部按钮：选中态 vs 未选中态
    for (i = 0; i < HOME_BTN_CNT; i++) {
        bool selected = (h->btn_sel == i);
        compo_picturebox_set(h->pic_btn[i], selected
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

    /* 顶部：SSD 剩余 */
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, 16, 22, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_11_BIN);
    compo_textbox_set_forecolor(txt, HOME_COLOR_STA);
    compo_textbox_set(txt, i18n[STR_SSD_REMAIN]);

    txt = compo_textbox_create(frm, 18);
    compo_textbox_set_location(txt, 16, 52, 0, 0);
    compo_textbox_set_autosize(txt, true);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set_font(txt, UI_BUF_FONT_BIN_FONT_SIZE_22_BIN);
    compo_textbox_set(txt, "1.42 TB");

    /* 顶部右侧：设置图标 */
    h->pic_set = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_SET_1_BIN);
    compo_picturebox_set_pos(h->pic_set, GUI_SCREEN_WIDTH - 28, 28);
    compo_picturebox_set_size(h->pic_set, 24, 24);

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
        compo_textbox_set(h->txt_name[i], i18n[s_card_name_id[i]]);

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

        h->pic_btn[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BOTTON_CLICK_BIN);
        compo_picturebox_set_pos(h->pic_btn[i], x, HOME_BTN_Y);

        h->txt_btn[i] = compo_textbox_create(frm, 16);
        compo_textbox_set_location(h->txt_btn[i], x, HOME_BTN_Y, 0, 0);
        compo_textbox_set_autosize(h->txt_btn[i], true);
        compo_textbox_set_align_center(h->txt_btn[i], true);
        compo_textbox_set_font(h->txt_btn[i], UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(h->txt_btn[i],
                          i18n[i == 0 ? STR_FULL_CARD : STR_LATEST_7D]);
    }

    h->selection = 0;
    h->btn_sel = 0;
    home_update_display();

    return frm;
}

static void func_home_page_process(void)
{
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
            home_update_display();
            return;
        }
    }
}

static void func_home_page_message(size_msg_t msg)
{
    f_home_t *h = (f_home_t *)func_cb.f_cb;

    switch (msg)
    {
    case MSG_QDEC_FORWARD:
    case MSG_CTP_SHORT_DOWN:
        home_select_next(h, 1);
        break;

    case MSG_QDEC_BACKWARD:
    case MSG_CTP_SHORT_UP:
        home_select_next(h, -1);
        break;

    case MSG_CTP_SHORT_LEFT:
        if (h->btn_sel != 0) {
            h->btn_sel = 0;
            home_update_display();
        }
        break;

    case MSG_CTP_SHORT_RIGHT:
        if (h->btn_sel != 1) {
            h->btn_sel = 1;
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
