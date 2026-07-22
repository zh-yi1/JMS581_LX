#include "include.h"
#include "func.h"

#define  TRACE_EN       0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define GEO_TEST_EN     1       //测试

//  步进 最大值 最小值
#define  MAX_STEP   40
#define  MIX_STEP   4

enum {
    //文本
    COMPO_ID_DIR_TEXT = 1,
    COMPO_ID_ORI_TEXT,

    //图片
    COMPO_ID_BIG_BG_PIC,
    COMPO_ID_MID_BG_PIC,
    COMPO_ID_SMALL_BG_PIC,
    COMPO_ID_BG2_PIC,
    COMPO_ID_BG3_PIC,
    COMPO_ID_LIGHT_PIC,

    //按钮
    COMPO_ID_BTN,
};

enum {
    COMPASS_DIRECTION_S = 0,
    COMPASS_DIRECTION_WS,
    COMPASS_DIRECTION_W,
    COMPASS_DIRECTION_WN,
    COMPASS_DIRECTION_N,
    COMPASS_DIRECTION_EN,
    COMPASS_DIRECTION_E,
    COMPASS_DIRECTION_ES,
};

enum {
    COMPASS_AUTO_ROTATE_STOP = 0,   //没有旋转
    COMPASS_AUTO_ROTATE_NORMAL,     //正常旋转
};

u16 func_clock_preview_get_type(void);

typedef struct f_compass_t_ {
    int ori;          //最终指示
    int current_ori;  //当前指示
    int step_ori;
    u32 tick;
    u32 ro_tick;
    u8 sub_form;
#if GEO_TEST_EN
    u8 test_dir;
    int ori_tmp;
#endif
} f_compass_t;

f_compass_t f_clock_compass;

const char* compass_char_direction[] = {
    "S", "WS", "W", "WN", "N", "EN", "E", "ES",
};

static u8 func_clock_compass_get_direction(s16 ori)
{
    u8 i = 0;
    if ((ori >= 0 && ori <= 22) || (ori > 337 && ori <= 360)) { //北
        i= COMPASS_DIRECTION_N;
    } else if ((ori > 22) && (ori <= 67)) {     //东北
        i= COMPASS_DIRECTION_EN;
    }  else if ((ori > 67) && (ori <= 112)) {   //东
        i= COMPASS_DIRECTION_E;
    } else if ((ori > 112) && (ori <= 157)) {   //东南
        i= COMPASS_DIRECTION_ES;
    } else if ((ori > 157) && (ori <= 202)) {   //南
        i= COMPASS_DIRECTION_S;
    } else if ((ori > 202) && (ori <= 247)) {   //西南
        i= COMPASS_DIRECTION_WS;
    } else if ((ori > 247) && (ori <= 292)) {   //西
        i= COMPASS_DIRECTION_W;
    } else if ((ori > 292) && (ori <= 337)) {   //西北
        i= COMPASS_DIRECTION_WN;
    }

    return i;
}

static void func_clock_compass_update(s16 ori)
{
    // 保证在0-3600之间
    ori =  (ori%3600 + 3600)%3600;

    f_compass_t *f_compass = &f_clock_compass;
    compo_picturebox_t *pic1, *pic2;
    compo_textbox_t *txt1, *txt2;

    if (f_compass->sub_form) {
        pic1 = compo_getobj_byid(COMPO_ID_BG3_PIC);
        pic2 = compo_getobj_byid(COMPO_ID_LIGHT_PIC);
    } else {
        pic1 = compo_getobj_byid(COMPO_ID_BIG_BG_PIC);
        pic2 = compo_getobj_byid(COMPO_ID_SMALL_BG_PIC);
        txt1 = compo_getobj_byid(COMPO_ID_DIR_TEXT);
        txt2 = compo_getobj_byid(COMPO_ID_ORI_TEXT);

        char buf[10];
        snprintf(buf, sizeof(buf), "%s", compass_char_direction[func_clock_compass_get_direction(ori/10)]);
        compo_textbox_set(txt1, buf);
        snprintf(buf, sizeof(buf), "%03d°", ori/10);
        compo_textbox_set(txt2, buf);
    }
    compo_picturebox_set_rotation(pic1, -ori);
    compo_picturebox_set_rotation(pic2, -ori);

    TRACE("[%d]upfade ori:%d\n", f_compass->sub_form, ori);
}

#if GEO_TEST_EN
// 读取地磁数据(测试)
void fit_geo_get_result_test(int *ori)
{
    //测试数据
    f_compass_t *f_compass = &f_clock_compass;
    if (f_compass->test_dir) {
        *ori = 0;
    } else {
        *ori = 3600;
    }

    return;
}
#endif

static s16 func_clock_compass_get_step(void)
{
    s16 step = 0;
    f_compass_t *f_compass = &f_clock_compass;
    step = (f_compass->ori - f_compass->current_ori)/8;

    if (step > 0) {
        if (step > MAX_STEP) {
            step = MAX_STEP;
        } else if (step < MIX_STEP) {
            step = MIX_STEP;
        }
    } else if (step < 0) {
        if (step < (0 - MAX_STEP) ) {
            step = (0 - MAX_STEP);
        } else if (step > (0 - MIX_STEP)) {
            step = (0 - MIX_STEP);
        }
    }
    TRACE("ori:%d, cur_ori:%d, step:%d\n", f_compass->ori, f_compass->current_ori, step);
    return step;
}

void func_clock_compass_init()
{
    f_compass_t *f_compass = &f_clock_compass;
    f_compass->tick = tick_get();
    f_compass->ro_tick = tick_get();
#if (SENSOR_GEO_SEL == SENSOR_GEO_QMC6309)
    qmc6309_20ms_process();
    fit_qmc6309_get_result(&f_compass->ori);
    f_compass->ori *= 10;
    f_compass->current_ori = f_compass->ori;
    func_clock_compass_update(f_compass->current_ori);
#elif GEO_TEST_EN
    func_clock_compass_update(f_compass->current_ori);
#endif
}

//创建指南针窗体
compo_form_t *func_clock_compass_main_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);
    int ori =  (f_clock_compass.current_ori%3600 + 3600)%3600;

    compo_picturebox_t *pic;
    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_COMPASS_BIG_BG_BIN);
    compo_setid(pic, COMPO_ID_BIG_BG_PIC);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_rotation(pic, -ori);

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_COMPASS_MID_BG_BIN);
    compo_setid(pic, COMPO_ID_MID_BG_PIC);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_COMPASS_SMALL_BG_BIN);
    compo_setid(pic, COMPO_ID_SMALL_BG_PIC);
    compo_picturebox_set_pos(pic, 218, 294);
    compo_picturebox_set_rotation(pic, -ori);

    //新建按钮
    area_t area = gui_image_get_size(UI_BUF_DIALPLATE_COMPASS_SMALL_BG_BIN);
	compo_button_t *btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN);
    compo_button_set_location(btn, 218, 294, area.wid, area.hei);

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_COMPASS_STOPWATCH_BIN);
    compo_picturebox_set_pos(pic, 50, 330);

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_COMPASS_BLOCK_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, 72);

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_COMPASS_HR_1_BIN);
    compo_picturebox_set_pos(pic, 80, 66);

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_COMPASS_WEA_30_31_BIN);
    compo_picturebox_set_pos(pic, 240, 66);

    compo_number_t *num = compo_number_create(frm, UI_BUF_COMMON_NUM_LEFT_18_18_BIN, 1);
    compo_number_set_radix(num, 10, false);
    compo_number_set(num, 6);
    compo_number_set_pos(num, 30, 60);

    num = compo_number_create(frm, UI_BUF_COMMON_NUM_LEFT_18_18_BIN, 1);
    compo_number_set_radix(num, 10, false);
    compo_number_set(num, 3);
    compo_number_set_pos(num, 44, 48);

    num = compo_number_create(frm, UI_BUF_COMMON_NUM_RIGHT_18_18_BIN, 1);
    compo_number_set_radix(num, 10, false);
    compo_number_set(num, 3);
    compo_number_set_pos(num, 238, 24);

    num = compo_number_create(frm, UI_BUF_COMMON_NUM_RIGHT_18_18_BIN, 1);
    compo_number_set_radix(num, 10, false);
    compo_number_set(num, 0);
    compo_number_set_pos(num, 254, 34);


    char buf[10];
    compo_textbox_t *txt = compo_textbox_create(frm, 5);
    compo_setid(txt, COMPO_ID_DIR_TEXT);
    compo_textbox_set_pos(txt, 130, 132);
    snprintf(buf, sizeof(buf), "%s", compass_char_direction[func_clock_compass_get_direction(ori/10)]);
    compo_textbox_set(txt, buf);

    txt = compo_textbox_create(frm, 5);
    compo_setid(txt, COMPO_ID_ORI_TEXT);
    compo_textbox_set_pos(txt, 190, 132);
    snprintf(buf, sizeof(buf), "%03d°", ori/10);
    compo_textbox_set(txt, buf);

    pic = compo_picturebox_create(frm, UI_BUF_COMMON_COLON_NUM_16_24_BIN);    // :
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    compo_number_t *hour = compo_number_create(frm, UI_BUF_DIALPLATE_COMPASS_NUM_38_50_BIN, 2);
    compo_number_set_radix(hour, 10, false);
    compo_number_set_zfill(hour, true);
    compo_number_set(hour, compo_cb.tm.hour);
    compo_number_set_pos(hour, 110, GUI_SCREEN_CENTER_Y);
    compo_bonddata(hour, COMPO_BOND_HOUR);

    compo_number_t *minute = compo_number_create(frm, UI_BUF_DIALPLATE_COMPASS_NUM_38_50_BIN, 2);
    compo_number_set_radix(minute, 10, false);
    compo_number_set_zfill(minute, true);
    compo_number_set(minute, compo_cb.tm.min);
    compo_number_set_pos(minute, 210, GUI_SCREEN_CENTER_Y);
    compo_bonddata(minute, COMPO_BOND_MINUTE);

    return frm;
}

//创建指南针子窗体
static compo_form_t *func_clock_compass_sub_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);
    int ori =  (f_clock_compass.current_ori%3600 + 3600)%3600;

    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_COMPASS_BG2_BIN);
    compo_setid(pic, COMPO_ID_BG2_PIC);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_COMPASS_BG3_BIN);
    compo_setid(pic, COMPO_ID_BG3_PIC);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_rotation(pic, -ori);

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_COMPASS_LIGHT_BIN);
    compo_picturebox_set_rotation_center(pic, 28, 83);
    compo_setid(pic, COMPO_ID_LIGHT_PIC);
    compo_picturebox_set_pos(pic, 160, 194);
    compo_picturebox_set_rotation(pic, -ori);

    return frm;
}

compo_form_t *func_clock_compass_form_create(void)
{
    compo_form_t *frm = NULL;
    if (f_clock_compass.sub_form) {
        frm = func_clock_compass_sub_form_create();
    } else {
        frm = func_clock_compass_main_form_create();
    }
    return frm;
}

//指南针功能事件处理
void func_clock_compass_process(void)
{
    f_compass_t *f_compass = &f_clock_compass;
#if (SENSOR_GEO_SEL == SENSOR_GEO_QMC6309)
    int ori;
    if (tick_check_expire(f_compass->tick, 20)) {
        f_compass->tick = tick_get();
        qmc6309_20ms_process();
        fit_qmc6309_get_result(&ori);
        ori *= 10;
        if (f_compass->ori != ori) {
            f_compass->ori = ori;
            TRACE("get geo ori: %d\n", f_compass->ori);
            f_compass->step_ori = func_clock_compass_get_step();
        }
    }
#elif GEO_TEST_EN
    fit_geo_get_result_test(&f_compass->ori_tmp);
    if (f_compass->ori != f_compass->ori_tmp) {
        f_compass->ori = f_compass->ori_tmp;
        TRACE("get geo ori: %d\n", f_compass->ori);
        f_compass->step_ori = func_clock_compass_get_step();
    }
#endif

    if (tick_check_expire(f_compass->ro_tick, 80)) {
        f_compass->ro_tick = tick_get();
        f_compass->current_ori += f_compass->step_ori;
        f_compass->step_ori = func_clock_compass_get_step();
        if (f_compass->step_ori == 0) {
            TRACE("auto rotate stop !\n");
            f_compass->current_ori = f_compass->ori;
            #if GEO_TEST_EN
            f_compass->test_dir ^= 1;
            #endif
        }
        func_clock_compass_update(f_compass->current_ori);
    }
    func_process();
}


//单击按钮
static void func_clock_compass_button_click(void)
{
    f_compass_t *f_compass = &f_clock_compass;
    int id = compo_get_button_id();

    switch (id) {
    case COMPO_ID_BTN:
        compo_form_destroy(func_cb.frm_main);
        f_compass->sub_form = 1;
        func_cb.frm_main = func_clock_compass_form_create();
        func_clock_compass_update(f_compass->current_ori);
        break;

    default:
        break;
    }
}

//指南针功能消息处理
void func_clock_compass_message(size_msg_t msg)
{
    f_compass_t *f_compass = &f_clock_compass;
    switch (msg) {
    case MSG_CTP_CLICK:
        func_clock_compass_button_click();
        break;

    case KU_BACK:
        if (f_compass->sub_form) {
            compo_form_destroy(func_cb.frm_main);
            f_compass->sub_form = 0;
            func_cb.frm_main = func_clock_compass_form_create();
            func_clock_compass_update(f_compass->current_ori);
        } else {
            func_message(msg);
        }
        break;

    case MSG_CTP_SHORT_UP:
//        func_clock_sub_pullup();                //上拉菜单
        func_switch_to(FUNC_CARD, FUNC_SWITCH_MENU_PULLUP_UP | FUNC_SWITCH_DOWN_BG_BLUR);  //上拉卡片界面
        break;

    case MSG_CTP_SHORT_RIGHT:
//        func_clock_sub_side();                  //右拉边菜单
        func_switch_to(FUNC_SIDEBAR, func_get_switching_mode_byidx(sys_cb.nav_index, false));  //右滑界面
        break;

    case MSG_CTP_SHORT_DOWN:
        func_clock_sub_dropdown();              //下拉菜单
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

