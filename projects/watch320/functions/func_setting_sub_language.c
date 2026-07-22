#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define LANGUAGE_LIST_CNT                       ((int)(sizeof(tbl_language_list) / sizeof(tbl_language_list[0])))

enum {
    COMPO_ID_LISTBOX = 1,
};

typedef struct f_language_list_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;

} f_language_list_t;

static const compo_listbox_item_t tbl_language_list[] = {
    {STR_LANGUAGE_CN},
    {STR_LANGUAGE_ENG},
    {STR_LANGUAGE_FN},
    {STR_LANGUAGE_RU},
    {STR_LANGUAGE_AT},
    {STR_LANGUAGE_JP},

};

//语言设置页面
compo_form_t *func_set_sub_language_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_SETTING_LANGUAGE]);

    //新建列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);
    compo_listbox_set(listbox, tbl_language_list, LANGUAGE_LIST_CNT);
    compo_listbox_set_bgimg(listbox, UI_BUF_COMMON_BG_BIN);
    compo_setid(listbox, COMPO_ID_LISTBOX);

    compo_listbox_set_item_text(listbox, 160, 40, GUI_SCREEN_WIDTH, 40, true);
    compo_listbox_set_focus_byidx(listbox, 1);
    compo_listbox_update(listbox);

    return frm;
}

//点进图标
void func_set_sub_language_list_icon_click(void)
{
    f_language_list_t *f_set = (f_language_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;
    int icon_idx = compo_listbox_select(listbox, ctp_get_sxy());


    if (icon_idx >= 0) {                                                                                //ls
        bsp_sys_reverse_ctlbit(tbl_language_list[icon_idx].vidx);
        compo_listbox_update(listbox);
        sys_cb.lang_id = icon_idx;
        if (func_cb.last == FUNC_MENU) {
            func_back_to();
        } else {
            u8 func_sta = task_stack_pop();
            func_switch_to(func_sta, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);

        }
    }
}

//语言设置功能事件处理
static void func_set_sub_language_list_process(void)
{
    f_language_list_t *f_set = (f_language_list_t *)func_cb.f_cb;
    compo_listbox_move(f_set->listbox);
    func_process();
}

//语言设置功能消息处理
static void func_set_sub_language_list_message(size_msg_t msg)
{
    f_language_list_t *f_set = (f_language_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        func_set_sub_language_list_icon_click();                //单击图标
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

//进入语言设置功能
void func_set_sub_language_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_language_list_t));
    func_cb.frm_main = func_set_sub_language_form_create();

    f_language_list_t *f_set = (f_language_list_t *)func_cb.f_cb;
    f_set->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_set->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_set->mcb;//func_zalloc(sizeof(compo_listbox_move_cb_t));        //建立移动控制块，退出时需要释放
   // compo_listbox_move_init(listbox);

    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, LANGUAGE_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();

}

//退出语言设置功能
void func_set_sub_language_exit(void)
{
//    f_language_list_t *f_set = (f_language_list_t *)func_cb.f_cb;
//    compo_listbox_t *listbox = f_set->listbox;
//    func_free(listbox->mcb);                                            //释放移动控制块
    func_cb.last = FUNC_SET_SUB_LANGUAGE;
}

//语言设置功能
void func_set_sub_language(void)
{
    printf("%s\n", __func__);
    func_set_sub_language_enter();
    while (func_cb.sta == FUNC_SET_SUB_LANGUAGE) {
        func_set_sub_language_list_process();
        func_set_sub_language_list_message(msg_dequeue());
    }
    func_set_sub_language_exit();
}
