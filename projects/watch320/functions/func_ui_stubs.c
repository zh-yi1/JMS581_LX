#include "include.h"
#include "func.h"
#include "func_bt.h"
#include "func_music.h"
#include "func_usbdev.h"
#include "func_recorder.h"

/* UI精简：业务 UI 源文件已删，保留声明所需的空实现，避免链接失败 */

void sfunc_bt_ring(void) {}
void sfunc_bt_call(void) {}

void func_bt_mp3_res_play(u32 addr, u32 len)
{
    (void)addr;
    (void)len;
}

void func_bt_mp3_play_restore(void) {}
void func_bt_exit(void) {}
void func_bt_init(void) {}
void func_bt_chk_off(void) {}
void func_bt_sub_process(void) {}
void func_bt_process(void) {}
void func_bt_enter(void) {}

void func_bt_display(void) {}
void func_bt_message(u16 msg) { (void)msg; }

void func_bt(void)
{
    /* 原蓝牙音乐页已删，直接退回时钟 */
    func_cb.sta = FUNC_CLOCK;
}

void func_bt_ring(void)
{
    func_cb.sta = FUNC_CLOCK;
}

void func_bt_call(void)
{
    func_cb.sta = FUNC_CLOCK;
}

void func_music_mp3_res_play(u32 addr, u32 len)
{
    (void)addr;
    (void)len;
}

bool func_music_filter_switch(u8 rec_type)
{
    (void)rec_type;
    return false;
}

void func_music_file_navigation(void) {}
void func_music_insert_device(u8 dev) { (void)dev; }
void func_music_remove_device(u8 dev) { (void)dev; }

bool func_music_is_play(void)
{
    return false;
}

void func_music_play(bool sta)
{
    (void)sta;
}

void ude_sdcard_insert(u8 dev) { (void)dev; }
void ude_sdcard_remove(u8 dev) { (void)dev; }

void func_recorder(void)
{
    func_cb.sta = FUNC_CLOCK;
}

void func_call_mgr_process(void) {}

/* dialplate time_map 已删，GUI bond 仍可能调用 */
u16 func_clock_time_map_r_get(bool is_sec)
{
    (void)is_sec;
    return 0;
}

void func_clock_time_map_r_set(bool is_sec, u16 value)
{
    (void)is_sec;
    (void)value;
}
