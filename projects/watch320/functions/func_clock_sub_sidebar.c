#include "include.h"
#include "func.h"

#define TRACE_EN    0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define SIDEBAR_PAGE_HEIGHT 1082    //长图总高度
#define QDEC_STEP_Y         180     //编码器步进距离

#define TIMER_HOUR          (sys_cb.timer_left_sec / 3600)  //计时器数据
#define TIMER_MIN           ((sys_cb.timer_left_sec % 3600) / 60)
#define TIMER_SEC           (sys_cb.timer_left_sec % 60)
#define STOPWATCH_HOUR      (sys_cb.stopwatch_total_msec / 1000 / 3600)  //秒表数据
#define STOPWATCH_MIN       (((sys_cb.stopwatch_total_msec / 1000) % 3600) / 60)
#define STOPWATCH_SEC       ((sys_cb.stopwatch_total_msec / 1000) % 60)

#define DEFAULT_LATEST_TASK_NUM 3   //最近任务不足3个时默认值补上
const u8 latest_default[DEFAULT_LATEST_TASK_NUM] = {FUNC_SLEEP, FUNC_SPORT, FUNC_HEARTRATE};

//卡片/按钮id
enum{
    SIDEBAR_CARD_ID_TIME_WEATHER = 1,
    SIDEBAR_CARD_ID_TIMER,
    SIDEBAR_CARD_ID_STOPWATCH,
    SIDEBAR_CARD_ID_CALCULATOR,
    SIDEBAR_CARD_ID_CALENDAR,
    SIDEBAR_CARD_ID_GAME,
    SIDEBAR_CARD_ID_BREATHE,
    SIDEBAR_CARD_ID_LATEST,

    SIDEBAR_CARD_ID_CNT,
};

typedef struct f_sidebar_t_ {
	page_tp_move_t ptm;

    s8 m_time_min;  //时间记录，判断是否更新控件
    s8 m_timer_sec;
    s8 m_stopwatch_sec;
} f_sidebar_t;

//根据序号获取最近任务序号（idx=0为最近，无任务返回0）(idx<=3)
static u8 sidebar_get_latest_func(u8 idx)
{
    u8 i, j;
    u8 latest_default_tmp[DEFAULT_LATEST_TASK_NUM] = {0};
    u8 latest_cnt = latest_task_count();
    if (latest_cnt > idx) {
        return latest_task_get(idx);
    } else {
        j = 0;
        for (i = 0; i < DEFAULT_LATEST_TASK_NUM; i++) {   //最近任务不足DEFAULT_LATEST_TASK_NUM个且包含默认值
            if (latest_task_find(latest_default[i]) == -1) {
                latest_default_tmp[j] = latest_default[i];
                j++;
            }
        }
        return latest_default_tmp[idx - latest_cnt];
    }
}

//根据序号获取最近任务图标资源地址
static u32 sidebar_get_latest_icon(u8 idx)
{
    return func_get_icon_addr(sidebar_get_latest_func(idx));
}

//创建右滑菜单
compo_form_t * func_clock_sub_sidebar_form_create(void)
{
    compo_cardbox_t *cardbox;
    char str_buff[16];

    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //时间天气
    cardbox = compo_cardbox_create(frm, 0, 2, 4, 308, 156);
    compo_cardbox_set_pos(cardbox, 160, 126);
    compo_setid(cardbox, SIDEBAR_CARD_ID_TIME_WEATHER);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_BG_308_156_BIN);  //方框
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, 308, 156);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_WEATHER_WEATHER_LIST_BIN);  //天气图标
    compo_cardbox_icon_set_location(cardbox, 1, 245-160, 111-126, 96, 88);
    compo_cardbox_icon_cut(cardbox, 1, sys_cb.weather_idx, WEATHER_CNT);
    snprintf(str_buff, sizeof(str_buff), "%02d:%02d", compo_cb.tm.hour, compo_cb.tm.min);    //时间
    compo_cardbox_text_set_font(cardbox, 0, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_cardbox_text_set(cardbox, 0, str_buff);
    compo_cardbox_text_set_location(cardbox, 0, 98-160, 93-126, 180, 56);
    snprintf(str_buff, sizeof(str_buff), "%02d/%02d", compo_cb.tm.mon, compo_cb.tm.day);    //日期
    compo_cardbox_text_set_font(cardbox, 1, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 1, str_buff);
    compo_cardbox_text_set_location(cardbox, 1, 95-160, 145-126, 100, 30);
    compo_cardbox_text_set(cardbox, 2, i18n[STR_SUNDAY + compo_cb.tm.weekday]);    //星期
    compo_cardbox_text_set_location(cardbox, 2, 95-160, 181-126, 80, 30);
    snprintf(str_buff, sizeof(str_buff), "%02d℃", sys_cb.temperature[1]);    //温度
    compo_cardbox_text_set_font(cardbox, 3, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_cardbox_text_set(cardbox, 3, str_buff);
    compo_cardbox_text_set_location(cardbox, 3, 245-160, 178-126, 120, 30);
    //计时器
    cardbox = compo_cardbox_create(frm, 0, 2, 2, 308, 156);
    compo_cardbox_set_pos(cardbox, 160, 296);
    compo_setid(cardbox, SIDEBAR_CARD_ID_TIMER);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_BG_308_156_BIN);  //方框
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, 308, 156);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_ICON_TIMER_BIN);  //图标
    compo_cardbox_icon_set_location(cardbox, 1, 54-160, 270-296, 64, 64);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_TIMER]);    //计时器
    compo_cardbox_text_set_location(cardbox, 0, 140-160, 270-296, 120, 30);
    snprintf(str_buff, sizeof(str_buff), "%02lu:%02lu:%02lu", TIMER_HOUR, TIMER_MIN, TIMER_SEC);    //计时时间
    compo_cardbox_text_set_font(cardbox, 1, UI_BUF_0FONT_FONT_NUM_38_BIN);
    compo_cardbox_text_set(cardbox, 1, str_buff);
    compo_cardbox_text_set_location(cardbox, 1, 10, 330-296, 300, 50);
    //秒表
    cardbox = compo_cardbox_create(frm, 0, 2, 2, 308, 156);
    compo_cardbox_set_pos(cardbox, 160, 466);
    compo_setid(cardbox, SIDEBAR_CARD_ID_STOPWATCH);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_BG_308_156_BIN);  //方框
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, 308, 156);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_ICON_STOPWATCH_BIN);  //图标
    compo_cardbox_icon_set_location(cardbox, 1, 54-160, 440-466, 64, 64);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_STOP_WATCH]);    //秒表
    compo_cardbox_text_set_location(cardbox, 0, 129-160, 440-466, 120, 30);
    snprintf(str_buff, sizeof(str_buff), "%02lu:%02lu:%02lu", STOPWATCH_HOUR, STOPWATCH_MIN, STOPWATCH_SEC);    //计时时间
    compo_cardbox_text_set_font(cardbox, 1, UI_BUF_0FONT_FONT_NUM_38_BIN);
    compo_cardbox_text_set(cardbox, 1, str_buff);
    compo_cardbox_text_set_location(cardbox, 1, 10, 500-466, 300, 50);
    //计算器
    cardbox = compo_cardbox_create(frm, 0, 2, 1, 146, 146);
    compo_cardbox_set_pos(cardbox, 81, 631);
    compo_setid(cardbox, SIDEBAR_CARD_ID_CALCULATOR);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_BG_146_146_BIN);  //方框
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, 146, 146);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_ICON_CALCULATOR_BIN);  //图标
    compo_cardbox_icon_set_location(cardbox, 1, 60-81, 612-631, 72, 72);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_CALCULATOR]);    //计算器
    compo_cardbox_text_set_location(cardbox, 0, 0, 675-631, 140, 30);
    //日历
    cardbox = compo_cardbox_create(frm, 0, 2, 1, 146, 146);
    compo_cardbox_set_pos(cardbox, 241, 631);
    compo_setid(cardbox, SIDEBAR_CARD_ID_CALENDAR);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_BG_146_146_BIN);  //方框
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, 146, 146);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_ICON_CALENDAR_BIN);  //图标
    compo_cardbox_icon_set_location(cardbox, 1, 220-241, 612-631, 72, 72);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_CALENDAR]);    //日历
    compo_cardbox_text_set_location(cardbox, 0, 0, 675-631, 140, 30);
    //游戏
    cardbox = compo_cardbox_create(frm, 0, 2, 1, 146, 146);
    compo_cardbox_set_pos(cardbox, 81, 791);
    compo_setid(cardbox, SIDEBAR_CARD_ID_GAME);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_BG_146_146_BIN);  //方框
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, 146, 146);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_ICON_GAME_BIN);  //图标
    compo_cardbox_icon_set_location(cardbox, 1, 60-81, 772-791, 72, 72);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_GAME]);    //游戏
    compo_cardbox_text_set_location(cardbox, 0, 0, 835-791, 140, 30);
    //呼吸
    cardbox = compo_cardbox_create(frm, 0, 2, 1, 146, 146);
    compo_cardbox_set_pos(cardbox, 241, 791);
    compo_setid(cardbox, SIDEBAR_CARD_ID_BREATHE);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_BG_146_146_BIN);  //方框
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, 146, 146);
    compo_cardbox_icon_set(cardbox, 1, UI_BUF_ICON_BREATHE_BIN);  //图标
    compo_cardbox_icon_set_location(cardbox, 1, 220-241, 772-791, 72, 72);
    compo_cardbox_text_set(cardbox, 0, i18n[STR_BREATHE]);    //呼吸
    compo_cardbox_text_set_location(cardbox, 0, 0, 835-791, 140, 30);
    //最近应用

    cardbox = compo_cardbox_create(frm, 0, 5, 1, 308, 156);
    compo_cardbox_set_pos(cardbox, 160, 956);
    compo_setid(cardbox, SIDEBAR_CARD_ID_LATEST);
    compo_cardbox_icon_set(cardbox, 0, UI_BUF_SIDEBAR_BG_308_156_BIN);  //方框
    compo_cardbox_icon_set_location(cardbox, 0, 0, 0, 308, 156);




    //compo_cardbox_icon_set(cardbox, 1, UI_BUF_SIDEBAR_TOP_308_50_BIN);  //方框头
    //compo_cardbox_icon_set_location(cardbox, 1, 0, 903-956, 308, 50);

    compo_cardbox_icon_set(cardbox, 2, sidebar_get_latest_icon(0));  //图标0
    compo_cardbox_icon_set_location(cardbox, 2, 60-160, 979-956, 72, 72);
    compo_cardbox_icon_set(cardbox, 3, sidebar_get_latest_icon(1));  //图标1
    compo_cardbox_icon_set_location(cardbox, 3, 0, 979-956, 72, 72);
    compo_cardbox_icon_set(cardbox, 4, sidebar_get_latest_icon(2));  //图标2
    compo_cardbox_icon_set_location(cardbox, 4, 260-160, 979-956, 72, 72);

    //compo_cardbox_text_set(cardbox, 0, i18n[STR_LATEST_APP]);    //最近应用
    //compo_cardbox_text_set_location(cardbox, 0, 160-160, 908-956, 300, 40);

    compo_shape_t *shape_bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(shape_bg, 0x4208);
    compo_shape_set_location(shape_bg, 160, 956-52, 308, 50);
    compo_shape_set_radius(shape_bg, 12);

    compo_textbox_t *txt_off = compo_textbox_create(frm, 9);
    compo_textbox_set_align_center(txt_off, false);
    compo_textbox_set_pos(txt_off, 100, 956-64);
    compo_textbox_set(txt_off, i18n[STR_LATEST_APP]);

    return frm;
}

//刷新右滑组件Widget（时间、温度、图标等）
void func_clock_sub_sidebar_update(void)
{
    f_sidebar_t *f_sidebar = (f_sidebar_t *)func_cb.f_cb;
    compo_cardbox_t *cardbox;
    char str_buff[16];
    //时间，天气图标，温度
    if (f_sidebar->m_time_min != compo_cb.tm.min) { //一分钟更新一次
        cardbox = compo_getobj_byid(SIDEBAR_CARD_ID_TIME_WEATHER);
        snprintf(str_buff, sizeof(str_buff), "%02d:%02d", compo_cb.tm.hour, compo_cb.tm.min);    //时间
        compo_cardbox_text_set(cardbox, 0, str_buff);
        compo_cardbox_icon_cut(cardbox, 1, sys_cb.weather_idx, WEATHER_CNT);  //天气
        snprintf(str_buff, sizeof(str_buff), "%02d℃", sys_cb.temperature[1]);    //温度
        compo_cardbox_text_set(cardbox, 3, str_buff);
        //日期，星期
        if (f_sidebar->m_time_min <= 0) { //一小时更新一次
            snprintf(str_buff, sizeof(str_buff), "%02d/%02d", compo_cb.tm.mon, compo_cb.tm.day);    //日期
            compo_cardbox_text_set(cardbox, 1, str_buff);
            compo_cardbox_text_set(cardbox, 2, i18n[STR_SUNDAY + compo_cb.tm.weekday]);    //星期
        }
        f_sidebar->m_time_min = compo_cb.tm.min;
    }
    //计时器
    if (f_sidebar->m_timer_sec != TIMER_SEC) {
        cardbox = compo_getobj_byid(SIDEBAR_CARD_ID_TIMER);
        snprintf(str_buff, sizeof(str_buff), "%02lu:%02lu:%02lu", TIMER_HOUR, TIMER_MIN, TIMER_SEC);    //计时时间
        compo_cardbox_text_set(cardbox, 1, str_buff);
        f_sidebar->m_timer_sec = TIMER_SEC;
    }
    //秒表
    if (f_sidebar->m_stopwatch_sec != STOPWATCH_SEC) {
        cardbox = compo_getobj_byid(SIDEBAR_CARD_ID_STOPWATCH);
        snprintf(str_buff, sizeof(str_buff), "%02lu:%02lu:%02lu", STOPWATCH_HOUR, STOPWATCH_MIN, STOPWATCH_SEC);    //计时时间
        compo_cardbox_text_set(cardbox, 1, str_buff);
        f_sidebar->m_stopwatch_sec = STOPWATCH_SEC;
    }
}

//时钟表盘右滑菜单点击处理
static void func_clock_sub_sidebar_click_handler(void)
{
    compo_cardbox_t *cardbox;
    rect_t rect;
    u8 func_jump = FUNC_NULL;

    point_t pt = ctp_get_sxy();
    u16 id = 0;
    for(u8 i=SIDEBAR_CARD_ID_TIME_WEATHER; i<SIDEBAR_CARD_ID_CNT; i++) {
        if (compo_cardbox_btn_is(compo_getobj_byid(i), pt)) {
            id = i;
        }
    }
    TRACE("click id:%d\n", id);

    switch (id) {
    case SIDEBAR_CARD_ID_TIME_WEATHER:
        if (pt.x > GUI_SCREEN_CENTER_X) {
            func_jump = FUNC_WEATHER;
        }
        break;

    case SIDEBAR_CARD_ID_TIMER:
        func_jump = FUNC_TIMER;
        break;

    case SIDEBAR_CARD_ID_STOPWATCH:
        func_jump = FUNC_STOPWATCH;
        break;

    case SIDEBAR_CARD_ID_CALCULATOR:
        func_jump = FUNC_CALCULATOR;
        break;

    case SIDEBAR_CARD_ID_CALENDAR:
        func_jump = FUNC_CALENDAER;
        break;

    case SIDEBAR_CARD_ID_GAME:
        func_jump = FUNC_GAME;
        break;

    case SIDEBAR_CARD_ID_BREATHE:
        func_jump = FUNC_BREATHE;
        break;

    case SIDEBAR_CARD_ID_LATEST:
        cardbox = compo_getobj_byid(SIDEBAR_CARD_ID_LATEST);
        rect = compo_cardbox_get_icon_absolute(cardbox, 2); //latest0
        if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
            func_jump = sidebar_get_latest_func(0);
        } else {
            rect = compo_cardbox_get_icon_absolute(cardbox, 3); //latest1
            if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
                func_jump = sidebar_get_latest_func(1);
            } else {
                rect = compo_cardbox_get_icon_absolute(cardbox, 4); //latest2
                if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
                    func_jump = sidebar_get_latest_func(2);
                }
            }
        }
        break;

    default:
        break;
    }

    if (func_jump != FUNC_NULL) {
        func_switch_to(func_jump, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);  //切换动画
//        func_cb.sta = btn_info.func_jump;  //直接跳转
    }
}

//时钟表盘右滑菜单主要事件流程处理
static void func_clock_sub_sidebar_process(void)
{
    f_sidebar_t *f_sidebar = (f_sidebar_t *)func_cb.f_cb;

    compo_page_move_process(&f_sidebar->ptm);
    func_clock_sub_sidebar_update();
    func_process();
}

//时钟表盘右滑菜单功能消息处理
static void func_clock_sub_sidebar_message(size_msg_t msg)
{
    f_sidebar_t *f_sidebar = (f_sidebar_t *)func_cb.f_cb;

    switch (msg) {
    case MSG_CTP_TOUCH:
        compo_page_move_touch_handler(&f_sidebar->ptm);
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_LONG:
        break;

    case MSG_CTP_SHORT_LEFT:
        func_switch_to(FUNC_CLOCK, func_get_switching_mode_byidx(sys_cb.nav_index, true));
        break;

    case MSG_CTP_CLICK:
        func_clock_sub_sidebar_click_handler();
        break;

    case MSG_QDEC_FORWARD:
        compo_page_move_set(&f_sidebar->ptm, -QDEC_STEP_Y);
        break;

    case MSG_QDEC_BACKWARD:
        compo_page_move_set(&f_sidebar->ptm, QDEC_STEP_Y);
        break;

    case KU_BACK:
        func_switch_to(FUNC_CLOCK, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);
        break;

    default:
        func_message(msg);
        break;
    }
}

//时钟表盘右滑菜单进入处理
void func_clock_sub_sidebar_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_sidebar_t));
    func_cb.frm_main = func_clock_sub_sidebar_form_create();

    f_sidebar_t *f_sidebar = (f_sidebar_t *)func_cb.f_cb;
    page_move_info_t info = {
        .page_size = SIDEBAR_PAGE_HEIGHT,
        .page_count = 1,
        .quick_jump_perc = 50,
        .up_over_perc = 5,
        .down_over_perc = 5,
    };
    compo_page_move_init(&f_sidebar->ptm, func_cb.frm_main->page_body, &info);
}

//时钟表盘右滑菜单退出处理
void func_clock_sub_sidebar_exit(void)
{

}

//时钟表盘右滑菜单
void func_clock_sub_sidebar(void)
{
    printf("%s\n", __func__);
    func_clock_sub_sidebar_enter();
    while (func_cb.sta == FUNC_SIDEBAR) {
        func_clock_sub_sidebar_process();
        func_clock_sub_sidebar_message(msg_dequeue());
    }
    func_clock_sub_sidebar_exit();
}
