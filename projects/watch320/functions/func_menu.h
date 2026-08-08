#ifndef _FUNC_MENU_H
#define _FUNC_MENU_H

/* 保留枚举，供 func.c 等历史逻辑编译；实际无菜单 UI */
enum {
    MENU_STYLE_HONEYCOMB,
    MENU_STYLE_WATERFALL,
    MENU_STYLE_LIST,
    MENU_STYLE_FOOTBALL,
    MENU_STYLE_SUDOKU,
    MENU_STYLE_SUDOKU_HRZ,
    MENU_STYLE_GRID,
    MENU_STYLE_DISK,
    MENU_STYLE_RING,
    MENU_STYLE_KALE,
    MENU_STYLE_SKYRER,
    MENU_STYLE_CUM_SUDOKU,
    MENU_STYLE_CUM_GRID,
    MENU_STYLE_CUM_HEXAGON,
    MENU_STYLE_CUM_FOURGRID,
    MENU_STYLE_CUM_HONEYGRID,
    MENU_STYLE_CNT,
};

compo_form_t *func_menu_form_create(void);
void func_menu_sub_message(size_msg_t msg);
void func_menu_sub_exit(void);
u8 func_menu_sub_skyrer_get_first_idx(void);

#endif
