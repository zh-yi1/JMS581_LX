#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define DOUSING_LIST_CNT                       ((int)(sizeof(tbl_dousing_list) / sizeof(tbl_dousing_list[0])))

enum {
    COMPO_ID_BTN_NUM0 = 0,
    COMPO_ID_BTN_NUM1,
    COMPO_ID_BTN_NUM2,
    COMPO_ID_BTN_NUM3,
    COMPO_ID_BTN_NUM4,
    COMPO_ID_BTN_NUM5,
    COMPO_ID_BTN_NUM6,
    COMPO_ID_BTN_NUM7,
    COMPO_ID_LISTBOX,
};

typedef struct f_dousing_list_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;
} f_dousing_list_t;

static const compo_listbox_item_t tbl_dousing_list[] = {
    {STR_FIVE_SEC},
    {STR_TEN_SEC},
    {STR_TWENTY_SEC},
    {STR_THIRTY_SEC},
    {STR_ONE_MIN},
    {STR_FIVE_MIN},
    {STR_NEVER},
};

//熄屏设置页面
compo_form_t *func_set_sub_dousing_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_SETTING_DOUSING]);

    //新建列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);
    compo_listbox_set(listbox, tbl_dousing_list, DOUSING_LIST_CNT);
    compo_listbox_set_bgimg(listbox, UI_BUF_COMMON_BG_BIN);
    compo_setid(listbox, COMPO_ID_LISTBOX);

    compo_listbox_set_item_text(listbox, 160, 40, GUI_SCREEN_WIDTH, 40, true);
    compo_listbox_set_focus_byidx(listbox, 1);
    compo_listbox_update(listbox);

    return frm;
}

//点进图标进入应用
void func_set_sub_dousing_list_icon_click(void)
{

    f_dousing_list_t *f_set = (f_dousing_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;
    int icon_idx = compo_listbox_select(listbox, ctp_get_sxy());

    if (icon_idx < 0 || icon_idx >= DOUSING_LIST_CNT) {
        return;
    }

    //切入应用
    switch(icon_idx) {
    case  COMPO_ID_BTN_NUM0...COMPO_ID_BTN_NUM6:
        if (icon_idx==COMPO_ID_BTN_NUM0) {
            sys_cb.sleep_time = 50;
        } else if (icon_idx==COMPO_ID_BTN_NUM1) {
            sys_cb.sleep_time = 100;
        } else if (icon_idx==COMPO_ID_BTN_NUM2) {
            sys_cb.sleep_time = 200;
        } else if (icon_idx==COMPO_ID_BTN_NUM3) {
            sys_cb.sleep_time = 300;
        } else if (icon_idx==COMPO_ID_BTN_NUM4) {
            sys_cb.sleep_time = 600;
        } else if (icon_idx==COMPO_ID_BTN_NUM5) {
            sys_cb.sleep_time = 3000;
        } else if (icon_idx==COMPO_ID_BTN_NUM6) {
            sys_cb.sleep_time = -1;
        }
        u8 func_sta = task_stack_pop();
        func_switch_to(func_sta, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);
        break;

    default:
        break;
    }
}

//熄屏时长功能事件处理
static void func_set_sub_dousing_list_process(void)
{
    f_dousing_list_t *f_set = (f_dousing_list_t *)func_cb.f_cb;
    compo_listbox_move(f_set->listbox);
    func_process();
}

//熄屏时长功能消息处理
static void func_set_sub_dousing_list_message(size_msg_t msg)
{
    f_dousing_list_t *f_set = (f_dousing_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_CLICK:
        func_set_sub_dousing_list_icon_click();                //单击图标
        break;


    case MSG_CTP_LONG:
        break;

    case KU_DELAY_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {

        }
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入设置功能
void func_set_sub_dousing_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_dousing_list_t));
    func_cb.frm_main = func_set_sub_dousing_form_create();

    f_dousing_list_t *f_set = (f_dousing_list_t *)func_cb.f_cb;
    f_set->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_set->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_set->mcb;//func_zalloc(sizeof(compo_listbox_move_cb_t));        //建立移动控制块，退出时需要释放
    //compo_listbox_move_init(listbox);

    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, DOUSING_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();
}

//退出设置功能
void func_set_sub_dousing_exit(void)
{
//    f_dousing_list_t *f_set = (f_dousing_list_t *)func_cb.f_cb;
//    compo_listbox_t *listbox = f_set->listbox;
//    func_free(listbox->mcb);                                            //释放移动控制块
    func_cb.last = FUNC_SET_SUB_DOUSING;
}

//设置功能
void func_set_sub_dousing(void)
{
    printf("%s\n", __func__);
    func_set_sub_dousing_enter();
    while (func_cb.sta == FUNC_SET_SUB_DOUSING) {
        func_set_sub_dousing_list_process();
        func_set_sub_dousing_list_message(msg_dequeue());
    }
    func_set_sub_dousing_exit();
}
