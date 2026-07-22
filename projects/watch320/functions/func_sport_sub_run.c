#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_sport_sub_run_t_ {
    u8 sta;
    u8 min;                 //分
    u8 sec;                 //秒
    u16 msec;               //毫秒
    u32 total_msec;         //总毫秒
} f_sport_sub_run_t;

enum {
	COMPO_ID_NUM_SPORT_MIN = 1,     //分
	COMPO_ID_NUM_SPORT_SEC,         //秒
	COMPO_ID_NUM_SPORT_MSEC,        //毫秒
};

u16 colr[] = {COLOR_BLUE, COLOR_GREEN, 0xbb42, COLOR_RED};
#define SPORT_NUM_ITEM_CNT                       3

//创建室内跑步窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_sport_sub_run_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //compo_form_add_image(frm, UI_BUF_SPORT_EXERCISING_BG_HR_BIN, 160, 160);
    // 几个矩形 使用矩形控件替代  颜色稍微有点差异
    for (u8 i = 0; i< 4; i++) {
        compo_shape_t *shape_for = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
        compo_shape_set_color(shape_for, colr[i]);
        compo_shape_set_location(shape_for, 61 *(i + 1)+6, 160, 59, 38);
        compo_shape_set_radius(shape_for, 6);
        compo_shape_set_alpha(shape_for, 120);
    }
    compo_form_add_image(frm, UI_BUF_SPORT_EXERCISING_DOU2_BIN, 160, 340);


    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_INDOOR_RUN]);

	//创建数字
	s16 txt_x = 50;
	s16 txt_y = 50;
	s16 txt_x_offset = 78;
    for (u8 idx = 0; idx < SPORT_NUM_ITEM_CNT; idx++) {
        compo_textbox_t *txt = compo_textbox_create(frm, 2);
		compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
        compo_setid(txt, COMPO_ID_NUM_SPORT_MIN + idx);
        compo_textbox_set_align_center(txt, false);
        compo_textbox_set_pos(txt, txt_x + (idx * txt_x_offset), txt_y);
        compo_textbox_set(txt, "00");
    }

    return frm;
}

//室内跑步功能事件处理
static void func_sport_sub_run_process(void)
{
    f_sport_sub_run_t *f_sport_sub_run = (f_sport_sub_run_t *)func_cb.f_cb;
    //获取数字组件的地址
    compo_textbox_t *txt_min = compo_getobj_byid(COMPO_ID_NUM_SPORT_MIN);
    compo_textbox_t *txt_sec = compo_getobj_byid(COMPO_ID_NUM_SPORT_SEC);
    compo_textbox_t *txt_msec = compo_getobj_byid(COMPO_ID_NUM_SPORT_MSEC);

    char buf_min[4], buf_sec[4], buf_msec[6];
    static u32 sport_ticks = 0;

    if (tick_check_expire(sport_ticks, 10)) {
        sport_ticks = tick_get();

        f_sport_sub_run->total_msec += 10;
        f_sport_sub_run->min = ((f_sport_sub_run->total_msec / 1000) % 3600) / 60;
        f_sport_sub_run->sec = (f_sport_sub_run->total_msec / 1000) % 60;
        f_sport_sub_run->msec = f_sport_sub_run->total_msec % 1000;

        memset(buf_min, 0, sizeof(buf_min));
        memset(buf_sec, 0, sizeof(buf_sec));
        memset(buf_msec, 0, sizeof(buf_msec));
        snprintf(buf_min, sizeof(buf_min), "%02d", f_sport_sub_run->min);
        snprintf(buf_sec, sizeof(buf_sec), "%02d", f_sport_sub_run->sec);
        snprintf(buf_msec, sizeof(buf_msec), "%02d", f_sport_sub_run->msec / 10);

        compo_textbox_set(txt_min, buf_min);
        compo_textbox_set(txt_sec, buf_sec);
        compo_textbox_set(txt_msec, buf_msec);
  }

    func_process();
}

//室内跑步功能消息处理
static void func_sport_sub_run_message(size_msg_t msg)
{

    switch (msg) {
    case MSG_CTP_CLICK:
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;

    case MSG_QDEC_FORWARD:
    case MSG_QDEC_BACKWARD:
        break;

    case MSG_CTP_SHORT_LEFT:
        break;


    default:
        func_message(msg);
        break;
    }
}

//进入室内跑步功能
void func_sport_sub_run_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_sport_sub_run_t));
    func_cb.frm_main = func_sport_sub_run_form_create();
}

//退出室内跑步功能
void func_sport_sub_run_exit(void)
{
    func_cb.last = FUNC_SPORT_SUB_RUN;
}

//室内跑步功能
void func_sport_sub_run(void)
{
    printf("%s\n", __func__);
    func_sport_sub_run_enter();
    while (func_cb.sta == FUNC_SPORT_SUB_RUN) {
        func_sport_sub_run_process();
        func_sport_sub_run_message(msg_dequeue());
    }
    func_sport_sub_run_exit();
}
