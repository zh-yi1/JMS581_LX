#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_about_t_ {
    u8 value;
} f_about_t;

enum {
    //数字
    COMPO_ID_NUM_DISP_4G_OFF = 1,
    COMPO_ID_NUM_DISP_4G_ON,

    //按钮
    COMPO_ID_BIN_4G,
    //图片
    COMPO_ID_PIC_4G_ON,
    COMPO_ID_PIC_4G_OFF,
};

typedef struct wrist_disp_pic_item_t_ {
    u32 res_addr;
    u16 pic_id;
    s16 x;
    s16 y;
    bool visible_en;
} wrist_disp_pic_item_t;

#define WRIST_DISP_PIC_ITEM_CNT             ((int)(sizeof(tbl_wrist_disp_pic_item) / sizeof(tbl_wrist_disp_pic_item[0])))

//图片item，创建时遍历一下
static const wrist_disp_pic_item_t tbl_wrist_disp_pic_item[] = {
    {UI_BUF_COMMON_ON_BIN,           COMPO_ID_PIC_4G_ON,         264,    212,    false},
    {UI_BUF_COMMON_OFF_BIN,          COMPO_ID_PIC_4G_OFF,        264,    212,    true},
};

//关于页面
compo_form_t *func_set_sub_4g_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_SETTING_4G]);

    compo_textbox_t *txt_shake = compo_textbox_create(frm, 4);
    compo_textbox_set_align_center(txt_shake, false);
    compo_textbox_set_pos(txt_shake, 30, 194);
    compo_textbox_set(txt_shake, "4G开关");

    //新建图像
    compo_picturebox_t *pic_click;
    for (u8 idx = 0; idx < WRIST_DISP_PIC_ITEM_CNT; idx++) {
        pic_click = compo_picturebox_create(frm, tbl_wrist_disp_pic_item[idx].res_addr);
        compo_setid(pic_click, tbl_wrist_disp_pic_item[idx].pic_id);
        compo_picturebox_set_pos(pic_click, tbl_wrist_disp_pic_item[idx].x, tbl_wrist_disp_pic_item[idx].y);
        compo_picturebox_set_visible(pic_click, tbl_wrist_disp_pic_item[idx].visible_en);
    }

    //创建按钮
    compo_button_t *btn;
    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BIN_4G);
    compo_button_set_location(btn, 264, 212, 80, 40);

    return frm;
}

//更新显示界面
static void func_set_sub_4g_disp(void)
{
    f_about_t *about = (f_about_t *)func_cb.f_cb;

    //获取图片组件的地址
    compo_picturebox_t *pic_wrs_on  = compo_getobj_byid(COMPO_ID_PIC_4G_ON);
    compo_picturebox_t *pic_wrs_off = compo_getobj_byid(COMPO_ID_PIC_4G_OFF);

    if (about->value == COMPO_ID_NUM_DISP_4G_ON) {
        compo_picturebox_set_visible(pic_wrs_on, true);
        compo_picturebox_set_visible(pic_wrs_off, false);
    } else if (about->value == COMPO_ID_NUM_DISP_4G_OFF) {
        compo_picturebox_set_visible(pic_wrs_on, false);
        compo_picturebox_set_visible(pic_wrs_off, true);
    }
}

//单击按钮
static void func_wrist_button_click(void)
{
    f_about_t *about = (f_about_t *)func_cb.f_cb;
    int id = compo_get_button_id();

    switch(id) {
    case COMPO_ID_BIN_4G:
        if (about->value == COMPO_ID_NUM_DISP_4G_OFF) {
            printf("4g on\n");
            about->value = COMPO_ID_NUM_DISP_4G_ON;
            bsp_modem_init();
        } else {
            printf("4g off\n");
            about->value = COMPO_ID_NUM_DISP_4G_OFF;
            bsp_modem_exit();
        }
    break;

    default:
    break;
    }
    func_set_sub_4g_disp();
}

//关于功能事件处理
static void func_set_sub_4g_process(void)
{
    func_process();

    // f_about_t *about = (f_about_t *)func_cb.f_cb;
}

//关于功能消息处理
static void func_set_sub_4g_message(size_msg_t msg)
{
    // f_about_t *about = (f_about_t *)func_cb.f_cb;
    switch (msg) {

    case MSG_CTP_CLICK:
        func_wrist_button_click();
        break;

    case KU_DELAY_BACK:
        break;

    default:
		func_message(msg);
        break;
    }
}

//进入关于功能
void func_set_sub_4g_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_about_t));
    func_cb.frm_main = func_set_sub_4g_form_create();

    f_about_t *about = (f_about_t *)func_cb.f_cb;
    printf("4g sta: %d\n", bsp_modem_get_init_flag());
    if (bsp_modem_get_init_flag()) {
        about->value = COMPO_ID_NUM_DISP_4G_ON;
    } else {
        about->value = COMPO_ID_NUM_DISP_4G_OFF;
    }
    func_set_sub_4g_disp();
}

//退出关于功能
void func_set_sub_4g_exit(void)
{
    func_cb.last = FUNC_SET_SUB_4G;
}

//关于功能
void func_set_sub_4g(void)
{
    printf("%s\n", __func__);
    func_set_sub_4g_enter();
    while (func_cb.sta == FUNC_SET_SUB_4G) {
        func_set_sub_4g_process();
        func_set_sub_4g_message(msg_dequeue());
    }
    func_set_sub_4g_exit();
}
