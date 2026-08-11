#ifndef __FUNC_TBL_H__
#define __FUNC_TBL_H__

#define FUNC_CREATE_CNT                       ((int)(sizeof(tbl_func_create) / sizeof(tbl_func_create[0])))
#define FUNC_ENTRY_CNT                        ((int)(sizeof(tbl_func_entry) / sizeof(tbl_func_entry[0])))
#define FUNC_ENTER_CNT                        ((int)(sizeof(tbl_func_enter) / sizeof(tbl_func_enter[0])))
#define FUNC_EXIT_CNT                         ((int)(sizeof(tbl_func_exit) / sizeof(tbl_func_exit[0])))

typedef struct func_t_ {
    int func_idx;
    void *func;
} func_t;


compo_form_t *func_menu_form_create(void);
compo_form_t *func_clock_form_create(void);
compo_form_t *func_charge_form_create(void);
compo_form_t *func_home_page_form_create(void);
compo_form_t *func_confirm_the_whole_card_page_form_create(void);
compo_form_t *func_contents_page_form_create(void);
compo_form_t *func_latest_n_day_backup_page_form_create(void);
compo_form_t *func_loading_1_page_form_create(void);
compo_form_t *func_backing_up_1_page_form_create(void);
compo_form_t *func_check_out_page_form_create(void);
compo_form_t *func_whole_card_done_page_form_create(void);
compo_form_t *func_loading_2_page_form_create(void);
compo_form_t *func_backing_up_2_page_form_create(void);
compo_form_t *func_latest_n_day_done_page_form_create(void);
compo_form_t *func_whole_card_no_new_page_form_create(void);
compo_form_t *func_latest_n_day_no_new_page_form_create(void);
compo_form_t *func_whole_card_not_enough_space_page_form_create(void);
compo_form_t *func_latest_n_day_not_enough_space_page_form_create(void);
compo_form_t *func_setup_page_form_create(void);
compo_form_t *func_language_page_form_create(void);
compo_form_t *func_format_page_form_create(void);
compo_form_t *func_formating_page_form_create(void);
compo_form_t *func_formated_page_form_create(void);
compo_form_t *func_upgrade_page_form_create(void);
compo_form_t *func_upgradeing_page_form_create(void);
compo_form_t *func_upgraded_page_form_create(void);

const func_t tbl_func_create[] = {
    {FUNC_MENU,                         func_menu_form_create},
    {FUNC_MENUSTYLE,                    NULL},
    {FUNC_CLOCK,                        func_clock_form_create},
    {FUNC_CHARGE,                       func_charge_form_create},
    {FUNC_HOME_PAGE,                    func_home_page_form_create},
    {FUNC_CONFIRM_WHOLE_CARD,           func_confirm_the_whole_card_page_form_create},
    {FUNC_CONTENTS_PAGE,                func_contents_page_form_create},
    {FUNC_LATEST_N_DAY_BACKUP,          func_latest_n_day_backup_page_form_create},
    {FUNC_LOADING_1_PAGE,               func_loading_1_page_form_create},
    {FUNC_BACKING_UP_1_PAGE,            func_backing_up_1_page_form_create},
    {FUNC_CHECK_OUT_PAGE,               func_check_out_page_form_create},
    {FUNC_WHOLE_CARD_DONE_PAGE,         func_whole_card_done_page_form_create},
    {FUNC_LOADING_2_PAGE,               func_loading_2_page_form_create},
    {FUNC_BACKING_UP_2_PAGE,            func_backing_up_2_page_form_create},
    {FUNC_LATEST_N_DAY_DONE_PAGE,       func_latest_n_day_done_page_form_create},
    {FUNC_WHOLE_CARD_NO_NEW_PAGE,       func_whole_card_no_new_page_form_create},
    {FUNC_LATEST_N_DAY_NO_NEW_PAGE,     func_latest_n_day_no_new_page_form_create},
    {FUNC_WHOLE_CARD_NOT_ENOUGH_SPACE_PAGE, func_whole_card_not_enough_space_page_form_create},
    {FUNC_LATEST_N_DAY_NOT_ENOUGH_SPACE_PAGE, func_latest_n_day_not_enough_space_page_form_create},
    {FUNC_SETUP_PAGE,                   func_setup_page_form_create},
    {FUNC_LANGUAGE_PAGE,                func_language_page_form_create},
    {FUNC_FORMAT_PAGE,                  func_format_page_form_create},
    {FUNC_FORMATING_PAGE,               func_formating_page_form_create},
    {FUNC_FORMATED_PAGE,                func_formated_page_form_create},
    {FUNC_UPGRADE_PAGE,                 func_upgrade_page_form_create},
    {FUNC_UPGRADEING_PAGE,              func_upgradeing_page_form_create},
    {FUNC_UPGRADED_PAGE,                func_upgraded_page_form_create},
};

extern void func_menu(void);
extern void func_clock(void);
extern void func_switching_to_menu(void);
extern void func_charge(void);
extern void func_home_page(void);
extern void func_confirm_the_whole_card_page(void);
extern void func_contents_page(void);
extern void func_latest_n_day_backup_page(void);
extern void func_loading_1_page(void);
extern void func_backing_up_1_page(void);
extern void func_check_out_page(void);
extern void func_whole_card_done_page(void);
extern void func_loading_2_page(void);
extern void func_backing_up_2_page(void);
extern void func_latest_n_day_done_page(void);
extern void func_whole_card_no_new_page(void);
extern void func_latest_n_day_no_new_page(void);
extern void func_whole_card_not_enough_space_page(void);
extern void func_latest_n_day_not_enough_space_page(void);
extern void func_setup_page(void);
extern void func_language_page(void);
extern void func_format_page(void);
extern void func_formating_page(void);
extern void func_formated_page(void);
extern void func_upgrade_page(void);
extern void func_upgradeing_page(void);
extern void func_upgraded_page(void);

extern void func_idle(void);
extern void func_ota_ui(void);
extern void func_bt_update(void);

const func_t tbl_func_entry[] = {
    {FUNC_MENU,                         func_menu},                     //已重定向到首页
    {FUNC_CLOCK,                        func_clock},                    //时钟表盘
    {FUNC_CHARGE,                       func_charge},                   //充电
    {FUNC_HOME_PAGE,                    func_home_page},                //主页
    {FUNC_CONFIRM_WHOLE_CARD,           func_confirm_the_whole_card_page},
    {FUNC_CONTENTS_PAGE,                func_contents_page},
    {FUNC_LATEST_N_DAY_BACKUP,          func_latest_n_day_backup_page},
    {FUNC_LOADING_1_PAGE,               func_loading_1_page},
    {FUNC_BACKING_UP_1_PAGE,            func_backing_up_1_page},
    {FUNC_CHECK_OUT_PAGE,               func_check_out_page},
    {FUNC_WHOLE_CARD_DONE_PAGE,         func_whole_card_done_page},
    {FUNC_LOADING_2_PAGE,               func_loading_2_page},
    {FUNC_BACKING_UP_2_PAGE,            func_backing_up_2_page},
    {FUNC_LATEST_N_DAY_DONE_PAGE,       func_latest_n_day_done_page},
    {FUNC_WHOLE_CARD_NO_NEW_PAGE,       func_whole_card_no_new_page},
    {FUNC_LATEST_N_DAY_NO_NEW_PAGE,     func_latest_n_day_no_new_page},
    {FUNC_WHOLE_CARD_NOT_ENOUGH_SPACE_PAGE, func_whole_card_not_enough_space_page},
    {FUNC_LATEST_N_DAY_NOT_ENOUGH_SPACE_PAGE, func_latest_n_day_not_enough_space_page},
    {FUNC_SETUP_PAGE,                   func_setup_page},
    {FUNC_LANGUAGE_PAGE,                func_language_page},
    {FUNC_FORMAT_PAGE,                  func_format_page},
    {FUNC_FORMATING_PAGE,               func_formating_page},
    {FUNC_FORMATED_PAGE,                func_formated_page},
    {FUNC_UPGRADE_PAGE,                 func_upgrade_page},
    {FUNC_UPGRADEING_PAGE,              func_upgradeing_page},
    {FUNC_UPGRADED_PAGE,                func_upgraded_page},
#if FUNC_IDLE_EN
    {FUNC_IDLE,                         func_idle},
#endif
#if FOTA_UI_EN
    {FUNC_OTA_UI_MODE,                  func_ota_ui},
#endif
    {FUNC_BT_UPDATE,                    func_bt_update},
};

void func_menu_enter(void);
void func_clock_enter(void);
void func_charge_enter(void);
void func_home_page_enter(void);
void func_confirm_the_whole_card_page_enter(void);
void func_contents_page_enter(void);
void func_latest_n_day_backup_page_enter(void);
void func_loading_1_page_enter(void);
void func_backing_up_1_page_enter(void);
void func_check_out_page_enter(void);
void func_whole_card_done_page_enter(void);
void func_loading_2_page_enter(void);
void func_backing_up_2_page_enter(void);
void func_latest_n_day_done_page_enter(void);
void func_whole_card_no_new_page_enter(void);
void func_latest_n_day_no_new_page_enter(void);
void func_whole_card_not_enough_space_page_enter(void);
void func_latest_n_day_not_enough_space_page_enter(void);
void func_setup_page_enter(void);
void func_language_page_enter(void);
void func_format_page_enter(void);
void func_formating_page_enter(void);
void func_formated_page_enter(void);
void func_upgrade_page_enter(void);
void func_upgradeing_page_enter(void);
void func_upgraded_page_enter(void);
#if FUNC_IDLE_EN
void func_idle_enter(void);
#endif
#if FOTA_UI_EN
void func_ota_ui_enter(void);
#endif
void func_bt_update_enter(void);

const func_t tbl_func_enter[] = {
    {FUNC_MENU,                         func_menu_enter},
    {FUNC_CLOCK,                        func_clock_enter},
    {FUNC_CHARGE,                       func_charge_enter},
    {FUNC_HOME_PAGE,                    func_home_page_enter},
    {FUNC_CONFIRM_WHOLE_CARD,           func_confirm_the_whole_card_page_enter},
    {FUNC_CONTENTS_PAGE,                func_contents_page_enter},
    {FUNC_LATEST_N_DAY_BACKUP,          func_latest_n_day_backup_page_enter},
    {FUNC_LOADING_1_PAGE,               func_loading_1_page_enter},
    {FUNC_BACKING_UP_1_PAGE,            func_backing_up_1_page_enter},
    {FUNC_CHECK_OUT_PAGE,               func_check_out_page_enter},
    {FUNC_WHOLE_CARD_DONE_PAGE,         func_whole_card_done_page_enter},
    {FUNC_LOADING_2_PAGE,               func_loading_2_page_enter},
    {FUNC_BACKING_UP_2_PAGE,            func_backing_up_2_page_enter},
    {FUNC_LATEST_N_DAY_DONE_PAGE,       func_latest_n_day_done_page_enter},
    {FUNC_WHOLE_CARD_NO_NEW_PAGE,       func_whole_card_no_new_page_enter},
    {FUNC_LATEST_N_DAY_NO_NEW_PAGE,     func_latest_n_day_no_new_page_enter},
    {FUNC_WHOLE_CARD_NOT_ENOUGH_SPACE_PAGE, func_whole_card_not_enough_space_page_enter},
    {FUNC_LATEST_N_DAY_NOT_ENOUGH_SPACE_PAGE, func_latest_n_day_not_enough_space_page_enter},
    {FUNC_SETUP_PAGE,                   func_setup_page_enter},
    {FUNC_LANGUAGE_PAGE,                func_language_page_enter},
    {FUNC_FORMAT_PAGE,                  func_format_page_enter},
    {FUNC_FORMATING_PAGE,               func_formating_page_enter},
    {FUNC_FORMATED_PAGE,                func_formated_page_enter},
    {FUNC_UPGRADE_PAGE,                 func_upgrade_page_enter},
    {FUNC_UPGRADEING_PAGE,              func_upgradeing_page_enter},
    {FUNC_UPGRADED_PAGE,                func_upgraded_page_enter},
#if FUNC_IDLE_EN
    {FUNC_IDLE,                         func_idle_enter},
#endif
#if FOTA_UI_EN
    {FUNC_OTA_UI_MODE,                  func_ota_ui_enter},
#endif
    {FUNC_BT_UPDATE,                    func_bt_update_enter},
};


void func_menu_exit(void);
void func_clock_exit(void);
void func_charge_exit(void);
void func_home_page_exit(void);
void func_confirm_the_whole_card_page_exit(void);
void func_contents_page_exit(void);
void func_latest_n_day_backup_page_exit(void);
void func_loading_1_page_exit(void);
void func_backing_up_1_page_exit(void);
void func_check_out_page_exit(void);
void func_whole_card_done_page_exit(void);
void func_loading_2_page_exit(void);
void func_backing_up_2_page_exit(void);
void func_latest_n_day_done_page_exit(void);
void func_whole_card_no_new_page_exit(void);
void func_latest_n_day_no_new_page_exit(void);
void func_whole_card_not_enough_space_page_exit(void);
void func_latest_n_day_not_enough_space_page_exit(void);
void func_setup_page_exit(void);
void func_language_page_exit(void);
void func_format_page_exit(void);
void func_formating_page_exit(void);
void func_formated_page_exit(void);
void func_upgrade_page_exit(void);
void func_upgradeing_page_exit(void);
void func_upgraded_page_exit(void);
#if FUNC_IDLE_EN
void func_idle_exit(void);
#endif
#if FOTA_UI_EN
void func_ota_ui_exit(void);
#endif
void func_bt_update_exit(void);

const func_t tbl_func_exit[] = {
    {FUNC_MENU,                         func_menu_exit},
    {FUNC_CLOCK,                        func_clock_exit},
    {FUNC_CHARGE,                       func_charge_exit},
    {FUNC_HOME_PAGE,                    func_home_page_exit},
    {FUNC_CONFIRM_WHOLE_CARD,           func_confirm_the_whole_card_page_exit},
    {FUNC_CONTENTS_PAGE,                func_contents_page_exit},
    {FUNC_LATEST_N_DAY_BACKUP,          func_latest_n_day_backup_page_exit},
    {FUNC_LOADING_1_PAGE,               func_loading_1_page_exit},
    {FUNC_BACKING_UP_1_PAGE,            func_backing_up_1_page_exit},
    {FUNC_CHECK_OUT_PAGE,               func_check_out_page_exit},
    {FUNC_WHOLE_CARD_DONE_PAGE,         func_whole_card_done_page_exit},
    {FUNC_LOADING_2_PAGE,               func_loading_2_page_exit},
    {FUNC_BACKING_UP_2_PAGE,            func_backing_up_2_page_exit},
    {FUNC_LATEST_N_DAY_DONE_PAGE,       func_latest_n_day_done_page_exit},
    {FUNC_WHOLE_CARD_NO_NEW_PAGE,       func_whole_card_no_new_page_exit},
    {FUNC_LATEST_N_DAY_NO_NEW_PAGE,     func_latest_n_day_no_new_page_exit},
    {FUNC_WHOLE_CARD_NOT_ENOUGH_SPACE_PAGE, func_whole_card_not_enough_space_page_exit},
    {FUNC_LATEST_N_DAY_NOT_ENOUGH_SPACE_PAGE, func_latest_n_day_not_enough_space_page_exit},
    {FUNC_SETUP_PAGE,                   func_setup_page_exit},
    {FUNC_LANGUAGE_PAGE,                func_language_page_exit},
    {FUNC_FORMAT_PAGE,                  func_format_page_exit},
    {FUNC_FORMATING_PAGE,               func_formating_page_exit},
    {FUNC_FORMATED_PAGE,                func_formated_page_exit},
    {FUNC_UPGRADE_PAGE,                 func_upgrade_page_exit},
    {FUNC_UPGRADEING_PAGE,              func_upgradeing_page_exit},
    {FUNC_UPGRADED_PAGE,                func_upgraded_page_exit},
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
};

#endif // _FUNC_H
