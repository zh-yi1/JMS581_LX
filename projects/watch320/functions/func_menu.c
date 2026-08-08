#include "include.h"
#include "func.h"
#include "func_menu.h"

/* 无菜单：FUNC_MENU 入口一律转到首页 */

compo_form_t *func_menu_form_create(void)
{
    return compo_form_create(true);
}

u8 func_menu_sub_skyrer_get_first_idx(void)
{
    return 0;
}

void func_menu_sub_message(size_msg_t msg)
{
    (void)msg;
}

void func_menu_sub_exit(void)
{
}

void func_menu_enter(void)
{
    func_cb.sta = FUNC_HOME_PAGE;
}

void func_menu_exit(void)
{
    func_cb.last = FUNC_HOME_PAGE;
}

void func_menu(void)
{
    func_cb.sta = FUNC_HOME_PAGE;
}
