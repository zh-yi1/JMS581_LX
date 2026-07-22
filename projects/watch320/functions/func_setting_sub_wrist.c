#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_wrist_t_ {
    u8 value;
} f_wrist_t;

enum {
    //数字
    COMPO_ID_NUM_DISP_ONE = 1,
    COMPO_ID_NUM_DISP_TWS,

    //按钮
    COMPO_ID_BIN_WRIST,
    //图片
    COMPO_ID_PIC_WRIST_ON,
    COMPO_ID_PIC_WRIST_OFF,
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
    {UI_BUF_COMMON_ON_BIN,           COMPO_ID_PIC_WRIST_ON,         264,    212,    false},
    {UI_BUF_COMMON_OFF_BIN,          COMPO_ID_PIC_WRIST_OFF,        264,    212,    true},
};

//抬腕亮屏页面
compo_form_t *func_set_sub_wrist_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_SETTING_UP]);

    compo_textbox_t *txt_shake = compo_textbox_create(frm, 4);
    compo_textbox_set_align_center(txt_shake, false);
    compo_textbox_set_pos(txt_shake, 30, 194);
    compo_textbox_set(txt_shake, i18n[STR_SETTING_UP]);

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
    compo_setid(btn, COMPO_ID_BIN_WRIST);
    compo_button_set_location(btn, 264, 212, 80, 40);

    return frm;
}

//抬腕亮屏事件处理
static void func_set_sub_wrist_process(void)
{
    func_process();
}

//更新显示界面
static void func_set_sub_wrist_disp(void)
{
    f_wrist_t *wrs = (f_wrist_t *)func_cb.f_cb;

    //获取图片组件的地址
    compo_picturebox_t *pic_wrs_on  = compo_getobj_byid(COMPO_ID_PIC_WRIST_ON);
    compo_picturebox_t *pic_wrs_off = compo_getobj_byid(COMPO_ID_PIC_WRIST_OFF);

    if (wrs->value == COMPO_ID_NUM_DISP_TWS) {
        compo_picturebox_set_visible(pic_wrs_on, true);
        compo_picturebox_set_visible(pic_wrs_off, false);
    } else if (wrs->value == COMPO_ID_NUM_DISP_ONE) {
        compo_picturebox_set_visible(pic_wrs_on, false);
        compo_picturebox_set_visible(pic_wrs_off, true);
    }
}

//单击按钮
static void func_wrist_button_click(void)
{
    u8 ret = 0;
    f_wrist_t *wrs = (f_wrist_t *)func_cb.f_cb;
    int id = compo_get_button_id();

    switch(id) {
    case COMPO_ID_BIN_WRIST:
        ret = msgbox((char *)i18n[STR_SETTING_UP], NULL , COMPO_ID_NUM_DISP_ONE, MSGBOX_MSG_TYPE_NONE);

        if (ret == COMPO_ID_NUM_DISP_ONE) {
            if (wrs->value == COMPO_ID_NUM_DISP_ONE) {
                wrs->value = COMPO_ID_NUM_DISP_TWS;
            } else {
                wrs->value = COMPO_ID_NUM_DISP_ONE;
            }
        }
    break;

    default:
    break;
    }
    func_set_sub_wrist_disp();
}

//抬腕亮屏功能消息处理
static void func_set_sub_wrist_message(size_msg_t msg)
{
    switch (msg) {

    case MSG_CTP_CLICK:
        func_wrist_button_click();
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入抬腕亮屏功能
void func_set_sub_wrist_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_wrist_t));
    func_cb.frm_main = func_set_sub_wrist_form_create();

    //初始化变量
    f_wrist_t *wrs = (f_wrist_t *)func_cb.f_cb;
    wrs->value = COMPO_ID_NUM_DISP_ONE;
    wrs->value = COMPO_ID_NUM_DISP_ONE;
}

//退出抬腕亮屏功能
void func_set_sub_wrist_exit(void)
{
    func_cb.last = FUNC_SET_SUB_WRIST;
}

//抬腕亮屏功能
void func_set_sub_wrist(void)
{
    printf("%s\n", __func__);
    func_set_sub_wrist_enter();
    while (func_cb.sta == FUNC_SET_SUB_WRIST) {
        func_set_sub_wrist_process();
        func_set_sub_wrist_message(msg_dequeue());
    }
    func_set_sub_wrist_exit();
}
