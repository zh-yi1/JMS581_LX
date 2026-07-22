#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define VOL_CHANGE          4  //音量等级每次增加或者减少4
#define MOTOR_MAX_LEVEL     4  //马达最大等级

typedef struct f_sav_t_ {
    u8 vol_value;
    u8 shk_value;
} f_sav_t;

enum {
    //数字
    COMPO_ID_NUM_DISP_ONE = 1,
    COMPO_ID_NUM_DISP_TWS,

    //按键
    COMPO_ID_BIN_REDUCE_ONE,
    COMPO_ID_BIN_INCREASE_ONE,
    COMPO_ID_BIN_REDUCE_TWS,
    COMPO_ID_BIN_INCREASE_TWS,
    COMPO_ID_BIN_REDUCE_THR,
    COMPO_ID_BIN_INCREASE_THR,
    //图像
	COMPO_ID_PIC_LEVEL1,            //音量目前等级 1~5
	COMPO_ID_PIC_LEVEL2,
	COMPO_ID_PIC_LEVEL3,
	COMPO_ID_PIC_LEVEL4,
	COMPO_ID_PIC_LEVEL5,
    COMPO_ID_PIC_LEVEL6,
	COMPO_ID_PIC_LEVEL7,
	COMPO_ID_PIC_LEVEL8,
	COMPO_ID_PIC_LEVEL9,
	COMPO_ID_PIC_LEVEL10,
    COMPO_ID_PIC_LEVEL11,
	COMPO_ID_PIC_LEVEL12,
	COMPO_ID_PIC_LEVEL13,
	COMPO_ID_PIC_LEVEL14,
	COMPO_ID_PIC_LEVEL15,
};

#define SAV_DISP_BTN_ITEM_CNT    ((int)(sizeof(tbl_sav_disp_btn_item) / sizeof(tbl_sav_disp_btn_item[0])))

typedef struct sav_disp_btn_item_t_ {
    u32 res_addr;
    u16 btn_id;
    s16 x;
    s16 y;
    s16 h;
    s16 l;
} sav_disp_btn_item_t;

//按钮item，创建时遍历一下
static const sav_disp_btn_item_t tbl_sav_disp_btn_item[] = {
    {UI_BUF_COMMON_REDUCE_BIN,    COMPO_ID_BIN_REDUCE_ONE,            54,      150,   80,  40},
    {UI_BUF_COMMON_INCREASE_BIN,  COMPO_ID_BIN_INCREASE_ONE,          266,     150,   80,  40},
    {UI_BUF_COMMON_REDUCE_BIN,    COMPO_ID_BIN_REDUCE_TWS,            54,      260,   80,  40},
    {UI_BUF_COMMON_INCREASE_BIN,  COMPO_ID_BIN_INCREASE_TWS,          266,     260,   80,  40},
    {UI_BUF_COMMON_REDUCE_BIN,    COMPO_ID_BIN_REDUCE_THR,            54,      360,   80,  40},
    {UI_BUF_COMMON_INCREASE_BIN,  COMPO_ID_BIN_INCREASE_THR,          266,     360,   80,  40},
};

typedef struct sav_disp_pic_item_t_ {
    u32 res_addr;
    u16 pic_id;
    s16 x;
    s16 y;
    bool visible_en;
} sav_disp_pic_item_t;

#define SAV_DISP_PIC_ITEM_CNT                       ((int)(sizeof(tbl_sav_disp_pic_item) / sizeof(tbl_sav_disp_pic_item[0])))

//图片item，创建时遍历一下
static const sav_disp_pic_item_t tbl_sav_disp_pic_item[] = {
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL1,          108,    150,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL2,          134,    150,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL3,          160,    150,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL4,          186,    150,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL5,          212,    150,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL6,          108,    260,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL7,          134,    260,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL8,          160,    260,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL9,          186,    260,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL10,         212,    260,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL11,         108,    360,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL12,         134,    360,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL13,         160,    360,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL14,         186,    360,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL15,         212,    360,    false},
};

//声音与振动页面
compo_form_t *func_set_sub_sav_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);
    compo_form_add_image(frm, UI_BUF_COMMON_BG1_BIN, 160, 150);
    compo_form_add_image(frm, UI_BUF_COMMON_BG1_BIN, 160, 260);
    compo_form_add_image(frm, UI_BUF_COMMON_BG1_BIN, 160, 360);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_SETTING_SAV]);

    //创建文本
    compo_textbox_t *txt_voice = compo_textbox_create(frm, 4);
    compo_textbox_set_align_center(txt_voice, false);
    compo_textbox_set_pos(txt_voice, 30, 80);
    compo_textbox_set(txt_voice, i18n[STR_MEDIA_VOL]);

    compo_textbox_t *txt_call = compo_textbox_create(frm, 4);
    compo_textbox_set_align_center(txt_call, false);
    compo_textbox_set_pos(txt_call, 30, 190);
    compo_textbox_set(txt_call, i18n[STR_CALL_VOL]);

    compo_textbox_t *txt_shake = compo_textbox_create(frm, 2);
    compo_textbox_set_align_center(txt_shake, false);
    compo_textbox_set_pos(txt_shake, 30, 300);
    compo_textbox_set(txt_shake, i18n[STR_SHK]);

    //创建按钮
    compo_button_t *btn;
    for (u8 idx = 0; idx < SAV_DISP_BTN_ITEM_CNT; idx++) {
        btn = compo_button_create_by_image(frm, tbl_sav_disp_btn_item[idx].res_addr);
        compo_setid(btn, tbl_sav_disp_btn_item[idx].btn_id);
        compo_button_set_pos(btn, tbl_sav_disp_btn_item[idx].x, tbl_sav_disp_btn_item[idx].y);

        compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA);

    }

    //新建图像
    compo_picturebox_t *pic_click[SAV_DISP_PIC_ITEM_CNT];
    for (u8 idx = 0; idx < SAV_DISP_PIC_ITEM_CNT; idx++) {
        pic_click[idx] = compo_picturebox_create(frm, tbl_sav_disp_pic_item[idx].res_addr);
        compo_setid(pic_click[idx], tbl_sav_disp_pic_item[idx].pic_id);
        compo_picturebox_set_pos(pic_click[idx], tbl_sav_disp_pic_item[idx].x, tbl_sav_disp_pic_item[idx].y);
        compo_picturebox_set_visible(pic_click[idx], tbl_sav_disp_pic_item[idx].visible_en);
    }

    compo_picturebox_set_visible(pic_click[0], true);
    compo_picturebox_set_visible(pic_click[5], true);
    compo_picturebox_set_visible(pic_click[10], true);

    for (u8 i=0; i<VOL_MAX; i+=VOL_CHANGE) {
        if (i < sys_cb.vol) {
            compo_picturebox_set_visible(pic_click[i/VOL_CHANGE + 1], true);
        } else {
            compo_picturebox_set_visible(pic_click[i/VOL_CHANGE + 1], false);
        }
        if (i < sys_cb.hfp_vol) {
            compo_picturebox_set_visible(pic_click[i/VOL_CHANGE + 6], true);
        } else {
            compo_picturebox_set_visible(pic_click[i/VOL_CHANGE + 6], false);
        }
    }

    for (u8 i=0; i<MOTOR_MAX_LEVEL; i+=1) {
        if (i < sys_cb.motor_level) {
            compo_picturebox_set_visible(pic_click[i + 11], true);
        } else {
            compo_picturebox_set_visible(pic_click[i + 11], false);
        }
    }

    return frm;
}

//声音与振动事件处理
static void func_set_sub_sav_process(void)
{
    func_process();
}

//更新显示界面
static void func_set_sub_sav_disp(void)
{
    compo_picturebox_t *pic_level[SAV_DISP_PIC_ITEM_CNT];
    for (int i=0;i<SAV_DISP_PIC_ITEM_CNT;i++) {
        pic_level[i] = compo_getobj_byid(COMPO_ID_PIC_LEVEL1 + i);
    }

    if (sys_cb.vol > VOL_MAX) {
        sys_cb.vol = VOL_MAX;
    } else if (sys_cb.vol <0) {
        sys_cb.vol = 0;
    }

    if (sys_cb.hfp_vol > VOL_MAX) {
        sys_cb.hfp_vol = VOL_MAX;
    } else if (sys_cb.hfp_vol <0) {
        sys_cb.hfp_vol = 0;
    }

    if (sys_cb.motor_level > MOTOR_MAX_LEVEL) {
        sys_cb.motor_level = MOTOR_MAX_LEVEL;
    } else if (sys_cb.motor_level <0) {
        sys_cb.motor_level = 0;
    }

    for (u8 i=0; i<VOL_MAX; i+=VOL_CHANGE) {
        if (i < sys_cb.vol) {
            compo_picturebox_set_visible(pic_level[i/VOL_CHANGE + 1], true);
        } else {
            compo_picturebox_set_visible(pic_level[i/VOL_CHANGE + 1], false);
        }
        if (i < sys_cb.hfp_vol) {
            compo_picturebox_set_visible(pic_level[i/VOL_CHANGE + 6], true);
        } else {
            compo_picturebox_set_visible(pic_level[i/VOL_CHANGE + 6], false);
        }
    }

    for (u8 i=0; i<MOTOR_MAX_LEVEL; i+=1) {
        if (i < sys_cb.motor_level) {
            compo_picturebox_set_visible(pic_level[i + 11], true);
        } else {
            compo_picturebox_set_visible(pic_level[i + 11], false);
        }
    }
}

//单击按钮
static void func_sav_button_click(void)
{
    int id = compo_get_button_id();
    switch(id) {
    case COMPO_ID_BIN_REDUCE_ONE:       //音频音量
        if (sys_cb.vol > 0 && sys_cb.vol <= VOL_MAX) {
            sys_cb.vol -= VOL_CHANGE;
        }
        break;

    case COMPO_ID_BIN_INCREASE_ONE:
    if (sys_cb.vol >= 0 && sys_cb.vol <= VOL_MAX) {
            sys_cb.vol += VOL_CHANGE;
        }
        break;

    case COMPO_ID_BIN_REDUCE_TWS:     //通话音量
       if (sys_cb.hfp_vol > 0 && sys_cb.hfp_vol <= VOL_MAX) {
            sys_cb.hfp_vol -= VOL_CHANGE;
        }
        break;

    case COMPO_ID_BIN_INCREASE_TWS:   //通话音量
       if (sys_cb.hfp_vol >= 0 && sys_cb.hfp_vol <= VOL_MAX) {
            sys_cb.hfp_vol += VOL_CHANGE;
        }
        break;

    case COMPO_ID_BIN_REDUCE_THR:     //马达等级
       if (sys_cb.motor_level > 0 && sys_cb.motor_level <= MOTOR_MAX_LEVEL) {
            sys_cb.motor_level -= 1;
        }
        break;

    case COMPO_ID_BIN_INCREASE_THR:   //马达等级
       if (sys_cb.motor_level >= 0 && sys_cb.motor_level <= 6) {
            sys_cb.motor_level += 1;
        }
        break;

    default:
        break;
    }
    func_set_sub_sav_disp();
}

//声音与振动功能消息处理
static void func_set_sub_sav_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_sav_button_click();
        break;

    default:
        func_message(msg);
        break;
    }

}

//进入声音与振动功能
void func_set_sub_sav_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_sav_t));
    func_cb.frm_main = func_set_sub_sav_form_create();

    f_sav_t *sav = (f_sav_t *)func_cb.f_cb;
    sav->shk_value = COMPO_ID_NUM_DISP_ONE;
    sav->vol_value = COMPO_ID_NUM_DISP_ONE;
}

//退出声音与振动功能
void func_set_sub_sav_exit(void)
{
    func_cb.last = FUNC_SET_SUB_SAV;
}

//声音与振动功能
void func_set_sub_sav(void)
{
    printf("%s\n", __func__);
    func_set_sub_sav_enter();
    while (func_cb.sta == FUNC_SET_SUB_SAV) {
        func_set_sub_sav_process();
        func_set_sub_sav_message(msg_dequeue());
    }
    func_set_sub_sav_exit();
}
