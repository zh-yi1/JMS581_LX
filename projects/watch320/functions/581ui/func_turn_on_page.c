#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* 开机页展示时长后进入首页 */
#define TURN_ON_SHOW_MS             2000

typedef struct {
    u32 show_tick;
    compo_picturebox_t *pic_logo;
} f_turn_on_t;

compo_form_t *func_turn_on_page_form_create(void)
{
    f_turn_on_t *f = (f_turn_on_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 黑色背景，避免 GPU 误读 UI 头当图解码 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 整个界面仅使用 turn_on.bin，居中显示 */
    f->pic_logo = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_TURN_ON_BIN);
    compo_picturebox_set_pos(f->pic_logo, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    return frm;
}

static void func_turn_on_page_process(void)
{
    f_turn_on_t *f = (f_turn_on_t *)func_cb.f_cb;

    if (f != NULL && tick_check_expire(f->show_tick, TURN_ON_SHOW_MS)) {
        func_cb.sta = FUNC_HOME_PAGE;
    }
    func_process();
}

static void func_turn_on_page_message(size_msg_t msg)
{
    switch (msg)
    {
    case MSG_CTP_CLICK:
    case KU_BACK:
        func_cb.sta = FUNC_HOME_PAGE;
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_turn_on_page_enter(void)
{
    f_turn_on_t *f;

    func_cb.f_cb = func_zalloc(sizeof(f_turn_on_t));
    f = (f_turn_on_t *)func_cb.f_cb;
    f->show_tick = tick_get();
    func_cb.frm_main = func_turn_on_page_form_create();
}

void func_turn_on_page_exit(void)
{
    func_cb.last = FUNC_TURN_ON_PAGE;
}

void func_turn_on_page(void)
{
    printf("%s\n", __func__);
    func_turn_on_page_enter();
    while (func_cb.sta == FUNC_TURN_ON_PAGE)
    {
        func_turn_on_page_process();
        func_turn_on_page_message(msg_dequeue());
    }
    func_turn_on_page_exit();
}
