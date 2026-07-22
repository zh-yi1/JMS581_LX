#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)      printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define DIA_TYPE_CNT        (sizeof(dia_type_color) / sizeof(dia_type_color[0]))
#define TIMER_TYPE_CNT      (sizeof(dia_timer_color) / sizeof(dia_timer_color[0]))

enum {
    COMPO_ID_PROSPECTS_HOUR_H = 1,
    COMPO_ID_PROSPECTS_HOUR_L,
    COMPO_ID_PROSPECTS_MIN_H,
    COMPO_ID_PROSPECTS_MIN_L,

    COMPO_ID_BACKGROUND_HOUR_H,
    COMPO_ID_BACKGROUND_HOUR_L,
    COMPO_ID_BACKGROUND_MIN_H,
    COMPO_ID_BACKGROUND_MIN_L,

    COMPO_ID_BACKGROUND_COLOR,          //背景颜色
    COMPO_ID_PROSPECTS_COLOR,           //前景颜色
};

u16 func_clock_preview_get_type(void);

///前景与背景风格类型
typedef struct dia_type_color_t_ {
    u16 background;
    u16 prospects;
    u16 background_num;
    u16 prospects_num;
    u8 alph;
}dia_type_color_t;

static const dia_type_color_t dia_type_color[] = {
    {0x8410,    0x0,        COLOR_BLACK, COLOR_WHITE,   0xff},
    {0xff37,    0xedc1,     COLOR_WHITE, COLOR_BLACK,   0xff},
    {0x5414,    0xadb9,     COLOR_BLACK, COLOR_WHITE,   0xff},
    {0x89ef,    0xb517,     COLOR_WHITE, COLOR_BLACK,   0xff},
    {0xc490,    0x0,        COLOR_BLACK, COLOR_WHITE,   0xff},
};

///数字图片坐标 与 资源
typedef struct dia_info_t_ {
    u32 res;        //图片资源
    s16 x, y;        //坐标
} dia_info_t;

static const dia_info_t info[] = {
    [COMPO_ID_PROSPECTS_HOUR_H]     = {UI_BUF_DIALPLATE_HOURGLASS_NUM1_BIN, 80, 96},
    [COMPO_ID_PROSPECTS_HOUR_L]     = {UI_BUF_DIALPLATE_HOURGLASS_NUM1_BIN, 240, 96},
    [COMPO_ID_PROSPECTS_MIN_H]      = {UI_BUF_DIALPLATE_HOURGLASS_NUM1_BIN, 80, 288},
    [COMPO_ID_PROSPECTS_MIN_L]      = {UI_BUF_DIALPLATE_HOURGLASS_NUM1_BIN, 240, 288},

    [COMPO_ID_BACKGROUND_HOUR_H]    = {UI_BUF_DIALPLATE_HOURGLASS_NUM1_BIN, 80, 96},
    [COMPO_ID_BACKGROUND_HOUR_L]    = {UI_BUF_DIALPLATE_HOURGLASS_NUM1_BIN, 240, 96},
    [COMPO_ID_BACKGROUND_MIN_H]     = {UI_BUF_DIALPLATE_HOURGLASS_NUM1_BIN, 80, 288},
    [COMPO_ID_BACKGROUND_MIN_L]     = {UI_BUF_DIALPLATE_HOURGLASS_NUM1_BIN, 240, 288},

};

static void func_clock_hourglass_update(widget_page_t* background, widget_page_t* prospects)
{
    if (background == NULL || prospects == NULL) {
        return;
    }

    if (widget_get_align_center(background) || widget_get_align_center(prospects)) {
        widget_set_align_center(background, false);
        widget_set_pos(background, 0, 0);
        widget_set_align_center(prospects, false);
        widget_set_pos(prospects, 0, 0);
    }

    u32 sec = compo_cb.tm.sec;
    u16 ms = compo_cb.mtime;
    u32 div = GUI_SCREEN_HEIGHT * (sec*1000 + ms) / 60000;
    widget_set_location(prospects, 0, 0, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT - div);
}

static void func_clock_hourglass_trans(widget_page_t* background, widget_page_t* prospects, bool center)
{
    widget_set_align_center(background, true);
    if (center == true) {
        widget_set_pos(background, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    } else {
        widget_set_pos(background, 0, 0);
    }

    widget_set_align_center(prospects, center);
    if (center == true) {
        widget_set_pos(prospects, GUI_SCREEN_CENTER_X, (GUI_SCREEN_HEIGHT - GUI_SCREEN_HEIGHT * time_to_tm(compo_cb.rtc_cnt).sec / 60) / 2);
    } else {
        widget_set_pos(prospects, 0, 0);
    }
}

compo_form_t *func_clock_hourglass_form_create(void)
{
    u8 type = 0;
    u8 timer_type = 0;

    if (func_cb.sta == FUNC_CLOCK) {
        f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
        u16 data = f_clk->user_data & 0xffff;
        type = data & 0xff;
        timer_type = data >> 8;
    }

    printf("%s->%d\n", __func__, type);
    tft_set_temode(0);

    compo_form_t *frm = compo_form_create(true);        //page_body 前景, page 背景

    widget_set_align_center(frm->page, false);
    widget_set_location(frm->page, 0, 0, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    widget_set_align_center(frm->page_body, false);
    widget_set_location(frm->page_body, 0, 0, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    ///背景
    compo_shape_t* background   = compo_shape_create_for_page(frm, frm->page, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_location(background, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(background, dia_type_color[type].background);
    compo_shape_set_alpha(background, dia_type_color[type].alph);
    compo_setid(background, COMPO_ID_BACKGROUND_COLOR);

    compo_number_t* hour_h_b = compo_number_create_for_page(frm, frm->page, info[COMPO_ID_BACKGROUND_HOUR_H].res, 1);
    compo_number_set_pos(hour_h_b, info[COMPO_ID_BACKGROUND_HOUR_H].x, info[COMPO_ID_BACKGROUND_HOUR_H].y);
    widget_image_set_color(hour_h_b->img_num[0], dia_type_color[timer_type].background_num);
    compo_bonddata(hour_h_b, COMPO_BOND_HOUR_H);
    compo_setid(hour_h_b, COMPO_ID_BACKGROUND_HOUR_H);

    compo_number_t* hour_l_b = compo_number_create_for_page(frm, frm->page, info[COMPO_ID_BACKGROUND_HOUR_L].res, 1);
    compo_number_set_pos(hour_l_b, info[COMPO_ID_BACKGROUND_HOUR_L].x, info[COMPO_ID_BACKGROUND_HOUR_L].y);
    widget_image_set_color(hour_l_b->img_num[0], dia_type_color[timer_type].background_num);
    compo_bonddata(hour_l_b, COMPO_BOND_HOUR_L);
    compo_setid(hour_l_b, COMPO_ID_BACKGROUND_HOUR_L);

    compo_number_t* min_h_b = compo_number_create_for_page(frm, frm->page, info[COMPO_ID_BACKGROUND_MIN_H].res, 1);
    compo_number_set_pos(min_h_b, info[COMPO_ID_BACKGROUND_MIN_H].x, info[COMPO_ID_BACKGROUND_MIN_H].y);
    widget_image_set_color(min_h_b->img_num[0], dia_type_color[timer_type].background_num);
    compo_bonddata(min_h_b, COMPO_BOND_MINUTE_H);
    compo_setid(min_h_b, COMPO_ID_BACKGROUND_MIN_H);

    compo_number_t* min_l_b = compo_number_create_for_page(frm, frm->page, info[COMPO_ID_BACKGROUND_MIN_L].res, 1);
    compo_number_set_pos(min_l_b, info[COMPO_ID_BACKGROUND_MIN_L].x, info[COMPO_ID_BACKGROUND_MIN_L].y);
    widget_image_set_color(min_l_b->img_num[0], dia_type_color[timer_type].background_num);
    compo_bonddata(min_l_b, COMPO_BOND_MINUTE_L);
    compo_setid(min_l_b, COMPO_ID_BACKGROUND_MIN_L);

    ///前景
    compo_shape_t* prospects    = compo_shape_create_for_page(frm, frm->page_body, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_location(prospects, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_color(prospects, dia_type_color[type].prospects);
    compo_shape_set_alpha(prospects, dia_type_color[type].alph);
    compo_setid(prospects, COMPO_ID_PROSPECTS_COLOR);

    compo_number_t* hour_h_p = compo_number_create_for_page(frm, frm->page_body, info[COMPO_ID_PROSPECTS_HOUR_H].res, 1);
    compo_number_set_pos(hour_h_p, info[COMPO_ID_PROSPECTS_HOUR_H].x, info[COMPO_ID_PROSPECTS_HOUR_H].y);
    widget_image_set_color(hour_h_p->img_num[0], dia_type_color[timer_type].prospects_num);
    compo_bonddata(hour_h_p, COMPO_BOND_HOUR_H);
    compo_setid(hour_h_p,COMPO_ID_PROSPECTS_HOUR_H);

    compo_number_t* hour_l_p = compo_number_create_for_page(frm, frm->page_body, info[COMPO_ID_PROSPECTS_HOUR_L].res, 1);
    compo_number_set_pos(hour_l_p, info[COMPO_ID_PROSPECTS_HOUR_L].x, info[COMPO_ID_PROSPECTS_HOUR_L].y);
    widget_image_set_color(hour_l_p->img_num[0], dia_type_color[timer_type].prospects_num);
    compo_bonddata(hour_l_p, COMPO_BOND_HOUR_L);
    compo_setid(hour_l_p, COMPO_ID_PROSPECTS_HOUR_L);

    compo_number_t* min_h_p = compo_number_create_for_page(frm, frm->page_body, info[COMPO_ID_PROSPECTS_MIN_H].res, 1);
    compo_number_set_pos(min_h_p, info[COMPO_ID_PROSPECTS_MIN_H].x, info[COMPO_ID_PROSPECTS_MIN_H].y);
    widget_image_set_color(min_h_p->img_num[0], dia_type_color[timer_type].prospects_num);
    compo_bonddata(min_h_p, COMPO_BOND_MINUTE_H);
    compo_setid(min_h_p, COMPO_ID_PROSPECTS_MIN_H);

    compo_number_t* min_l_p = compo_number_create_for_page(frm, frm->page_body, info[COMPO_ID_PROSPECTS_MIN_L].res, 1);
    compo_number_set_pos(min_l_p, info[COMPO_ID_PROSPECTS_MIN_L].x, info[COMPO_ID_PROSPECTS_MIN_L].y);
    widget_image_set_color(min_l_p->img_num[0], dia_type_color[timer_type].prospects_num);
    compo_bonddata(min_l_p, COMPO_BOND_MINUTE_L);
    compo_setid(min_l_p, COMPO_ID_PROSPECTS_MIN_L);

    //tst
//    widget_set_location(frm->page_body, 0, 0, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT*0.3);

    func_clock_hourglass_update(frm->page, frm->page_body);

    if (func_cb.sta != FUNC_CLOCK) {
        func_clock_hourglass_trans(frm->page, frm->page_body, true);
    } else {
//        type++;
    }

    return frm;

}

void func_clock_hourglass_process(void)
{
    func_clock_hourglass_update(func_cb.frm_main->page, func_cb.frm_main->page_body);
}

void func_clock_hourglass_message(size_msg_t msg)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    u16 data = f_clk->user_data & 0xffff;
    u8 type = data & 0xff;
    u8 timer_type = data >> 8;


    switch (msg) {
    case MSG_CTP_SHORT_UP:
        func_clock_hourglass_trans(func_cb.frm_main->page, func_cb.frm_main->page_body, true);
        func_switch_to(FUNC_CARD, FUNC_SWITCH_MENU_PULLUP_UP | FUNC_SWITCH_DOWN_BG_BLUR);
        break;


    case MSG_CTP_SHORT_LEFT:
        func_clock_hourglass_trans(func_cb.frm_main->page, func_cb.frm_main->page_body, true);
        func_message(msg);
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_clock_hourglass_trans(func_cb.frm_main->page, func_cb.frm_main->page_body, true);
		func_switch_to(FUNC_SIDEBAR, func_get_switching_mode_byidx(sys_cb.nav_index, false));  //右滑界面
        break;

    case MSG_CTP_SHORT_DOWN:
        func_clock_hourglass_trans(func_cb.frm_main->page, func_cb.frm_main->page_body, true);
        func_clock_sub_dropdown();
        break;

    case MSG_CTP_CLICK: {
        compo_shape_t* background = compo_getobj_byid(COMPO_ID_BACKGROUND_COLOR);
        compo_shape_t* prospects = compo_getobj_byid(COMPO_ID_PROSPECTS_COLOR);
        if (background != NULL && prospects != NULL) {
            type++;
            if (type >= DIA_TYPE_CNT) {
                type = 0;
            }
            compo_shape_set_color(background, dia_type_color[type].background);
            compo_shape_set_alpha(background, dia_type_color[type].alph);
            compo_shape_set_color(prospects, dia_type_color[type].prospects);
            compo_shape_set_alpha(prospects, dia_type_color[type].alph);

        }

        compo_number_t* hour_h_b = compo_getobj_byid(COMPO_ID_BACKGROUND_HOUR_H);
        compo_number_t* hour_l_b = compo_getobj_byid(COMPO_ID_BACKGROUND_HOUR_L);
        compo_number_t* min_h_b = compo_getobj_byid(COMPO_ID_BACKGROUND_MIN_H);
        compo_number_t* min_l_b = compo_getobj_byid(COMPO_ID_BACKGROUND_MIN_L);
        compo_number_t* hour_h_p = compo_getobj_byid(COMPO_ID_PROSPECTS_HOUR_H);
        compo_number_t* hour_l_p = compo_getobj_byid(COMPO_ID_PROSPECTS_HOUR_L);
        compo_number_t* min_h_p = compo_getobj_byid(COMPO_ID_PROSPECTS_MIN_H);
        compo_number_t* min_l_p = compo_getobj_byid(COMPO_ID_PROSPECTS_MIN_L);
        timer_type++;
        if (timer_type >= DIA_TYPE_CNT) {
            timer_type = 0;
        }
        widget_image_set_color(hour_h_b->img_num[0], dia_type_color[timer_type].background_num);
        widget_set_alpha(hour_h_b->img_num[0], dia_type_color[timer_type].alph);
        widget_image_set_color(hour_l_b->img_num[0], dia_type_color[timer_type].background_num);
        widget_set_alpha(hour_l_b->img_num[0], dia_type_color[timer_type].alph);
        widget_image_set_color(min_h_b->img_num[0], dia_type_color[timer_type].background_num);
        widget_set_alpha(min_h_b->img_num[0], dia_type_color[timer_type].alph);
        widget_image_set_color(min_l_b->img_num[0], dia_type_color[timer_type].background_num);
        widget_set_alpha(min_l_b->img_num[0], dia_type_color[timer_type].alph);

        widget_image_set_color(hour_h_p->img_num[0], dia_type_color[timer_type].prospects_num);
        widget_set_alpha(hour_h_p->img_num[0], dia_type_color[timer_type].alph);
        widget_image_set_color(hour_l_p->img_num[0], dia_type_color[timer_type].prospects_num);
        widget_set_alpha(hour_l_p->img_num[0], dia_type_color[timer_type].alph);
        widget_image_set_color(min_h_p->img_num[0], dia_type_color[timer_type].prospects_num);
        widget_set_alpha(min_h_p->img_num[0], dia_type_color[timer_type].alph);
        widget_image_set_color(min_l_p->img_num[0], dia_type_color[timer_type].prospects_num);
        widget_set_alpha(min_l_p->img_num[0], dia_type_color[timer_type].alph);
        f_clk->user_data = (timer_type) << 8 | type;
    } break;

    case MSG_CTP_LONG:
        if (func_clock_preview_get_type() == PREVIEW_ROTARY_STYLE) {
            func_cb.sta = FUNC_CLOCK_PREVIEW;
        } else {
            func_switch_to(FUNC_CLOCK_PREVIEW, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);                    //切换回主时钟
        }
        break;

    case KU_BACK:
        func_clock_hourglass_trans(func_cb.frm_main->page, func_cb.frm_main->page_body, false);                    //返回动画居中
        func_message(msg);
        break;

    default:
        func_message(msg);
        break;
    }
}

