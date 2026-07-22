#ifndef __FUNC_TBL_H__
#define __FUNC_TBL_H__

#define FUNC_CREATE_CNT                       ((int)(sizeof(tbl_func_create) / sizeof(tbl_func_create[0])))
#define FUNC_ENTRY_CNT                        ((int)(sizeof(tbl_func_entry) / sizeof(tbl_func_entry[0])))
#define FUNC_ENTER_CNT                        ((int)(sizeof(tbl_func_enter) / sizeof(tbl_func_enter[0])))
#define FUNC_EXIT_CNT                        ((int)(sizeof(tbl_func_exit) / sizeof(tbl_func_exit[0])))

typedef struct func_t_ {
    int func_idx;
    void *func;
} func_t;


compo_form_t *func_menu_form_create(void);
compo_form_t *func_clock_form_create(void);
compo_form_t *func_compass_form_create(void);
compo_form_t *func_charge_form_create(void);
compo_form_t *func_debug_info_form_create(void);
compo_form_t *func_home_page_form_create(void);
compo_form_t *func_gif_form_create(void);

const func_t tbl_func_create[] = {
    {FUNC_MENU,                         func_menu_form_create},
    {FUNC_MENUSTYLE,                    NULL},
    {FUNC_CLOCK,                        func_clock_form_create},
    {FUNC_COMPASS,                      func_compass_form_create},
    {FUNC_CHARGE,                       func_charge_form_create},
    {FUNC_DEBUG_INFO,                   func_debug_info_form_create},
    {FUNC_HOME_PAGE,                    func_home_page_form_create},
    {FUNC_GIF,                          func_gif_form_create},

};

extern void func_menu(void);
extern void func_clock(void);
extern void func_compass(void);
extern void func_switching_to_menu(void);
extern void func_charge(void);
extern void func_debug_info(void);
extern void func_home_page(void);

extern void func_idle(void);
extern void func_ota_ui(void);
extern void func_bt_update(void);
extern void func_gif(void);

const func_t tbl_func_entry[] = {
    {FUNC_MENU,                         func_menu},                     //主菜单(蜂窝)
    {FUNC_CLOCK,                        func_clock},                    //时钟表盘
    {FUNC_COMPASS,                      func_compass},                  //指南针
    {FUNC_CHARGE,                       func_charge},                   //充电
    {FUNC_DEBUG_INFO,                   func_debug_info},               //DEBUG
    {FUNC_HOME_PAGE,                    func_home_page},                //主页
#if FUNC_BT_EN
  //  {FUNC_BT,                           func_bt},                       //桩：回时钟
 //   {FUNC_BT_RING,                      func_bt_ring},                  //桩：回时钟
  //  {FUNC_BT_CALL,                      func_bt_call},                  //桩：回时钟
#endif // FUNC_BT_EN
#if FUNC_BT_DUT_EN
    {FUNC_BT_DUT,                       func_bt_dut},
#endif // FUNC_BT_DUT_EN
#if FUNC_FMRX_EN
    {FUNC_FMRX,                         func_fmrx},
#endif
#if FUNC_IDLE_EN
    {FUNC_IDLE,                         func_idle},
#endif
#if FOTA_UI_EN
    {FUNC_OTA_UI_MODE,                  func_ota_ui},
#endif
    {FUNC_BT_UPDATE,                    func_bt_update},
    {FUNC_GIF,                          func_gif},


};

void func_menu_enter(void);
void func_clock_enter(void);
void func_compass_enter(void);
void func_charge_enter(void);
void func_debug_enter(void);
void func_home_page_enter(void);
#if FUNC_FMRX_EN
void func_fmrx_enter(void);
#endif
#if FUNC_IDLE_EN
void func_idle_enter(void);
#endif
#if FOTA_UI_EN
void func_ota_ui_enter(void);
#endif
void func_bt_update_enter(void);
void func_gif_enter(void);

const func_t tbl_func_enter[] = {
    {FUNC_MENU,                         func_menu_enter},                     //主菜单(蜂窝)
    {FUNC_CLOCK,                        func_clock_enter},                    //时钟表盘
    {FUNC_COMPASS,                      func_compass_enter},                  //指南针
    {FUNC_CHARGE,                       func_charge_enter},                   //充电
    {FUNC_DEBUG_INFO,                   func_debug_enter},               //DEBUG
    {FUNC_HOME_PAGE,                    func_home_page_enter},           //主页
#if FUNC_BT_DUT_EN
    {FUNC_BT_DUT,                       NULL},
#endif // FUNC_BT_DUT_EN
#if FUNC_FMRX_EN
    {FUNC_FMRX,                         func_fmrx_enter},
#endif
#if FUNC_IDLE_EN
    {FUNC_IDLE,                         func_idle_enter},
#endif
#if FOTA_UI_EN
    {FUNC_OTA_UI_MODE,                  func_ota_ui_enter},
#endif
    {FUNC_BT_UPDATE,                    func_bt_update_enter},
    {FUNC_GIF,                          func_gif_enter},
};


void func_menu_exit(void);
void func_clock_exit(void);
void func_compass_exit(void);
void func_charge_exit(void);
void func_debug_info_exit(void);
void func_home_page_exit(void);
#if FUNC_FMRX_EN
void func_fmrx_exit(void);
#endif
#if FUNC_IDLE_EN
void func_idle_exit(void);
#endif
#if FOTA_UI_EN
void func_ota_ui_exit(void);
#endif
void func_bt_update_exit(void);
void func_gif_exit(void);

const func_t tbl_func_exit[] = {
    {FUNC_MENU,                         func_menu_exit},                     //主菜单(蜂窝)
    {FUNC_CLOCK,                        func_clock_exit},                    //时钟表盘
    {FUNC_COMPASS,                      func_compass_exit},                  //指南针
    {FUNC_CHARGE,                       func_charge_exit},                   //充电
    {FUNC_DEBUG_INFO,                   func_debug_info_exit},               //DEBUG
    {FUNC_HOME_PAGE,                    func_home_page_exit},            //主页
#if FUNC_BT_DUT_EN
    {FUNC_BT_DUT,                       NULL},
#endif // FUNC_BT_DUT_EN
#if FUNC_FMRX_EN
    {FUNC_FMRX,                         func_fmrx_exit},
#endif
#if FUNC_IDLE_EN
    {FUNC_IDLE,                         func_idle_exit},
#endif
#if FOTA_UI_EN
    {FUNC_OTA_UI_MODE,                  func_ota_ui_exit},
#endif
    {FUNC_BT_UPDATE,                    func_bt_update_exit},
#if FLASHDB_EN
    {FUNC_MESSAGE_REPLY,                NULL},
#endif
    {FUNC_GIF,                          func_gif_exit},
};

#endif // _FUNC_H
