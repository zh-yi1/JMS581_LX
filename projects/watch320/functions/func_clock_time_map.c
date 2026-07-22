#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define SR_SHF  0
#define MR_SHF  10
#define STA_SHF  20
#define SWITCHING_FADE_SPEED            1                           //淡入淡出速度系数
#define SWITCHING_FADE_STEP             (1 << SWITCHING_FADE_SPEED) //淡入淡出速度，2的n次方
#define SWITCHING_TICK_EXPIRE           4                          //松手后自动切换单位时间(ms)

enum{
    COMPO_ID_BG_M = 1,
    COMPO_ID_BG_S,
    COMPO_ID_RING_M,
    COMPO_ID_RING_S,
};

enum{
    STA_IDLE,
    STA_RING_SEC,
    STA_RING_MIN,
};

u16 func_clock_preview_get_type(void);

u16 func_clock_time_map_r_get(bool is_sec)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    u16 value = is_sec ? ((f_clk->user_data >> SR_SHF) & 0x3ff) : ((f_clk->user_data >> MR_SHF) & 0x3ff);

    return value;
}

void func_clock_time_map_r_set(bool is_sec, u16 value)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;

    if (is_sec) {
        f_clk->user_data = (f_clk->user_data & ~(0x3ff << SR_SHF)) | (value << SR_SHF);
    } else {
        f_clk->user_data = (f_clk->user_data & ~(0x3ff << MR_SHF)) | (value << MR_SHF);
    }
}

void func_clock_visible_check(widget_image_t *img)
{
    if (widget_get_alpha(img) == 0) {
        widget_set_visible(img, false);
    } else if (widget_get_alpha(img) > 0 && widget_get_visble(img) == false) {
        widget_set_visible(img, true);
    }
}

void func_clock_time_map_cal_xy(s16 angle, u16 r, int *x, int *y)
{
    s32 sx = muls_shift16(r << 16, COS(angle + 900));
    s32 sy = muls_shift16(r << 16, SIN(angle + 900));
    float f_sx = (float)sx / 65536;
    float f_sy = (float)sy / 65536;

    *x = (int)f_sx;
    *y = (int)f_sy;
}

//单独淡入淡出
__attribute__((optimize("-O3")))
bool func_clock_time_map_switching_fade(widget_image_t *img1_in, widget_image_t *img2_in, widget_image_t *img1_out, widget_image_t *img2_out, widget_image_t *mid1, widget_image_t *mid2, u16 switch_mode, u16 sbg_size, u16 mbg_size, bool is_sec)
{
    u8 flag_pos;                                                //当前状态
    s32 cur_alpha;
    s32 fade_in_alpha;
    s32 fade_in_resume_alpha;
    u32 tick = tick_get();
    tm_t *tm = &compo_cb.tm;

    bool flag_resume = (img1_out != NULL && img2_out != NULL);
    int delt_sx = 0;
    int delt_sy = 0;
    int delt_mx = 0;
    int delt_my = 0;
    int delt_rsx = 0;
    int delt_rsy = 0;
    int delt_rmx = 0;
    int delt_rmy = 0;
    s16 angle_m = 0;
    s16 angle_s = 0;
    u16 rs_full_step = 0;
    u16 rm_full_step = 0;
    s16 rs_step = 0;
    s16 rm_step = 0;
    s16 r_step = 0;
    int delt_x = 0;
    int delt_y = 0;
    u8 fade_step = SWITCHING_FADE_STEP;                     //渐变速度
    u16 s_size_step = (sbg_size*5 - sbg_size) / (255 / fade_step);
    u16 m_size_step = (sbg_size*5 - mbg_size) / (255 / fade_step);


    rect_t rect_img1_in = widget_get_location(img1_in);
    rect_t rect_img1_out = widget_get_location(img1_out);

    rect_t rect_img2_in = widget_get_location(img1_in);     //防止警告
    rect_t rect_img2_out = widget_get_location(img1_in);    //防止警告

    widget_image_t *img1_use = img1_in;
    widget_image_t *img2_use = img2_in;

    widget_image_t *img1_out_use = img1_out;
    widget_image_t *img2_out_use = img2_out;

    switch (switch_mode) {
    case FUNC_SWITCH_FADE_OUT:
        cur_alpha = 255;
        fade_in_alpha = 255;
        fade_in_resume_alpha = flag_resume ? 255 << 4 : 255;
        break;

    default:
        halt(HALT_FUNC_SWITCH_ZOOM_MODE);
        return false;
    }

    if (flag_resume) {
        rect_t tmp_rect_img2_in = widget_get_location(img2_in);
        rect_t tmp_rect_img2_out = widget_get_location(img2_out);

        memcpy(&rect_img2_in, &tmp_rect_img2_in, sizeof(rect_t));
        memcpy(&rect_img2_out, &tmp_rect_img2_out, sizeof(rect_t));
    }

    for (;;) {
        flag_pos = FLAG_POS_NORM;
        if (tick_check_expire(tick, SWITCHING_TICK_EXPIRE)) {
            tick = tick_get();
            switch (switch_mode) {
            case FUNC_SWITCH_FADE_OUT:
                //淡出
                cur_alpha -= SWITCHING_FADE_STEP;
                fade_in_alpha -= SWITCHING_FADE_STEP << 3;
                fade_in_resume_alpha -= SWITCHING_FADE_STEP << 4;
                if (cur_alpha <= 0) {
                    cur_alpha = 0;
                    flag_pos = FLAG_POS_END;
                }
                break;

            default:
                halt(HALT_FUNC_SWITCH_ZOOM_MODE);
                break;
            }


            if (flag_resume) {
//                printf("### %d, mid1:%x\n", rect_img1_in.wid - rm_step, mid1);
                if ((img1_use != mid1) && (rect_img1_in.wid - rm_step < 900) && (mid1 != NULL)) {
//                    printf("AAA:%d, %d, %d, %d\n", rect_img1_in.x, rect_img1_in.y, rect_img1_in.wid, rect_img1_in.hei);
                    widget_set_location(mid1, rect_img1_in.x, rect_img1_in.y, rect_img1_in.wid, rect_img1_in.hei);
                    rect_t rect_mid1_in = widget_get_location(mid1);
                    memcpy(&rect_img1_in, &rect_mid1_in, sizeof(rect_t));
                    img1_use = mid1;
                    widget_set_alpha(img1_in, 0);
                    func_clock_visible_check(img1_in);
                    widget_set_location(img1_in, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, mbg_size, mbg_size);
                }

                if ((img2_use != mid2) && (rect_img2_in.wid - rs_step < 900) && (mid2 != NULL)) {
                    widget_set_location(mid2, rect_img2_in.x, rect_img2_in.y, rect_img2_in.wid, rect_img2_in.hei);
                    rect_t rect_mid2_in = widget_get_location(mid2);
                    memcpy(&rect_img2_in, &rect_mid2_in, sizeof(rect_t));
                    img2_use = mid2;
                    widget_set_alpha(img2_in, 0);
                    func_clock_visible_check(img2_in);
                    widget_set_location(img2_in, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, sbg_size, sbg_size);
                }
            } else {
                if ((img1_out_use != mid1) && (rect_img1_out.wid + r_step > 900) && (mid1 != NULL)) {
                    widget_set_location(mid1, rect_img1_out.x, rect_img1_out.y, rect_img1_out.wid, rect_img1_out.hei);
                    rect_t rect_mid1_out = widget_get_location(mid1);
                    memcpy(&rect_img1_out, &rect_mid1_out, sizeof(rect_t));
                    img1_out_use = mid1;
                    widget_set_alpha(img1_out, 0);
                    func_clock_visible_check(img1_out);
                    widget_set_location(img1_out, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, mbg_size, mbg_size);
                }
            }

            angle_m = tm->min * 60 + tm->sec;
            angle_s = tm->sec * 60 + compo_cb.mtime * 3 / 100 * 2;          //按33ms(0.2度)对齐
            rs_full_step = s_size_step * 255 / fade_step;
            rm_full_step = m_size_step * 255 / fade_step;
            rs_step = s_size_step * (255 - cur_alpha) / fade_step;
            rm_step = m_size_step * (255 - cur_alpha) / fade_step;
            r_step = is_sec ? rs_step : rm_step;

            func_clock_time_map_cal_xy(angle_s, rs_step >> 1, &delt_sx, &delt_sy);
            func_clock_time_map_cal_xy(angle_m, rm_step >> 1, &delt_mx, &delt_my);

            func_clock_time_map_cal_xy(angle_s, rs_full_step >> 1, &delt_rsx, &delt_rsy);
            func_clock_time_map_cal_xy(angle_m, rm_full_step >> 1, &delt_rmx, &delt_rmy);

            delt_x = is_sec ? delt_sx : delt_mx;
            delt_y = is_sec ? delt_sy : delt_my;

            s16 alpha_in_set = flag_resume ? fade_in_resume_alpha : fade_in_alpha;
            if (alpha_in_set < 0) {
                alpha_in_set = 0;
            } else if (alpha_in_set > 255) {
                alpha_in_set = 255;
            }

           s16 alpha_out_set = 255 - alpha_in_set;
           if (alpha_out_set > 255) {
                alpha_out_set = 255;
           }
            widget_set_alpha(img1_use, alpha_in_set);
            if (flag_resume) {
//                printf("AAA:%d, %d, %d, %d, alpha:%d\n", GUI_SCREEN_CENTER_X + delt_rmx - delt_mx, GUI_SCREEN_CENTER_Y + delt_rmy - delt_my, rect_img1_in.wid - rm_step, rect_img1_in.hei - rm_step, alpha_in_set);
                widget_set_location(img1_use, GUI_SCREEN_CENTER_X + delt_rmx - delt_mx, GUI_SCREEN_CENTER_Y + delt_rmy - delt_my, rect_img1_in.wid - rm_step, rect_img1_in.hei - rm_step);

            } else {
                widget_set_location(img1_use, GUI_SCREEN_CENTER_X + delt_x, GUI_SCREEN_CENTER_Y + delt_y, rect_img1_in.wid + r_step, rect_img1_in.hei + r_step);

            }

            func_clock_visible_check(img1_use);
            if (img2_use != NULL) {
                widget_set_alpha(img2_use, alpha_in_set);
                if (flag_resume) {
                    widget_set_location(img2_use, GUI_SCREEN_CENTER_X + delt_rsx - delt_sx, GUI_SCREEN_CENTER_Y + delt_rsy - delt_sy, rect_img2_in.wid - rs_step, rect_img2_in.hei - rs_step);
                }
                func_clock_visible_check(img2_use);
            }

//            printf("alpha_out_set:%d, alpha_in_set:%d\n", alpha_out_set, alpha_in_set);
            widget_set_alpha(img1_out_use, alpha_out_set);
            if (flag_resume) {
                widget_set_location(img1_out_use, GUI_SCREEN_CENTER_X + delt_rmx - delt_mx, GUI_SCREEN_CENTER_Y + delt_rmy - delt_my, rect_img1_out.wid - rm_step, rect_img1_out.hei - rm_step);
            } else {
                widget_set_location(img1_out_use, GUI_SCREEN_CENTER_X + delt_x, GUI_SCREEN_CENTER_Y + delt_y, rect_img1_out.wid + r_step, rect_img1_out.hei + r_step);    //使用in在长宽
//                printf("AAA:%d, %d, %d, %d\n", GUI_SCREEN_CENTER_X + delt_x, GUI_SCREEN_CENTER_Y + delt_y, rect_img1_out.wid + r_step, rect_img1_out.hei + r_step);
                func_clock_time_map_r_set(is_sec, r_step >> 1);
            }
            func_clock_visible_check(img1_out_use);


            if (img2_out != NULL) {
                widget_set_alpha(img2_out_use, alpha_out_set);
                func_clock_visible_check(img2_out_use);
                if (flag_resume) {
                    widget_set_location(img2_out_use, GUI_SCREEN_CENTER_X + delt_rsx - delt_sx, GUI_SCREEN_CENTER_Y + delt_rsy - delt_sy, rect_img2_out.wid - rs_step, rect_img2_out.hei - rs_step);
                }
            }
        }

        func_process();

        if (flag_pos != FLAG_POS_NORM) {
            break;
        }
    }

    return true;
}
bool func_clock_time_map_swithing(void)
{
    bool res;
    static u16 swid = 0;
    static u16 mwid = 0;
    bool flag_bg_visible = true;
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    u8 sta = (f_clk->user_data >> STA_SHF) & 0xf;

    if (sta >= (STA_RING_SEC - 1) && sta <= (STA_RING_MIN - 1)) {
        flag_bg_visible = false;
    }

    compo_picturebox_t *ring_m = compo_getobj_byid(COMPO_BOND_TIME_MAP_M);
    compo_picturebox_t *bg_m = compo_getobj_byid(COMPO_BOND_TMIE_MAP_BG_M);
    compo_picturebox_t *ring_s = compo_getobj_byid(COMPO_BOND_TIME_MAP_S);
    compo_picturebox_t *bg_s = compo_getobj_byid(COMPO_BOND_TMIE_MAP_BG_S);

    compo_picturebox_t *ring_m_mid = compo_getobj_byid(COMPO_BOND_TIME_MAP_M_MID);
    compo_picturebox_t *ring_s_mid = compo_getobj_byid(COMPO_BOND_TIME_MAP_S_MID);


    rect_t rect_bg_s = widget_get_location(bg_s->img);
    rect_t rect_bg_m = widget_get_location(bg_m->img);

    if (sta == STA_IDLE) {
        compo_bonddata(ring_s, COMPO_BOND_TIME_MAP_S);
        compo_bonddata(ring_s_mid, COMPO_BOND_TIME_MAP_S_MID);
        for(u8 i=0;i<4;i++) {
            compo_picturebox_t *pic = compo_getobj_byid(COMPO_BOND_TIME_MAP_LEFT_UP + i);
            compo_picturebox_set_visible(pic, flag_bg_visible);
        }

        swid = rect_bg_s.wid;
        mwid = rect_bg_m.wid;
        compo_picturebox_set_size(ring_s_mid, rect_bg_s.wid, rect_bg_s.hei);
        compo_picturebox_set_size(ring_m_mid, rect_bg_m.wid, rect_bg_m.hei);
        compo_picturebox_set_size(ring_m, 900, 900);
        compo_picturebox_set_size(ring_s, 900, 900);
        func_clock_time_map_switching_fade(bg_s->img, NULL, ring_s_mid->img, NULL, ring_s->img, NULL, FUNC_SWITCH_FADE_OUT, swid, mwid, true);
    } else if (sta == STA_RING_SEC) {
        compo_bonddata(ring_m, COMPO_BOND_TIME_MAP_M);
        compo_bonddata(ring_m_mid, COMPO_BOND_TIME_MAP_M_MID);
        func_clock_time_map_switching_fade(bg_m->img, NULL, ring_m_mid->img, NULL, ring_m->img, NULL, FUNC_SWITCH_FADE_OUT, swid, mwid, false);
    } else if (sta == STA_RING_MIN) {
        compo_bonddata(ring_m, COMPO_BOND_NONE);
        compo_bonddata(ring_s, COMPO_BOND_NONE);
        compo_bonddata(ring_m_mid, COMPO_BOND_NONE);
        compo_bonddata(ring_s_mid, COMPO_BOND_NONE);
        func_clock_time_map_switching_fade(ring_m->img, ring_s->img, bg_m->img, bg_s->img, ring_m_mid->img, ring_s_mid->img, FUNC_SWITCH_FADE_OUT, swid, mwid, true);
        for(u8 i=0;i<4;i++) {
            compo_picturebox_t *pic = compo_getobj_byid(COMPO_BOND_TIME_MAP_LEFT_UP + i);
            compo_picturebox_set_visible(pic, flag_bg_visible);
        }
    }

    res = (sta != STA_RING_MIN);
    sta++;

    if (sta > STA_RING_MIN) {
        sta = STA_IDLE;
    }

    f_clk->user_data = (f_clk->user_data & ~(0xf << STA_SHF)) | (sta << STA_SHF);

    return res;
}

void func_clock_time_map_init(void)
{
    tft_set_temode(2);
    //tft_set_baud(1, 4);
}

#if 0   //使用表盘工具
compo_form_t *func_clock_time_map_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);       //菜单一般创建在底层

    //创建背景图
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_TIME_MAP_BG_H_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_TIME_MAP_BG_M_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_setid(pic, COMPO_ID_BG_M);

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_TIME_MAP_BG_S_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_setid(pic, COMPO_ID_BG_S);

    compo_datetime_t *pointer = compo_datetime_create(frm, UI_BUF_DIALPLATE_TIME_MAP_H_BIN);
    compo_bonddata(pointer, COMPO_BOND_HOUR);
    compo_datetime_set_pos(pointer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    widget_set_alpha(pointer->img, 200);
    compo_datetime_set_center(pointer, 5, 5);
    compo_datetime_set_start_angle(pointer, 900);

    pointer = compo_datetime_create(frm, UI_BUF_DIALPLATE_TIME_MAP_M_BIN);
    compo_bonddata(pointer, COMPO_BOND_MINUTE);
    compo_datetime_set_pos(pointer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    widget_set_alpha(pointer->img, 200);
    compo_datetime_set_center(pointer, 6, 6);
    compo_datetime_set_start_angle(pointer, 900);

    pointer = compo_datetime_create(frm, UI_BUF_DIALPLATE_TIME_MAP_S_BIN);
    compo_bonddata(pointer, COMPO_BOND_SECOND);
    compo_datetime_set_pos(pointer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_datetime_set_center(pointer, 20, 6);
    compo_datetime_set_start_angle(pointer, 900);


    compo_picturebox_t*ring_m = compo_picturebox_create(frm, UI_BUF_DIALPLATE_TIME_MAP_RING_BIN);
    compo_picturebox_set_visible(ring_m, false);
    compo_picturebox_set_alpha(ring_m, 0);
    compo_picturebox_set_pos(ring_m, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
//    compo_picturebox_set_color(ring_m, COLOR_BLUE);
    compo_setid(ring_m, COMPO_ID_RING_M);

    compo_picturebox_t*ring_s = compo_picturebox_create(frm, UI_BUF_DIALPLATE_TIME_MAP_RING_BIN);
    compo_picturebox_set_visible(ring_s, false);
    compo_picturebox_set_alpha(ring_s, 0);
    compo_picturebox_set_pos(ring_s, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
//    compo_picturebox_set_color(ring_s, COLOR_BLUE);
    compo_setid(ring_s, COMPO_ID_RING_S);

//    compo_picturebox_t *get_ring_s = compo_getobj_byid(COMPO_ID_RING_S);
//    printf("set ring_s:%x, get_ring_s:%x, ID:%d\n", ring_s, get_ring_s, COMPO_ID_RING_S);


    return frm;
}

void func_clock_time_map_process(void)
{


}

void func_clock_time_map_message(size_msg_t msg)
{

    switch (msg) {
    case MSG_CTP_CLICK:
        func_clock_time_map_swithing();
        break;

    case MSG_CTP_SHORT_UP:
//        func_clock_sub_pullup();                //上拉菜单
        func_switch_to(FUNC_CARD, FUNC_SWITCH_MENU_PULLUP_UP);  //上拉卡片界面
        break;

    case MSG_CTP_SHORT_RIGHT:
//        func_clock_sub_side();                  //右拉边菜单
        func_switch_to(FUNC_SIDEBAR, func_get_switching_mode_byidx(sys_cb.nav_index, false));  //右滑界面
        break;

    case MSG_CTP_SHORT_DOWN:
        func_clock_sub_dropdown();              //下拉菜单
        break;

    case MSG_QDEC_FORWARD:                              //向前滚动菜单
        break;

    case MSG_QDEC_BACKWARD:                             //向后滚动菜单
        break;

    case MSG_CTP_LONG:
        if (func_clock_preview_get_type() == PREVIEW_ROTARY_STYLE) {
            func_cb.sta = FUNC_CLOCK_PREVIEW;
        } else {
            func_switch_to(FUNC_CLOCK_PREVIEW, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);                    //切换回主时钟
        }
        break;

    default:
        func_message(msg);
        break;
    }
}
#endif
