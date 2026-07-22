#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define SET_LIST_CNT                       ((int)(sizeof(tbl_setting_list) / sizeof(tbl_setting_list[0])))

enum {
    COMPO_ID_LISTBOX = 1,
};

typedef struct f_set_list_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;

} f_set_list_t;

static const compo_listbox_item_t tbl_setting_list[] = {
    {STR_SETTING_LIGHT,                  UI_BUF_SETTING_LIGHT_BIN,                      .func_sta = FUNC_LIGHT},            //亮度调节
    {STR_SETTING_MENU_NAVGAITON,         UI_BUF_SETTING_ABOUT_BIN,                      .func_sta = FUNC_SET_SUB_MENU_NAVIGATION},            //亮度调节
    //{STR_SETTING_DOUSING,                UI_BUF_SETTING_BREATHING_SCREEN_CLICK_BIN,     .func_sta = FUNC_SET_SUB_DOUSING},  //息屏时长
    {STR_SETTING_DOUSING,                UI_BUF_SETTING_BREATHING_SCREEN_BIN,           .func_sta = FUNC_SET_SUB_DOUSING},  //息屏时长
    {STR_SETTING_UP,                     UI_BUF_SETTING_WRIST_BIN,                      .func_sta = FUNC_SET_SUB_WRIST},    //抬腕亮屏
    //{STR_SETTING_DISTURD,                UI_BUF_SETTING_DISTURB_CLICK_BIN,              .func_sta = FUNC_SET_SUB_DISTURD},  //勿扰模式
    {STR_SETTING_DISTURD,                UI_BUF_SETTING_DISTURB_BIN,                    .func_sta = FUNC_SET_SUB_DISTURD},  //勿扰模式

    {STR_SETTING_SAV,                    UI_BUF_SETTING_SOUND_AND_VIBRATION_BIN,        .func_sta = FUNC_SET_SUB_SAV},      //声音与振动
    {STR_SETTING_LANGUAGE,               UI_BUF_SETTING_LANGUAGE_BIN,                   .func_sta = FUNC_SET_SUB_LANGUAGE}, //语言设置
    {STR_SETTING_TIME,                   UI_BUF_SETTING_TIME_BIN,                       .func_sta = FUNC_SET_SUB_TIME},     //时间设置
    {STR_SETTING_CALENDAR,               UI_BUF_SETTING_CALENDAR_BIN,                   .func_sta = FUNC_CALENDAER},        //日期设置
    {STR_SETTING_PASSWORD,               UI_BUF_SETTING_PASSWORD_BIN,                   .func_sta = FUNC_SET_SUB_PASSWORD}, //密码锁
    {STR_SETTING_ABOUT,                  UI_BUF_SETTING_ABOUT_BIN,                      .func_sta = FUNC_SET_SUB_ABOUT},    //关于
    {STR_SETTING_4G,                     UI_BUF_SETTING_ABOUT_BIN,                      .func_sta = FUNC_SET_SUB_4G},       //4G
    {STR_SETTING_RESTART,                UI_BUF_SETTING_RESTART_BIN,                    .func_sta = FUNC_SET_SUB_RESTART},  //重启
    {STR_SETTING_RSTFY,                  UI_BUF_SETTING_RESTORE_FACTORY_BIN,            .func_sta = FUNC_SET_SUB_RSTFY},    //恢复出厂
    {STR_SETTING_OFF,                    UI_BUF_SETTING_OFF_BIN,                        .func_sta = FUNC_SET_SUB_OFF},      //关机

#if AVI_DVP_DEMOLIST
    {STR_VIDEO_MODE,                    UI_BUF_SETTING_ABOUT_BIN,                       .func_sta = FUNC_VIDEO_SHOWLIST},       //3D效果内容演示
#endif // AVI_DVP_DEMOLIST

#if FUNC_MUSIC_EN
    {STR_SD_MUSIC,                      UI_BUF_SETTING_ABOUT_BIN,                       .func_sta = FUNC_MUSIC},       //3D效果内容演示
#endif // FUNC_MUSIC_EN

#if FUNC_RECORDER_EN
    {STR_MIC_RECORD,                    UI_BUF_SETTING_ABOUT_BIN,                       .func_sta = FUNC_RECORDER},
#endif // FUNC_RECORDER_EN

};

//创建主菜单窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_set_sub_list_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_SETTING]);

    //新建菜单列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);
    compo_listbox_set(listbox, tbl_setting_list, SET_LIST_CNT);
    compo_listbox_set_bgimg(listbox, UI_BUF_COMMON_BG_BIN);
    compo_setid(listbox, COMPO_ID_LISTBOX);

    u8 set_idx = sys_cb.set_idx;
    if (set_idx < 1) {
        set_idx = 1;
    }

    compo_listbox_set_focus_byidx(listbox, set_idx);
    compo_listbox_update(listbox);

    return frm;
}

//点进图标进入应用
static void func_set_sub_list_icon_click(void)
{
    int icon_idx;
    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;
    u8 func_sta;

    icon_idx = compo_listbox_select(listbox, ctp_get_sxy());
    if (icon_idx < 0 || icon_idx >= SET_LIST_CNT) {
        return;
    }

    //根据图标索引获取应用ID
    func_sta = tbl_setting_list[icon_idx].func_sta;
    //切入应用
    if (func_sta > 0) {
        if (sys_cb.nav_index == NAV_DRIFT) {
            func_switch_to(func_sta, func_get_switching_mode_byidx(sys_cb.nav_index, false) | FUNC_SWITCH_AUTO);
        } else {
            func_switch_to(func_sta, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);
        }
        sys_cb.set_idx = listbox->focus_icon_idx;
    }
}

//切换到设置菜单页面
static void func_set_sub_list_switch_to_clock(void)
{
    u8 func_sta = FUNC_CLOCK;
    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;
    widget_icon_t *icon = compo_listbox_select_byidx(listbox, 0);
    compo_form_t *frm = func_create_form(func_sta);
    func_switching(FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO, icon);
    compo_form_destroy(frm);
    func_cb.sta = func_sta;
    sys_cb.set_idx = 0;
}

//主菜单功能事件处理
static void func_set_sub_list_process(void)
{
    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;
    compo_listbox_move(f_set->listbox);
    func_process();
}

//进入设置主菜单页面
void func_set_sub_list_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_set_list_t));
    func_cb.frm_main = func_set_sub_list_form_create();

    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;
    f_set->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_set->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_set->mcb   ;//func_zalloc(sizeof(compo_listbox_move_cb_t));        //建立移动控制块，退出时需要释放
    printf("func_set_sub_enter:%x\r\n", listbox->mcb);
    compo_listbox_move_init_modify(listbox, 127, compo_listbox_gety_byidx(listbox, SET_LIST_CNT - 2));
    func_cb.enter_tick = tick_get();
}

//主菜单功能消息处理
static void func_set_sub_list_message(size_msg_t msg)
{
    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;

    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_CLICK:
        func_set_sub_list_icon_click();                //单击图标
        break;

    case MSG_CTP_LONG:
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_message(msg);
        sys_cb.set_idx = 0;
        break;

    case KU_DELAY_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_set_sub_list_switch_to_clock();       //返回设置界面
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

//退出菜单样式
void func_set_sub_exit(void)
{

//    f_set_list_t *f_set = (f_set_list_t *)func_cb.f_cb;

//    compo_listbox_t *listbox = f_set->listbox;
//    printf("func_set_sub_exit:%x\r\n", listbox->mcb);
//    func_free(listbox->mcb);                                            //释放移动控制块
    func_cb.last = FUNC_SETTING;
}

//主菜单功能
void func_set_sub_list(void)
{
    func_set_sub_list_enter();
    while (func_cb.sta == FUNC_SETTING) {
        func_set_sub_list_process();
        func_set_sub_list_message(msg_dequeue());
    }
    func_set_sub_exit();
}
