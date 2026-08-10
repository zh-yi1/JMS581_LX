#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define CONFIRM_DIVIDER_Y           22
#define CONFIRM_DIVIDER_DOWN_Y      250

#define CONTENTS_VISIBLE            4
#define CONTENTS_ITEM_CNT           8

#define CONTENTS_ICON_Y             14
#define CONTENTS_BACK_X             18
#define CONTENTS_UP_X               (GUI_SCREEN_WIDTH - 22)

#define CONTENTS_ROW0_Y             84
#define CONTENTS_ROW_H              48
#define CONTENTS_ROW_GAP            0
#define CONTENTS_NAME_X             45
#define CONTENTS_CHECK_X            28

#define CONTENTS_FOOTER_Y           (GUI_SCREEN_HEIGHT - 28)
#define CONTENTS_ADD_X              28
#define CONTENTS_DOWN_X             (GUI_SCREEN_WIDTH - 22)

#define CONTENTS_COLOR_ADD          make_color(0x00, 0x7A, 0xFF)

/* 演示目录列表（从新到旧） */
static const char *s_dir_name[CONTENTS_ITEM_CNT] = {
    "CARD_011",
    "CARD_010",
    "CARD_009",
    "CARD_008",
    "CARD_007",
    "CARD_006",
    "CARD_005",
    "CARD_004",
};

typedef struct {
    u8 top_idx;     // 列表窗口起始下标
    u8 selection;   // 当前选中（相对全表）
    u8 press_idx;   // 按下中的行（相对全表），0xFF=无
    compo_picturebox_t *pic_divider;
    compo_picturebox_t *pic_back;
    compo_picturebox_t *pic_up;
    compo_picturebox_t *pic_down;
    compo_picturebox_t *pic_add;
    compo_picturebox_t *pic_row[CONTENTS_VISIBLE];
    compo_picturebox_t *pic_check[CONTENTS_VISIBLE];
    compo_textbox_t *txt_name[CONTENTS_VISIBLE];
    compo_textbox_t *txt_add;
} f_contents_t;

static void contents_update_display(void)
{
    f_contents_t *f = (f_contents_t *)func_cb.f_cb;
    u8 i;

    if (f == NULL) {
        return;
    }

    for (i = 0; i < CONTENTS_VISIBLE; i++) {
        u8 idx = f->top_idx + i;
        bool selected = (idx == f->selection);
        bool pressed  = (idx == f->press_idx);

        if (idx < CONTENTS_ITEM_CNT) {
            compo_picturebox_set(f->pic_row[i], (selected || pressed)
                ? UI_BUF_IMAGE_BIN_CONTENTS_CLICK_BIN
                : UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
            compo_textbox_set(f->txt_name[i], s_dir_name[idx]);
            compo_picturebox_set_visible(f->pic_row[i], true);
            compo_textbox_set_visible(f->txt_name[i], true);
            compo_picturebox_set_visible(f->pic_check[i], selected);
        } else {
            compo_textbox_set(f->txt_name[i], "");
            compo_picturebox_set_visible(f->pic_row[i], false);
            compo_textbox_set_visible(f->txt_name[i], false);
            compo_picturebox_set_visible(f->pic_check[i], false);
        }
    }

    /* 上下箭头：可滚时高亮 */
    compo_picturebox_set(f->pic_up, (f->top_idx > 0)
        ? UI_BUF_IMAGE_BIN_UP_CLICK_BIN
        : UI_BUF_IMAGE_BIN_UP_BIN);
    compo_picturebox_set(f->pic_down,
        (f->top_idx + CONTENTS_VISIBLE < CONTENTS_ITEM_CNT)
        ? UI_BUF_IMAGE_BIN_DOWN_CLICK_BIN
        : UI_BUF_IMAGE_BIN_DOWN_BIN);
}

static u8 contents_hit_row(point_t pt)
{
    f_contents_t *f = (f_contents_t *)func_cb.f_cb;
    u8 i;

    for (i = 0; i < CONTENTS_VISIBLE; i++) {
        s16 y = CONTENTS_ROW0_Y + i * (CONTENTS_ROW_H + CONTENTS_ROW_GAP);
        if (pt.y > (y - CONTENTS_ROW_H / 2) && pt.y < (y + CONTENTS_ROW_H / 2)) {
            u8 idx = f->top_idx + i;
            return (idx < CONTENTS_ITEM_CNT) ? idx : 0xFF;
        }
    }
    return 0xFF;
}

static void contents_scroll(s8 dir)
{
    f_contents_t *f = (f_contents_t *)func_cb.f_cb;
    s16 next_top = (s16)f->top_idx + dir * CONTENTS_VISIBLE;

    if (next_top < 0) {
        next_top = 0;
    } else if (next_top >= CONTENTS_ITEM_CNT) {
        return;     /* 已到最后一页，不可再向下翻 */
    }
    if ((s16)f->top_idx == next_top) {
        return;     /* 已是第一页，不可再向上翻 */
    }

    f->top_idx = (u8)next_top;
    f->selection = f->top_idx;
    f->press_idx = 0xFF;
    contents_update_display();
}

compo_form_t *func_contents_page_form_create(void)
{
    f_contents_t *f = (f_contents_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    u8 i;

    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部：返回 + 标题 + 上箭头 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, CONFIRM_DIVIDER_Y);

    f->pic_back = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACK_BIN);
    compo_picturebox_set_pos(f->pic_back, CONTENTS_BACK_X, CONTENTS_ICON_Y + 8);
    // 多语言：选择 + SSD + 目标目录
    {
        compo_textbox_t *txt_sel;
        compo_textbox_t *txt_ssd;
        compo_textbox_t *txt_dir;
        s16 title_x;
        s16 w1, w2, w3;

        txt_sel = compo_textbox_create(frm, 16);
        compo_textbox_set_autosize(txt_sel, true);
        compo_textbox_set_align_center(txt_sel, false);
        compo_textbox_set_font(txt_sel, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(txt_sel, i18n[STR_SELECT]);
        w1 = compo_textbox_get_wid(txt_sel);

        txt_ssd = compo_textbox_create(frm, 8);
        compo_textbox_set_autosize(txt_ssd, true);
        compo_textbox_set_align_center(txt_ssd, false);
        compo_textbox_set_font(txt_ssd, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(txt_ssd, " SSD ");
        w2 = compo_textbox_get_wid(txt_ssd);

        txt_dir = compo_textbox_create(frm, 16);
        compo_textbox_set_autosize(txt_dir, true);
        compo_textbox_set_align_center(txt_dir, false);
        compo_textbox_set_font(txt_dir, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(txt_dir, i18n[STR_TARGET_DIR]);
        w3 = compo_textbox_get_wid(txt_dir);

        title_x = GUI_SCREEN_CENTER_X - (w1 + w2 + w3) / 2;
        compo_textbox_set_location(txt_sel, title_x, CONTENTS_ICON_Y, 0, 0);
        compo_textbox_set_location(txt_ssd, title_x + w1, CONTENTS_ICON_Y, 0, 0);
        compo_textbox_set_location(txt_dir, title_x + w1 + w2, CONTENTS_ICON_Y, 0, 0);
    }

    f->pic_up = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_UP_BIN);
    compo_picturebox_set_pos(f->pic_up, CONTENTS_UP_X, CONTENTS_ICON_Y  + 8);

    /* 目录列表 */
    for (i = 0; i < CONTENTS_VISIBLE; i++) {
        s16 y = CONTENTS_ROW0_Y + i * (CONTENTS_ROW_H + CONTENTS_ROW_GAP);

        f->pic_row[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
        compo_picturebox_set_pos(f->pic_row[i], GUI_SCREEN_CENTER_X, y);

        f->txt_name[i] = compo_textbox_create(frm, 16);
        compo_textbox_set_location(f->txt_name[i], CONTENTS_NAME_X, y - 10, 0, 0);
        compo_textbox_set_autosize(f->txt_name[i], true);
        compo_textbox_set_align_center(f->txt_name[i], false);
        compo_textbox_set_font(f->txt_name[i], UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
        compo_textbox_set(f->txt_name[i], s_dir_name[i]);

        f->pic_check[i] = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_CHECK_MARK_BIN);
        compo_picturebox_set_pos(f->pic_check[i], CONTENTS_CHECK_X, y);
        compo_picturebox_set_visible(f->pic_check[i], false);
    }

    /* 底部：新建 + 下箭头 */
    f->pic_divider = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DIVIDER_UNCLICK_BIN);
    compo_picturebox_set_pos(f->pic_divider, GUI_SCREEN_CENTER_X, CONFIRM_DIVIDER_DOWN_Y);

    f->pic_add = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_ADD_BIN);
    compo_picturebox_set_pos(f->pic_add, CONTENTS_ADD_X, CONTENTS_FOOTER_Y + 5);

    f->txt_add = compo_textbox_create(frm, 16);
    compo_textbox_set_location(f->txt_add, CONTENTS_ADD_X + 16, CONTENTS_FOOTER_Y - 4, 0, 0);
    compo_textbox_set_autosize(f->txt_add, true);
    compo_textbox_set_align_center(f->txt_add, false);
    compo_textbox_set_font(f->txt_add, UI_BUF_FONT_BIN_FONT_SIZE_15_BIN);
    compo_textbox_set_forecolor(f->txt_add, CONTENTS_COLOR_ADD);
    compo_textbox_set(f->txt_add, "CARD_012");

    f->pic_down = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_DOWN_BIN);
    compo_picturebox_set_pos(f->pic_down, CONTENTS_DOWN_X, CONTENTS_FOOTER_Y + 5);

    f->top_idx = 0;
    f->selection = 0;
    f->press_idx = 0xFF;

    /* 再次进入：恢复上次选中的目录 */
    if (backup_param.dir_sel[0]) {
        for (i = 0; i < CONTENTS_ITEM_CNT; i++) {
            if (strcmp(s_dir_name[i], backup_param.dir_sel) == 0) {
                f->selection = i;
                break;
            }
        }
    }
    /* 让选中项出现在可视窗口内 */
    if (f->selection < f->top_idx) {
        f->top_idx = f->selection;
    } else if (f->selection >= f->top_idx + CONTENTS_VISIBLE) {
        f->top_idx = f->selection - CONTENTS_VISIBLE + 1;
    }
    contents_update_display();

    return frm;
}

static void func_contents_page_process(void)
{
    f_contents_t *f = (f_contents_t *)func_cb.f_cb;

    /* 触摸抬起或滑动移开时，恢复行按下状态 */
    if (f->press_idx != 0xFF && !ctp_is_touch()) {
        f->press_idx = 0xFF;
        contents_update_display();
    }
    func_process();
}

static void func_contents_page_message(size_msg_t msg)
{
    f_contents_t *f = (f_contents_t *)func_cb.f_cb;
    point_t pt;
    u8 idx;

    switch (msg)
    {
    case MSG_QDEC_FORWARD:
        contents_scroll(1);
        break;

    case MSG_QDEC_BACKWARD:
        contents_scroll(-1);
        break;

    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_TOUCH:
        pt = ctp_get_sxy();
        idx = contents_hit_row(pt);
        if (idx != 0xFF) {
            f->press_idx = idx;
            contents_update_display();
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_LONG_LIFT:
        if (f->press_idx != 0xFF) {
            f->press_idx = 0xFF;
            contents_update_display();
        }
        break;

    case MSG_CTP_CLICK:
        pt = ctp_get_sxy();
        if (pt.x < 48 && pt.y < 48) {
            if (func_cb.last == FUNC_LATEST_N_DAY_BACKUP) {
                func_cb.sta = FUNC_LATEST_N_DAY_BACKUP;
            } else {
                func_cb.sta = FUNC_CONFIRM_WHOLE_CARD;
            }
            break;
        }
        /* 点击上箭头：向前翻一页 */
        if (pt.y < 48 && pt.x > (GUI_SCREEN_WIDTH - 48)) {
            contents_scroll(-1);
            break;
        }
        /* 点击下箭头：向后翻一页 */
        if (pt.y > (CONTENTS_FOOTER_Y - 24) && pt.x > (GUI_SCREEN_WIDTH - 48)) {
            contents_scroll(1);
            break;
        }
        if (pt.y > (CONTENTS_FOOTER_Y - 24)) {
            /* 点击新建：后续接业务 */
            break;
        }
        idx = contents_hit_row(pt);
        if (idx != 0xFF) {
            f->selection = idx;
            f->press_idx = 0xFF;
            /* 保存选中的目录名，返回后目标路径卡片显示 */
            strcpy(backup_param.dir_sel, s_dir_name[idx]);
            contents_update_display();
            /* 选中目录后返回来源页 */
            if (func_cb.last == FUNC_LATEST_N_DAY_BACKUP) {
                func_cb.sta = FUNC_LATEST_N_DAY_BACKUP;
            } else {
                func_cb.sta = FUNC_CONFIRM_WHOLE_CARD;
            }
        }
        break;

    case KU_BACK:
        if (func_cb.last == FUNC_LATEST_N_DAY_BACKUP) {
            func_cb.sta = FUNC_LATEST_N_DAY_BACKUP;
        } else {
            func_cb.sta = FUNC_CONFIRM_WHOLE_CARD;
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_contents_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_contents_t));
    func_cb.frm_main = func_contents_page_form_create();
}

void func_contents_page_exit(void)
{
    if (func_cb.sta != FUNC_LATEST_N_DAY_BACKUP &&
        func_cb.sta != FUNC_CONFIRM_WHOLE_CARD) {
        func_cb.last = FUNC_CONFIRM_WHOLE_CARD;
    }
}

void func_contents_page(void)
{
    printf("%s\n", __func__);
    func_contents_page_enter();
    while (func_cb.sta == FUNC_CONTENTS_PAGE)
    {
        func_contents_page_process();
        func_contents_page_message(msg_dequeue());
    }
    func_contents_page_exit();
}
