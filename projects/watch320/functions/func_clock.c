#include "include.h"
#include "func.h"
#include "func_clock.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define DIALPLATE_NUM               (sizeof(dialplate_info) / sizeof(u32))
#define DIALPLATE_BTF_IDX           DIALPLATE_NUM - 1        //蝴蝶表盘默认最后一个
#define DIALPLATE_CUBE_IDX          DIALPLATE_NUM - 2        //立方体表盘默认倒数第二个
#define DIALPLATE_WINDMILL_IDX      DIALPLATE_NUM - 3        //风车表盘默认倒数第三个
#define DIALPLATE_FISH_IDX          DIALPLATE_NUM - 4        //鱼表盘默认倒数第四个
#define DIALPLATE_HOURGLASS_IDX     DIALPLATE_NUM - 5        //沙漏表盘默认倒数第四个
#define DIALPLATE_COMPASS_IDX       DIALPLATE_NUM - 6        //指南针表盘默认倒数第六个
#define DIALPLATE_TIMEMAP_IDX       DIALPLATE_NUM - 8

// UI精简：dialplate 源 bin 已删，占位避免 0 长度；实际表盘改用字体最小窗体
const u32 dialplate_info[] = {
    0,
//    UI_BUF_DIALPLATE_1_BIN,
//    UI_BUF_DIALPLATE_8_BIN,                //精准时图
//#if AVI_DIALPLATE_EN && VIDEO_PLAY_EN
//    UI_BUF_DIALPLATE_AVI_2_BIN,
//#endif // AVI_DIALPLATE_EN
//    UI_BUF_DIALPLATE_COMPASS_BIN,         //指南针
//    UI_BUF_DIALPLATE_HOURGLASS_BIN,       //沙漏
//    UI_BUF_DIALPLATE_FISH_BIN,            //鱼
//    UI_BUF_DIALPLATE_WINDMILL_BIN,        //风车
//    UI_BUF_DIALPLATE_CUBE_BIN,
//    UI_BUF_DIALPLATE_BTF_BIN,
};

//表盘快捷按钮编号表（UI精简：业务页资源已删，全部禁用）
const u8 quick_btn_tbl[] =
{
    FUNC_NULL,
    FUNC_NULL, // FUNC_HEARTRATE,
    FUNC_NULL, // FUNC_BT,
    FUNC_NULL, // FUNC_ALARM_CLOCK,
    FUNC_NULL, // FUNC_BLOOD_OXYGEN,
    FUNC_NULL, // FUNC_BLOODSUGAR,
    FUNC_NULL, // FUNC_BLOOD_PRESSURE,
    FUNC_NULL, // FUNC_BREATHE,
    FUNC_NULL, // FUNC_CALCULATOR,

    FUNC_NULL, // FUNC_CAMERA,
    FUNC_NULL, // FUNC_TIMER,
    FUNC_NULL, // FUNC_SLEEP,
    FUNC_NULL, // FUNC_STOPWATCH,
    FUNC_NULL, // FUNC_WEATHER,
    FUNC_NULL, // FUNC_GAME,
    FUNC_NULL, // FUNC_STYLE,
    FUNC_NULL, // FUNC_ALTITUDE,
    FUNC_NULL, // FUNC_MAP,
    FUNC_NULL, // FUNC_MESSAGE,
    FUNC_NULL, // FUNC_SCAN,
    FUNC_NULL, // FUNC_VOICE,
#if SECURITY_PAY_EN
    FUNC_NULL, // FUNC_ALIPAY,
#else
    FUNC_NULL,
#endif // SECURITY_PAY_EN
    FUNC_NULL, // FUNC_COMPASS,
    FUNC_NULL, // FUNC_ADDRESS_BOOK,
    FUNC_NULL, // FUNC_SPORT,
    FUNC_NULL, // FUNC_CALL,
    FUNC_NULL, // FUNC_FINDPHONE,
    FUNC_NULL, // FUNC_CALENDAER,
    FUNC_NULL, // FUNC_ACTIVITY,
    FUNC_NULL, // FUNC_FLASHLIGHT,
    FUNC_NULL, // FUNC_SETTING,
};

int compo_get_animation_id(void);
void compo_animation_manual_next(compo_animation_t *animation);
void func_switch_to(u8 sta, u16 switch_mode);

static bool func_clock_time_map_swithing(void) { return false; }

u8 func_clock_get_max_dialplate_num(void)
{
    return (sizeof(dialplate_info) / sizeof(u32));
}

u32 func_clock_get_dialplate_info(u8 index)
{
    return dialplate_info[index];
}

u32 func_clock_get_dialplate_cube_idx(void)
{
    return DIALPLATE_CUBE_IDX;
}

u32 func_clock_get_dialplate_butterfly_idx(void)
{
    return DIALPLATE_BTF_IDX;
}

compo_form_t *func_clock_form_create(void)
{
    // UI精简：不再加载 dialplate 资源，仅用 0font 显示时分
    /*
    if (sys_cb.dialplate_index == DIALPLATE_BTF_IDX) {
        return func_clock_butterfly_form_create();
    } else if (sys_cb.dialplate_index == DIALPLATE_CUBE_IDX) {
        return func_clock_cube_form_create();
    } else if (sys_cb.dialplate_index == DIALPLATE_WINDMILL_IDX) {
        return func_clock_windmill_form_create();
    } else if (sys_cb.dialplate_index == DIALPLATE_FISH_IDX) {
        return func_clock_fish_form_create();
    } else if (sys_cb.dialplate_index == DIALPLATE_HOURGLASS_IDX) {
        return func_clock_hourglass_form_create();
    } else if (sys_cb.dialplate_index == DIALPLATE_COMPASS_IDX) {
        return func_clock_compass_form_create();
    } else {
        u32 base_addr = dialplate_info[sys_cb.dialplate_index];
        u16 compo_num = bsp_uitool_header_phrase(base_addr);
        if (!compo_num) {
            halt(HALT_GUI_DIALPLATE_HEAD);
        }
        compo_form_t *frm = compo_form_create(true);
        bsp_uitool_create(frm, base_addr, compo_num);
        return frm;
    }
    */

    compo_form_t *frm = compo_form_create(true);
    compo_textbox_t *txt;

    /* 窗体默认全屏 icon(res=0)：若不设背景，GPU 可能把 UI 头(字库)当图解码 → 花屏 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 时:分 — 用 ASC 字库，避免数字点阵字库与 textbox 绑定异常 */
    txt = compo_textbox_create(frm, 2);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X - 55, GUI_SCREEN_CENTER_Y - 10, 90, 60);
    compo_bonddata(txt, COMPO_BOND_HOUR);

    txt = compo_textbox_create(frm, 1);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 10, 30, 60);
    compo_textbox_set(txt, ":");

    txt = compo_textbox_create(frm, 2);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X + 55, GUI_SCREEN_CENTER_Y - 10, 90, 60);
    compo_bonddata(txt, COMPO_BOND_MINUTE);

    txt = compo_textbox_create(frm, 10);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 50, 200, 40);
    compo_bonddata(txt, COMPO_BOND_DATE);

    return frm;
}

#if GUI_USE_BLUR
//截图
compo_form_t *func_clock_form_create_by_screenshoot(void)
{
    compo_form_t *frm = compo_form_create(true);
        //新建图像
    compo_picturebox_t *pic = compo_picturebox_create(frm, 0);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    gui_set_ram_check(cur_scbuf, __func__);      //检测ram对不对
    compo_picturebox_set_ram(pic, cur_scbuf);
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk->blur_pic = pic;

    return frm;
}
#endif

//单击按钮
static void func_clock_button_click(void)
{
    u16 btn_id = compo_get_button_id();
    u16 animation_id = compo_get_animation_id();
    static bool flag_time_map = false;
    if (btn_id) {
        if (btn_id == 31) {
            flag_time_map = func_clock_time_map_swithing();
        }else if (!flag_time_map){
            func_switch_to(quick_btn_tbl[btn_id], func_get_switching_mode_byidx(sys_cb.nav_index, false) | FUNC_SWITCH_AUTO);
        }
    } else if (animation_id) {
        compo_animation_t *animation = compo_getobj_byid(animation_id);
        if (animation->bond_data == COMPO_BOND_IMAGE_CLICK) {
            compo_animation_manual_next(animation);
        } else if (animation->bond_data == COMPO_BOND_ANIMATION_AREA_CLICK) {
            compo_animation_click_set_vis(animation);
        }
    }

}


//子功能公共事件处理
void func_clock_sub_process(void)
{
    func_process();                                     //刷新UI
}

//子功能公共消息处理
void func_clock_sub_message(size_msg_t msg)
{
    func_message(msg);
}

//时钟表盘功能事件处理
static void func_clock_process(void)
{
    // UI精简：禁用特效表盘 process
    /*
    if (sys_cb.dialplate_index == DIALPLATE_CUBE_IDX) {
        func_clock_cube_process();
    } else if (sys_cb.dialplate_index == DIALPLATE_BTF_IDX) {
        func_clock_butterfly_process();
    } else if (sys_cb.dialplate_index == DIALPLATE_WINDMILL_IDX) {
        func_clock_windmill_process();
    } else if (sys_cb.dialplate_index == DIALPLATE_FISH_IDX) {
        func_clock_fish_process();
    } else if (sys_cb.dialplate_index == DIALPLATE_HOURGLASS_IDX) {
        func_clock_hourglass_process();
    } else if (sys_cb.dialplate_index == DIALPLATE_COMPASS_IDX) {
        func_clock_compass_process();
    }
    */

    func_process();                                  //刷新UI

#if VIDEO_PLAY_EN
    // UI精简：无视频表盘资源
#endif // VIDEO_PLAY_EN
}

static void func_clock_message_nomal(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_SHORT_UP:
        // UI精简：card 依赖已删资源，禁用上拉
        // func_clock_butterfly_set_light_visible(false);
        // func_switch_to(FUNC_CARD, FUNC_SWITCH_MENU_PULLUP_UP | FUNC_SWITCH_DOWN_BG_BLUR);
        // if (func_cb.sta == FUNC_CLOCK) {
        //     func_clock_butterfly_set_light_visible(true);
        // }
        break;

    case MSG_CTP_SHORT_RIGHT:
        // UI精简：sidebar 依赖已删资源，禁用右滑
        // func_switch_to(FUNC_SIDEBAR, func_get_switching_mode_byidx(sys_cb.nav_index, false));
        break;

    case MSG_CTP_SHORT_DOWN:
        // UI精简：dropdown 依赖已删资源，禁用下拉
        // printf("MSG_CTP_SHORT_DOWN\n");
        // func_clock_sub_dropdown();
        break;

    case MSG_CTP_CLICK:
        func_clock_button_click();
        break;

    case MSG_CTP_LONG:
        // UI精简：表盘预览依赖 dialplate，禁用
        // if (func_clock_preview_get_type() == PREVIEW_ROTARY_STYLE) {
        //     func_cb.sta = FUNC_CLOCK_PREVIEW;
        // } else {
        //     func_switch_to(FUNC_CLOCK_PREVIEW, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);
        // }
        break;

    default:
        func_message(msg);
        break;
    }
}

//时钟表盘功能消息处理
static void func_clock_message(size_msg_t msg)
{
    // UI精简：统一走最小表盘消息，禁用特效表盘分支
    /*
    switch (sys_cb.dialplate_index) {
    case DIALPLATE_CUBE_IDX:
        func_clock_cube_message(msg);
        break;
    case DIALPLATE_BTF_IDX:
        func_clock_butterfly_message(msg);
        break;
    case DIALPLATE_WINDMILL_IDX:
        func_clock_windmill_message(msg);
        break;
    case DIALPLATE_FISH_IDX:
        func_clock_fish_message(msg);
        break;
    case DIALPLATE_HOURGLASS_IDX:
        func_clock_hourglass_message(msg);
        break;
    case DIALPLATE_COMPASS_IDX:
        func_clock_compass_message(msg);
        break;
    default:
        func_clock_message_nomal(msg);
        break;
    }
    */
    func_clock_message_nomal(msg);
}


//进入时钟表盘功能
void func_clock_enter(void)
{
    /* 表盘 bin 已删，强制索引 0，避免 NVRAM 旧值触发其它逻辑 */
    sys_cb.dialplate_index = 0;

    func_cb.f_cb = func_zalloc(sizeof(f_clock_t));
    func_cb.frm_main = func_clock_form_create();
    // UI精简：禁用指南针/time_map 表盘初始化
    /*
    if (sys_cb.dialplate_index == DIALPLATE_COMPASS_IDX) {
        func_clock_compass_init();
    } else if (sys_cb.dialplate_index == DIALPLATE_TIMEMAP_IDX) {
        func_clock_time_map_init();
    }
    */

#if VIDEO_PLAY_EN
    // UI精简：无视频表盘
#endif // VIDEO_PLAY_EN

}

//退出时钟表盘功能
void func_clock_exit(void)
{
    // UI精简：禁用特效表盘退出清理
    /*
    if (sys_cb.dialplate_index == DIALPLATE_BTF_IDX || sys_cb.dialplate_index == DIALPLATE_TIMEMAP_IDX) {
        tft_set_temode(DEFAULT_TE_MODE);
    } else if(sys_cb.dialplate_index == DIALPLATE_WINDMILL_IDX) {
        func_clock_windmill_pbubbles_destory();
    }
    */

    //tft_set_baud(3, 4);
    func_cb.last = FUNC_CLOCK;

#if VIDEO_PLAY_EN
    // UI精简：无视频表盘
#endif // VIDEO_PLAY_EN

}

//时钟表盘功能
void func_clock(void)
{
    printf("%s\n", __func__);
//sys_cb.dialplate_index = DIALPLATE_CUBE_IDX;
    func_clock_enter();
    while (func_cb.sta == FUNC_CLOCK) {
        func_clock_process();
        func_clock_message(msg_dequeue());
    }
    func_clock_exit();
}
