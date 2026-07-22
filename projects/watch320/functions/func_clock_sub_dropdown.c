#include "include.h"
#include "func.h"
#include "func_clock.h"

#define MENU_DROPDOWN_ALPHA             200
#define MENU_DROPDOWN_MODE              0   //1:下拉模糊 0:截图背景模糊+缩放

static uint8_t dropdown_disturb_sw;     //功能未做，加个开关先放着

enum{
    COMPO_ID_BTN_START = FUNC_MAX_NUM, //从任务最大枚举开始，避免和表盘跳转et的d冲突
    //按键
    COMPO_ID_BTN_CONNECT ,      //蓝牙连接开关
    COMPO_ID_BTN_POWER,         //电量开关
    COMPO_ID_BTN_MUTE,          //静音模式开关
    COMPO_ID_BTN_FLASHLIGHT,    //手电筒
    COMPO_ID_BTN_DISCURD,       //勿扰模式开关
    COMPO_ID_BTN_LIGHT,         //亮度调节

    //电池百分比文本
    COMPO_ID_TXT_BATTERY_PERCENT,
    //电池百分比图片
    COMPO_ID_TXT_BATTERY_PIC,
    //蓝牙状态图片
    COMPO_ID_TXT_BLUETOOTH_STA_PIC,
};


#define DROPDOWN_DISP_BTN_ITEM_CNT    ((int)(sizeof(tbl_dropdown_disp_btn_item) / sizeof(tbl_dropdown_disp_btn_item[0])))

typedef struct dropdown_disp_btn_item_t_ {
    u32 res_addr;
    u16 btn_id;
    s16 x;
    s16 y;

} dropdown_disp_btn_item_t;

//类型有显示信息，控制开关，点击跳转任务
//按钮item，创建时遍历一下
static  dropdown_disp_btn_item_t tbl_dropdown_disp_btn_item[] = {
    {UI_BUF_DROPDOWN_BLUETOOTH_OFF_BIN,          COMPO_ID_BTN_CONNECT,          84,  98},     //蓝牙扫描开关
    {UI_BUF_DROPDOWN_POWER_BG_BIN,               COMPO_ID_BTN_POWER,            236, 98},
    {UI_BUF_DROPDOWN_FLASHLIGHT_ON_BIN,          COMPO_ID_BTN_FLASHLIGHT,       84,  200},
    {UI_BUF_DROPDOWN_MUTE_ON_BIN,                COMPO_ID_BTN_MUTE,             236, 200},
    {UI_BUF_DROPDOWN_DISTURB_ON_BIN,             COMPO_ID_BTN_DISCURD,             84,  303},
    {UI_BUF_DROPDOWN_LIGHT_1_BIN,                COMPO_ID_BTN_LIGHT,            236, 303},

};

//下拉电量图标更新
static void func_clock_sub_dropdown_battery_pic_update(void)
{
    compo_picturebox_t *battery_pic = compo_getobj_byid(COMPO_ID_TXT_BATTERY_PIC);
    switch(sys_cb.vbat_percent){
        case 1 ... 16:
            compo_picturebox_set(battery_pic, UI_BUF_DROPDOWN_POWER1_BIN);
            break;
        case 17 ... 32:
            compo_picturebox_set(battery_pic, UI_BUF_DROPDOWN_POWER2_BIN);
            break;
        case 33 ... 48:
            compo_picturebox_set(battery_pic, UI_BUF_DROPDOWN_POWER3_BIN);
            break;
        case 49 ... 64:
            compo_picturebox_set(battery_pic, UI_BUF_DROPDOWN_POWER4_BIN);
            break;
        case 65 ... 80:
            compo_picturebox_set(battery_pic, UI_BUF_DROPDOWN_POWER5_BIN);
            break;
        case 81 ... 100:
            compo_picturebox_set(battery_pic, UI_BUF_DROPDOWN_POWER6_BIN);
            break;
        default:
            compo_picturebox_set(battery_pic, UI_BUF_DROPDOWN_POWER6_BIN);
            break;
    }
}

//下拉蓝牙连接标志更新
static void func_clock_sub_dropdown_bluetooth_pic_update(void)
{
    compo_picturebox_t *bluetooth_pic = compo_getobj_byid(COMPO_ID_TXT_BLUETOOTH_STA_PIC);
    if(bt_is_connected()) {
        compo_picturebox_set(bluetooth_pic, UI_BUF_DROPDOWN_BLUETOOTH_CONNECT_ON_BIN);
    } else{
        compo_picturebox_set(bluetooth_pic, UI_BUF_DROPDOWN_BLUETOOTH_CONNECT_OFF_BIN);
    }
}

//下拉静音图标更新
static void func_clock_sub_dropdown_mute_pic_update(void)
{
    compo_button_t *mute_pic = compo_getobj_byid(COMPO_ID_BTN_MUTE);
    if(sys_cb.mute) {
        compo_button_set_bgimg(mute_pic, UI_BUF_DROPDOWN_MUTE_ON_BIN);
    } else {
        compo_button_set_bgimg(mute_pic, UI_BUF_DROPDOWN_MUTE_OFF_BIN);
    }
}

//下拉蓝牙按钮更新
static void func_clock_sub_dropdown_bluetooth_btn_pic_update(void)
{
    compo_button_t *bluetooth_pic = compo_getobj_byid(COMPO_ID_BTN_CONNECT);
    printf("bt_get_scan: 0x%x\n", bt_get_scan());
    if(bt_get_scan()) {
        compo_button_set_bgimg(bluetooth_pic, UI_BUF_DROPDOWN_BLUETOOTH_ON_BIN);
    } else {
        compo_button_set_bgimg(bluetooth_pic, UI_BUF_DROPDOWN_BLUETOOTH_OFF_BIN);
    }
}

//下拉勿扰按钮更新
static void func_clock_sub_dropdown_disturb_pic_update(void)
{
    compo_button_t *disturb_pic = compo_getobj_byid(COMPO_ID_BTN_DISCURD);
    if(dropdown_disturb_sw) {
        compo_button_set_bgimg(disturb_pic, UI_BUF_DROPDOWN_DISTURB_ON_BIN);
    } else {
        compo_button_set_bgimg(disturb_pic, UI_BUF_DROPDOWN_DISTURB_OFF_BIN);
    }

}


//创建下拉菜单
static void func_clock_sub_dropdown_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

#if GUI_USE_BLUR && MENU_DROPDOWN_MODE
    //新建图像
    compo_picturebox_t *pic = compo_picturebox_create(frm, 0);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_blur_set_ram(pic, cur_scbuf, blur_obuf, 1);
#else
    //创建遮罩层
    compo_shape_t *masklayer = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer, COLOR_BLACK);
    compo_shape_set_location(masklayer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(masklayer, 140);
#endif

    //创建按钮
    static compo_button_t *btn;
    for (u8 idx_btn = 0; idx_btn < DROPDOWN_DISP_BTN_ITEM_CNT; idx_btn++) {
        btn = compo_button_create_by_image(frm, tbl_dropdown_disp_btn_item[idx_btn].res_addr);
        compo_setid(btn, tbl_dropdown_disp_btn_item[idx_btn].btn_id);
        compo_button_set_pos(btn, tbl_dropdown_disp_btn_item[idx_btn].x, tbl_dropdown_disp_btn_item[idx_btn].y);
        compo_button_set_alpha(btn, MENU_DROPDOWN_ALPHA);
    }

    //电池
    compo_picturebox_t *battery_pic = compo_picturebox_create(frm, UI_BUF_DROPDOWN_POWER1_BIN);
    compo_setid(battery_pic, COMPO_ID_TXT_BATTERY_PIC);
    compo_picturebox_set_pos(battery_pic, 260, 30);
    compo_picturebox_set_visible(battery_pic, true);

    //蓝牙状态
    compo_picturebox_t *bluetooth_pic = compo_picturebox_create(frm, UI_BUF_DROPDOWN_BLUETOOTH_CONNECT_OFF_BIN);
    compo_setid(bluetooth_pic, COMPO_ID_TXT_BLUETOOTH_STA_PIC);
    compo_picturebox_set_pos(bluetooth_pic, 60, 30);
    compo_picturebox_set_visible(bluetooth_pic, true);


    //电池电量
    compo_textbox_t *battery_txt = compo_textbox_create(frm, 4);
    compo_textbox_set_location(battery_txt, 200, 28, 0, 0);
    compo_textbox_set_autosize(battery_txt, true);
    compo_bonddata(battery_txt, COMPO_BOND_BATTERY);
    compo_textbox_set_font(battery_txt, UI_BUF_0FONT_FONT_ASC_BIN);
    compo_textbox_set_forecolor(battery_txt, make_color(0xae, 0xb3, 0xbc));     //和电池底图颜色一致

    func_clock_sub_dropdown_battery_pic_update();
    func_clock_sub_dropdown_bluetooth_pic_update();     //蓝牙更新
    func_clock_sub_dropdown_bluetooth_btn_pic_update();
    func_clock_sub_dropdown_mute_pic_update();          //静音更新
    func_clock_sub_dropdown_disturb_pic_update();       //勿扰

    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk->sub_frm = frm;

#if GUI_USE_BLUR && MENU_DROPDOWN_MODE
    f_clk->blur_pic = pic;
#else
    f_clk->masklayer = masklayer;
#endif
}

//时钟表盘主要事件流程处理
static void func_clock_sub_dropdown_process(void)
{
    func_clock_sub_process();
}

static void func_clock_sub_dropdown_click_handler(void)
{
    int id = compo_get_button_id();
    printf("id: %d\n", id);
    switch(id) {
    case COMPO_ID_BTN_CONNECT:
        if(bt_get_scan()) {
            if (bt_is_connected()) {
                bt_disconnect(0);
            }
            bt_scan_disable();
        }else {
            bt_scan_enable();
            if (app_phone_type_get()) {
                if (!ble_is_connect()) {
                    if (bt_nor_get_link_info(NULL)) {  //如果存在配对信息
                        bt_connect();
                    }
                } else {
                    app_once_connect(true);
                }
            }
        }
        printf("bt_get_scan: %d\n", bt_get_scan());
        func_clock_sub_dropdown_bluetooth_btn_pic_update();
        break;

    case COMPO_ID_BTN_DISCURD:
        dropdown_disturb_sw = !dropdown_disturb_sw;
        func_clock_sub_dropdown_disturb_pic_update();
        break;

    case COMPO_ID_BTN_POWER:
        func_cb.sta = FUNC_CHARGE;
        break;

    case COMPO_ID_BTN_MUTE:
        if(sys_cb.mute) {
            bsp_sys_unmute();
        }else {
            bsp_sys_mute();
        }
        func_clock_sub_dropdown_mute_pic_update();          //静音更新
        break;

    //点击任务跳转
    case COMPO_ID_BTN_FLASHLIGHT:
        func_cb.sta = FUNC_FLASHLIGHT;
        break;

    case COMPO_ID_BTN_LIGHT:
        func_cb.sta = FUNC_LIGHT;
        break;

    default:
        break;
    }
}

//时钟表盘下拉菜单功能消息处理
static void func_clock_sub_dropdown_message(size_msg_t msg)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_CLICK:
        func_clock_sub_dropdown_click_handler();
        break;
    case MSG_CTP_SHORT_LEFT:
        break;
    case MSG_CTP_SHORT_RIGHT:
        break;
    case MSG_CTP_SHORT_UP:
        {
            u16 sw_mode = FUNC_SWITCH_MENU_DROPDOWN_UP;
            if (f_clk->blur_pic) {
                if (f_clk->masklayer) {
                    sw_mode |= FUNC_SWITCH_DOWN_BG_BLUR;
                } else {
                    sw_mode |= FUNC_SWITCH_DOWN_BLUR;
                }
            }

            if (func_switching(sw_mode, NULL)) {
                f_clk->sta = FUNC_CLOCK_MAIN;                   //上滑返回到时钟主界面
            }
        }
        break;

    case KU_BACK:
        func_switching(FUNC_SWITCH_MENU_DROPDOWN_UP | FUNC_SWITCH_AUTO, NULL);
        f_clk->sta = FUNC_CLOCK_MAIN;                       //单击BACK键返回到时钟主界面
        break;
    case MSG_SYS_1S:
        func_clock_sub_dropdown_battery_pic_update();       //电量更新
        func_clock_sub_dropdown_bluetooth_pic_update();     //蓝牙更新
        func_clock_sub_dropdown_mute_pic_update();          //静音更新
        break;
    case MSG_QDEC_BACKWARD:
        printf("MSG_QDEC_BACKWARD\n");
        break;

    case MSG_QDEC_FORWARD:
        printf("MSG_QDEC_FORWARD\n");
        break;
    default:
        func_clock_sub_message(msg);
        break;
    }
}


compo_form_t *func_clock_form_create(void);
compo_form_t *func_clock_form_create_by_screenshoot(void);
void func_switch_screenshot(void *cur_scbuf, void *next_scbuf, u8 next_sta);

//时钟表盘下拉菜单进入处理
static void func_clock_sub_dropdown_enter(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    func_clock_butterfly_set_light_visible(false);

#if GUI_USE_BLUR
    func_switch_screenshot(cur_scbuf, NULL, 0);
    compo_form_destroy(func_cb.frm_main);
    func_cb.frm_main = func_clock_form_create_by_screenshoot();
#endif

    func_clock_sub_dropdown_form_create();
    u16 sw_mode = FUNC_SWITCH_MENU_DROPDOWN_DOWN;
    if (f_clk->blur_pic) {
        if (f_clk->masklayer) {
            sw_mode |= FUNC_SWITCH_DOWN_BG_BLUR;
        } else {
            sw_mode |= FUNC_SWITCH_DOWN_BLUR;
        }
    }

    if (!func_switching(sw_mode, NULL)) {
        return;                                             //下拉到一半取消
    }

    f_clk->sta = FUNC_CLOCK_SUB_DROPDOWN;                   //进入到下拉菜单
}

//时钟表盘下拉菜单退出处理
/* static  */void func_clock_sub_dropdown_exit(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    compo_form_destroy(f_clk->sub_frm);
    func_clock_butterfly_set_light_visible(true);
    f_clk->sub_frm = NULL;

#if GUI_USE_BLUR
    compo_form_destroy(func_cb.frm_main);
    func_cb.frm_main = func_clock_form_create();
#endif
}

//时钟表盘下拉菜单
void func_clock_sub_dropdown(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN
    func_clock_sub_dropdown_enter();
    while (func_cb.sta == FUNC_CLOCK && ((f_clock_t *)func_cb.f_cb)->sta == FUNC_CLOCK_SUB_DROPDOWN) {
        func_clock_sub_dropdown_process();
        func_clock_sub_dropdown_message(msg_dequeue());
    }
    func_clock_sub_dropdown_exit();
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}
