#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_sport_config_t_ {

} f_sport_config_t;

enum {
    COMPO_ID_BTN_NUM0 = 1,
    COMPO_ID_BTN_NUM1,
    COMPO_ID_BTN_NUM2,
    COMPO_ID_BTN_NUM3,

};
#define SPORT_DISP_BUT_ITEM_CNT    ((int)(sizeof(tbl_sport_disp_but_item) / sizeof(tbl_sport_disp_but_item[0])))

typedef struct sport_disp_butt_item_t_ {
    u32 res_addr;
    u16 btn_id;
    s16 x;
    s16 y;
} sport_disp_butt_item_t;

//图片item，创建时遍历一下
static const sport_disp_butt_item_t tbl_sport_disp_but_item[] = {
    {UI_BUF_SPORT_EXERCISING_KCAL_BIN,     COMPO_ID_BTN_NUM0,        60,  120},
    {UI_BUF_SPORT_EXERCISING_KM_BIN,       COMPO_ID_BTN_NUM1,        260, 120},
    {UI_BUF_SPORT_EXERCISING_STEP_BIN,     COMPO_ID_BTN_NUM2,        60,  260},
    {UI_BUF_SPORT_EXERCISING_TIME_BIN,     COMPO_ID_BTN_NUM3,        260, 260},
};


//创建运动配置窗体
compo_form_t *func_sport_config_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
    compo_form_set_alpha(frm, 200);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_SPORTS_CONFIG]);

    //新建按钮
	compo_button_t *btn;
    for (u8 idx = 0; idx < SPORT_DISP_BUT_ITEM_CNT; idx++) {
        btn = compo_button_create_by_image(frm, tbl_sport_disp_but_item[idx].res_addr);
        compo_setid(btn, tbl_sport_disp_but_item[idx].btn_id);
        compo_button_set_pos(btn, tbl_sport_disp_but_item[idx].x, tbl_sport_disp_but_item[idx].y);
    }

	//创建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 2);
    compo_textbox_set_pos(txt, 30, 160);
    compo_textbox_set_align_center(txt, false);
    compo_textbox_set(txt, "消耗");

    compo_textbox_t *txt_k = compo_textbox_create(frm, 2);
    compo_textbox_set_pos(txt_k, 230, 160);
    compo_textbox_set_align_center(txt_k, false);
    compo_textbox_set(txt_k, "距离");

    compo_textbox_t *txt_l = compo_textbox_create(frm, 2);
    compo_textbox_set_pos(txt_l, 30, 300);
    compo_textbox_set_align_center(txt_l, false);
    compo_textbox_set(txt_l, "步数");

    compo_textbox_t *txt_t = compo_textbox_create(frm, 2);
    compo_textbox_set_align_center(txt_t, false);
    compo_textbox_set_pos(txt_t, 230, 300);
    compo_textbox_set(txt_t, "时间");

    return frm;
}

//运动配置功能事件处理
static void func_sport_config_process(void)
{
    func_process();
}

//运动配置功能消息处理
static void func_sport_config_message(size_msg_t msg)
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


    default:
        func_message(msg);
        break;
    }
}

//进入运动配置功能
void func_sport_config_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_sport_config_t));
    func_cb.frm_main = func_sport_config_form_create();
}

//退出运动配置功能
void func_sport_config_exit(void)
{
    func_cb.last = FUNC_SPORT_CONFIG;
}

//运动配置功能
void func_sport_config(void)
{
    printf("%s\n", __func__);
    func_sport_config_enter();
    while (func_cb.sta == FUNC_SPORT_CONFIG) {
        func_sport_config_process();
        func_sport_config_message(msg_dequeue());
    }
    func_sport_config_exit();
}
