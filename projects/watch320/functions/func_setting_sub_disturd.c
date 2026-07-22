#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_disturd_t_ {
    bool flag_drag;                 //开始拖动
    s32 x_pos;
    s32 y_pos;
    s32 x;
    s32 y;
} f_disturd_t;

enum{
    //数字
    COMPO_ID_NUM_DISP_ONE = 1,
    COMPO_ID_NUM_DISP_ZERO,
    COMPO_ID_NUM_DISP_TWS,
    COMPO_ID_NUM_DISP_THR,
    //按钮
    COMPO_ID_BIN_TIMING_ON,
    COMPO_ID_BIN_ALLDAY_ON,
    COMPO_ID_BIN_START,
    COMPO_ID_BIN_END,
    //图片
    COMPO_ID_PIC_ADL_ON,
    COMPO_ID_PIC_TIM_ON,
    COMPO_ID_PIC_ADL_OFF,
    COMPO_ID_PIC_TIM_OFF,
    //文字
    COMPO_ID_TXT_ALL,
    COMPO_ID_TXT_TIM,
    COMPO_ID_TXT_START,
    COMPO_ID_TXT_END,
    COMPO_ID_TXT_PRCOLON,
    COMPO_ID_TXT_LASTCOLON,
    //图形
    COMPO_ID_MSK_ONE,
    COMPO_ID_MSK_TWS,
};

typedef struct disturd_disp_pic_item_t_ {
    u32 res_addr;
    u16 pic_id;
    s16 x;
    s16 y;
    bool visible_en;
} disturd_disp_pic_item_t;

#define DISTURD_DISP_PIC_ITEM_CNT                       ((int)(sizeof(tbl_disturd_disp_pic_item) / sizeof(tbl_disturd_disp_pic_item[0])))

//图片item，创建时遍历一下
static const disturd_disp_pic_item_t tbl_disturd_disp_pic_item[] = {
    {UI_BUF_COMMON_ON_BIN,           COMPO_ID_PIC_ADL_ON,         264,    110,    false},
    {UI_BUF_COMMON_ON_BIN,           COMPO_ID_PIC_TIM_ON,         264,    200,    false},
    {UI_BUF_COMMON_OFF_BIN,          COMPO_ID_PIC_ADL_OFF,        264,    110,    false},
    {UI_BUF_COMMON_OFF_BIN,          COMPO_ID_PIC_TIM_OFF,        264,    200,    false},
};

#define DISURD_DISP_BTN_ITEM_CNT    ((int)(sizeof(tbl_disturd_disp_btn_item) / sizeof(tbl_disturd_disp_btn_item[0])))

typedef struct disturd_disp_btn_item_t_ {
    u16 btn_id;
    s16 x;
    s16 y;
    s16 h;
    s16 l;
} disturd_disp_btn_item_t;

//按钮item，创建时遍历一下
static const disturd_disp_btn_item_t tbl_disturd_disp_btn_item[] = {
    {COMPO_ID_BIN_ALLDAY_ON,        160,     105},
    {COMPO_ID_BIN_TIMING_ON,        160,     195},
    {COMPO_ID_BIN_START,            160,     280,   320,    40},
    {COMPO_ID_BIN_END,              160,     340,   320,    40},
};

typedef struct disturd_num_item_t_ {
    u32 res_addr;
    int num_cnt;
    u16 num_id;
    int val;
    s16 x;
    s16 y;
    bool visible_en;
} disturd_num_item_t;

#define DISTURD_NUM_ITEM_CNT                       ((int)(sizeof(tbl_disturd_num_item) / sizeof(tbl_disturd_num_item[0])))

//搞个数字item，创建时遍历一下
static const disturd_num_item_t tbl_disturd_num_item[] = {
    /*   res_addr,                           num_cnt,        num_id,                val,   x,     y,   visible_en*/
    {UI_BUF_0FONT_FONT_NUM_24_BIN,            2,     COMPO_ID_NUM_DISP_ZERO,         0,   200,    275,   false},
    {UI_BUF_0FONT_FONT_NUM_24_BIN,            2,     COMPO_ID_NUM_DISP_ONE,          0,   260,    275,   false},
    {UI_BUF_0FONT_FONT_NUM_24_BIN,            2,     COMPO_ID_NUM_DISP_TWS,          0,   200,    345,   false},
    {UI_BUF_0FONT_FONT_NUM_24_BIN,            2,     COMPO_ID_NUM_DISP_THR,          0,   260,    345,   false},
};

#define DISURD_DISP_TXT_ITEM_CNT    ((int)(sizeof(disturd_disp_txt_item) / sizeof(disturd_disp_txt_item[0])))

typedef struct disturd_disp_txt_item_t_ {
    u16 btn_id;
    s16 x;
    s16 y;
} disturd_disp_txt_item_t;

//文字item，创建时遍历一下
static const disturd_disp_txt_item_t disturd_disp_txt_item[] = {
    {COMPO_ID_TXT_ALL,        30,   95},
    {COMPO_ID_TXT_TIM,        30,   185},
    {COMPO_ID_TXT_START,      30,   260},
    {COMPO_ID_TXT_END,        30,   330},
    {COMPO_ID_TXT_PRCOLON,    230,  260},
    {COMPO_ID_TXT_LASTCOLON,  230,  330},
};

//勿扰模式页面
compo_form_t *func_set_sub_disturd_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_SETTING_DISTURD]);

    //创建图像框
    compo_shape_t *masklayer = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer, COLOR_DIMGRAY);
    compo_setid(masklayer, COMPO_ID_MSK_ONE);
    compo_shape_set_location(masklayer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 80, 280, 40);
    compo_shape_set_alpha(masklayer, 0);

    compo_shape_t *masklayer_tws = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer_tws, COLOR_DIMGRAY);
    compo_setid(masklayer_tws, COMPO_ID_MSK_TWS);
    compo_shape_set_location(masklayer_tws, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 150, 280, 40);
    compo_shape_set_alpha(masklayer_tws, 0);

    //创建按钮
    compo_button_t *btn;
    for (u8 idx_btn = 0; idx_btn < DISURD_DISP_BTN_ITEM_CNT; idx_btn++) {
        if (idx_btn < 2) {
            btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BG_BIN);
            compo_setid(btn, tbl_disturd_disp_btn_item[idx_btn].btn_id);
            compo_button_set_pos(btn, tbl_disturd_disp_btn_item[idx_btn].x, tbl_disturd_disp_btn_item[idx_btn].y);
        } else {
            btn = compo_button_create(frm);
            compo_setid(btn, tbl_disturd_disp_btn_item[idx_btn].btn_id);
            compo_button_set_location(btn, tbl_disturd_disp_btn_item[idx_btn].x, tbl_disturd_disp_btn_item[idx_btn].y, tbl_disturd_disp_btn_item[idx_btn].h, tbl_disturd_disp_btn_item[idx_btn].l);

        }
    }

    //创建文本
    compo_textbox_t *textbox[DISURD_DISP_TXT_ITEM_CNT];
    for (int i=0;i<DISURD_DISP_TXT_ITEM_CNT;i++) {
        textbox[i] = compo_textbox_create(frm, 4);
        compo_setid(textbox[i], COMPO_ID_TXT_ALL + i);
        compo_textbox_set_pos(textbox[i], disturd_disp_txt_item[i].x, disturd_disp_txt_item[i].y);
        compo_textbox_set_align_center(textbox[i], false);

        if(i == 0) {
            compo_textbox_set(textbox[i], i18n[STR_DISTURD_ALL]);
        } else if(i == 1) {
            compo_textbox_set(textbox[i], i18n[STR_DISTURD_TIM]);
        } else if(i == 2) {
            compo_textbox_set_forecolor(textbox[i], COLOR_GREEN);
            compo_textbox_set(textbox[i], i18n[STR_DISTURD_TIM_START]);
            compo_textbox_set_visible(textbox[i], false);
        } else if(i == 3) {
            compo_textbox_set_forecolor(textbox[i], COLOR_GREEN);
            compo_textbox_set(textbox[i], i18n[STR_DISTURD_TIM_END]);
            compo_textbox_set_visible(textbox[i], false);
        } else {
            compo_textbox_set(textbox[i], ":");
            compo_textbox_set_visible(textbox[i], false);
        }
    }

    //新建图像
    compo_picturebox_t *pic_click;
    for (u8 idx = 0; idx < DISTURD_DISP_PIC_ITEM_CNT; idx++) {
        pic_click = compo_picturebox_create(frm, tbl_disturd_disp_pic_item[idx].res_addr);
        compo_setid(pic_click, tbl_disturd_disp_pic_item[idx].pic_id);
        compo_picturebox_set_pos(pic_click, tbl_disturd_disp_pic_item[idx].x, tbl_disturd_disp_pic_item[idx].y);
        compo_picturebox_set_visible(pic_click, tbl_disturd_disp_pic_item[idx].visible_en);
    }

    //获取显示时间
    u32 hour_start = sys_cb.disturd_start_time_sec / 3600;
    u32 min_start  = (sys_cb.disturd_start_time_sec % 3600) / 60;
    u32 hour_end   = sys_cb.disturd_end_time_sec / 3600;
    u32 min_end  = (sys_cb.disturd_end_time_sec % 3600) / 60;

    //创建数字
    compo_textbox_t *num_txt;
    char str_buff[8];
    for (u8 idx = 0; idx < DISTURD_NUM_ITEM_CNT; idx++) {
        num_txt = compo_textbox_create(frm, tbl_disturd_num_item[idx].num_cnt);
        compo_textbox_set_font(num_txt, tbl_disturd_num_item[idx].res_addr);
        compo_setid(num_txt, tbl_disturd_num_item[idx].num_id);
        compo_textbox_set_pos(num_txt, tbl_disturd_num_item[idx].x, tbl_disturd_num_item[idx].y);
        compo_textbox_set_visible(num_txt, tbl_disturd_num_item[idx].visible_en);

        if (tbl_disturd_num_item[idx].num_id ==  COMPO_ID_NUM_DISP_ZERO) {
            snprintf(str_buff, sizeof(str_buff), "%02ld", hour_start);
            compo_textbox_set(num_txt, str_buff);
        } else if (tbl_disturd_num_item[idx].num_id == COMPO_ID_NUM_DISP_ONE) {
            snprintf(str_buff, sizeof(str_buff), "%02ld", min_start);
            compo_textbox_set(num_txt, str_buff);
        } else if (tbl_disturd_num_item[idx].num_id == COMPO_ID_NUM_DISP_TWS) {
            snprintf(str_buff, sizeof(str_buff), "%02ld", hour_end);
            compo_textbox_set(num_txt, str_buff);
        } else if (tbl_disturd_num_item[idx].num_id == COMPO_ID_NUM_DISP_THR) {
            snprintf(str_buff, sizeof(str_buff), "%02ld", min_end);
            compo_textbox_set(num_txt, str_buff);
        }
    }

    return frm;
}

//勿扰模式事件处理
static void func_set_sub_disturd_process(void)
{
    //获取图片组件的地址
    compo_picturebox_t *pic_adl_on  = compo_getobj_byid(COMPO_ID_PIC_ADL_ON);
    compo_picturebox_t *pic_tim_on  = compo_getobj_byid(COMPO_ID_PIC_TIM_ON);
    compo_picturebox_t *pic_adl_off = compo_getobj_byid(COMPO_ID_PIC_ADL_OFF);
    compo_picturebox_t *pic_tim_off = compo_getobj_byid(COMPO_ID_PIC_TIM_OFF);

    //获取图像组件的地址
    //获取文本组件的地址
    compo_textbox_t *txt_disp[4];
    compo_textbox_t *num_disp[4];
    compo_shape_t *masklayer[2];
    for (int i=0;i<4;i++) {
        txt_disp[i] = compo_getobj_byid(COMPO_ID_TXT_START + i);
        num_disp[i] = compo_getobj_byid(COMPO_ID_NUM_DISP_ONE + i);
        if (i<2) {
            masklayer[i] = compo_getobj_byid(COMPO_ID_MSK_ONE + i);
        }
    }

    if (sys_cb.disturd_adl==0) {
        compo_picturebox_set_visible(pic_adl_off, true);
    } else {
        compo_picturebox_set_visible(pic_adl_on, true);
    }

    if(sys_cb.disturd_tim == 0) {
        compo_picturebox_set_visible(pic_tim_off, true);
        for (int i=0;i<4;i++) {
            compo_textbox_set_visible(txt_disp[i], false);
            compo_textbox_set_visible(num_disp[i], false);
            if(i<2) {
                compo_shape_set_alpha(masklayer[i], 0);
                }
        }
    } else {
        compo_picturebox_set_visible(pic_tim_on, true);
        for (int i=0;i<4;i++) {
            compo_textbox_set_visible(txt_disp[i], true);
            compo_textbox_set_visible(num_disp[i], true);
            if(i<2) {
                compo_shape_set_alpha(masklayer[i], 255);
            }
        }
    }

    func_process();
}

//更新显示勿扰模式界面
static void func_set_sub_disturd_disp(void)
{
    //获取图片组件的地址
    compo_picturebox_t *pic_adl_on  = compo_getobj_byid(COMPO_ID_PIC_ADL_ON);
    compo_picturebox_t *pic_tim_on  = compo_getobj_byid(COMPO_ID_PIC_TIM_ON);
    compo_picturebox_t *pic_adl_off = compo_getobj_byid(COMPO_ID_PIC_ADL_OFF);
    compo_picturebox_t *pic_tim_off = compo_getobj_byid(COMPO_ID_PIC_TIM_OFF);

    //获取文本组件的地址
    compo_textbox_t *txt[4];
    compo_textbox_t *num[4];
    for (int i=0;i<4;i++) {
        txt[i] = compo_getobj_byid(COMPO_ID_TXT_START+i);
        num[i] = compo_getobj_byid(COMPO_ID_NUM_DISP_ONE+i);
    }

    //显示界面各个组件
    if (sys_cb.disturd_adl == COMPO_ID_NUM_DISP_ONE) {
        compo_picturebox_set_visible(pic_adl_on, true);
        compo_picturebox_set_visible(pic_adl_off, false);
    } else {
        compo_picturebox_set_visible(pic_adl_on, false);
        compo_picturebox_set_visible(pic_adl_off, true);
    }

    if (sys_cb.disturd_tim == COMPO_ID_NUM_DISP_ONE) {
        compo_picturebox_set_visible(pic_tim_on, true);
        compo_picturebox_set_visible(pic_tim_off, false);
        for (int i=0;i<4;i++) {
            compo_textbox_set_visible(txt[i], true);
            compo_textbox_set_visible(num[i], true);
        }

    } else {
        compo_picturebox_set_visible(pic_tim_on, false);
        compo_picturebox_set_visible(pic_tim_off, true);
        for (int i=0;i<4;i++) {
            compo_textbox_set_visible(txt[i], false);
            compo_textbox_set_visible(num[i], false);
        }
    }
}

//单击按钮
static void func_disturd_button_click(void)
{
    u8 ret = 0;
    int id = compo_get_button_id();

    switch(id) {
    case COMPO_ID_BIN_ALLDAY_ON:
        if (!sys_cb.disturd_adl) {
            ret = msgbox((char *)i18n[STR_DISTURD_TIM_CTT], NULL , COMPO_ID_NUM_DISP_ONE, MSGBOX_MSG_TYPE_NONE);
        } else {
            ret = 0;
            sys_cb.disturd_adl = 0;
        }
        if (ret == COMPO_ID_NUM_DISP_ONE) {
            if (sys_cb.disturd_adl == COMPO_ID_NUM_DISP_ONE - 1) {
                sys_cb.disturd_adl = COMPO_ID_NUM_DISP_ONE;
            }
        }
    break;

    case COMPO_ID_BIN_TIMING_ON:
        if (!sys_cb.disturd_tim) {
            ret = msgbox((char *)i18n[STR_DISTURD_TIM_CTT], NULL , COMPO_ID_NUM_DISP_ONE, MSGBOX_MSG_TYPE_NONE);
        } else {
            ret = 0;
            sys_cb.disturd_tim = 0;
        }

        if (ret == COMPO_ID_NUM_DISP_ONE) {
            if(sys_cb.disturd_tim == COMPO_ID_NUM_DISP_ONE - 1) {
                sys_cb.disturd_tim = COMPO_ID_NUM_DISP_ONE;
            }
        }
    break;

    case COMPO_ID_BIN_START:
        if(sys_cb.disturd_tim) {
            sys_cb.disturd_sel = 0;
            func_cb.sta = FUNC_DISTURD_SUB_SET;
            task_stack_pop();
        }
        break;

    case COMPO_ID_BIN_END:
        if(sys_cb.disturd_tim) {
            sys_cb.disturd_sel = 1;
            func_cb.sta = FUNC_DISTURD_SUB_SET;
            task_stack_pop();
        }
        break;

    default:
    break;
    }

    func_set_sub_disturd_disp();
}

//勿扰模式能消息处理
static void func_set_sub_disturd_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_disturd_button_click();
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入勿扰模式功能
void func_set_sub_disturd_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_disturd_t));
    func_cb.frm_main = func_set_sub_disturd_form_create();
}

//退出勿扰模式功能
void func_set_sub_disturd_exit(void)
{
    func_cb.last = FUNC_SET_SUB_DISTURD;
}

//勿扰模式功能
void func_set_sub_disturd(void)
{
    printf("%s\n", __func__);
    func_set_sub_disturd_enter();
    while (func_cb.sta == FUNC_SET_SUB_DISTURD) {
        func_set_sub_disturd_process();
        func_set_sub_disturd_message(msg_dequeue());
    }
    func_set_sub_disturd_exit();
}
