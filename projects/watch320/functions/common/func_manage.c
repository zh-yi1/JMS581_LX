#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/**
 * -------------function item infomation-----------------
            任务项id、图标资源、标题序号信息
 **/
typedef struct {
    u8 func_sta;
    u32 res_addr;
    u32 str_idx;
} func_item_info_t;

#define FUNC_ITEM_CNT   (sizeof(func_item_tbl) / sizeof(func_item_tbl[0]))
const static func_item_info_t func_item_tbl[] = {
    {FUNC_CLOCK,                    UI_BUF_ICON_ACTIVITY_BIN,                    STR_CLOCK},
    {FUNC_HOME_PAGE,                UI_BUF_ICON_ACTIVITY_BIN,                    STR_HOME},
    {FUNC_CHARGE,                   UI_BUF_ICON_ACTIVITY_BIN,                    STR_HOME},
};

/**
 * @brief 获取任务图标资源地址
 * @param[in] func_sta : 任务id
 * @return 资源地址
 **/
u32 func_get_icon_addr(u8 func_sta) {
    for (u8 i = 0; i < FUNC_ITEM_CNT; i++) {
        if (func_item_tbl[i].func_sta == func_sta) {
            return func_item_tbl[i].res_addr;
        }
    }
    return 0;
}

/**
 * @brief 获取任务标题字符串序号
 * @param[in] func_sta : 任务id
 * @return 标题字符串序号
 **/
u32 func_get_str_idx(u8 func_sta) {
    for (u8 i = 0; i < FUNC_ITEM_CNT; i++) {
        if (func_item_tbl[i].func_sta == func_sta) {
            return func_item_tbl[i].str_idx;
        }
    }
    return 0;
}


/**
 * -------------gui stack manage-----------------
    根据界面跳转的复杂程度是否入栈管理
 **/
typedef struct {
    u8 task_tbl[TASK_STACK_MAX];
    u8 num;
} task_stack_t;

static task_stack_t task_stack;

/**
 * @brief 堆栈初始化
 **/
void task_stack_init(void)
{
    task_stack.num = 0;
}

/**
 * @brief 入栈
          目前首先入栈的是时钟表盘
 * @param[in] val : 入栈的值
 **/
void task_stack_push(u8 val)
{
    printf("%s:[", __func__);
    for (u8 i = 0; i < task_stack.num; i++) {    //有重复值直接回退到第一次入栈处
        if (task_stack.task_tbl[i] == val) {
            task_stack.num = i + 1;
            printf("%d]<--[%d]\n", val, val);
            return;
        } else {
            printf("%d ", task_stack.task_tbl[i]);
        }
    }
    if (task_stack.num < TASK_STACK_MAX) {
        task_stack.task_tbl[task_stack.num] = val;
        task_stack.num++;
        printf("%d]<--[%d]\n", val, val);
    } else {
        printf("]<--[%d]\n", val, val);
    }
}

/**
 * @brief 出栈
 * @return 出栈后，处于栈顶的值
 **/
u8 task_stack_pop(void)
{
    printf("%s", __func__);
    TRACE(":[");
    u8 val = 0;
#if TRACE_EN
    if (task_stack.num > 0) {
        for (u8 i = 0; i < (task_stack.num - 1); i++) {
            if (i == task_stack.num - 2) {
                val = task_stack.task_tbl[i];
                TRACE("%d", val);
            } else {
                TRACE("%d ", task_stack.task_tbl[i]);
            }
        }
        task_stack.num--;
        TRACE("]-->[%d]\n", task_stack.task_tbl[task_stack.num]);
    } else {
	    TRACE("]-->[]\n");
	}
#else
    if (task_stack.num > 0) {
        task_stack.num--;
        if (task_stack.num) {
            val = task_stack.task_tbl[task_stack.num - 1];
        }
    }
    printf("[top:%d]\n", val);
#endif // TRACE_EN
    return val;
}

/**
 * @brief 获取栈顶
 * @return 栈顶的值
 **/
u8 gui_get_stack_top(void)
{
    if (task_stack.num > 0) {
        return task_stack.task_tbl[task_stack.num - 1];
    }
    return 0;
}


/**
* -----------------latest tasks manage-----------------
                      历史任务管理
**/
typedef struct {
    u8 task_tbl[LATEST_TASK_MAX];
    u8 num;
} latest_list_t;

static latest_list_t latest_list;

//最近任务允许记录的id
#define LATEST_ALLOWED_CNT  (sizeof(latest_allowed_id) / sizeof(latest_allowed_id[0]))
const static u8 latest_allowed_id[] = {
    FUNC_CLOCK,
    FUNC_HOME_PAGE,
    FUNC_CHARGE,
};

/**
 * @brief 最近任务列表初始化
 **/
void latest_task_init(void)
{
    latest_list.num = 0;
}

/**
 * @brief 获取记录的最近任务个数
 * @return 最近任务个数
 **/
u8 latest_task_count(void)
{
    return latest_list.num;
}

/**
 * @brief 插入最近任务到列表头部，尾部元素先入先出
          注意：idx=0为最新
 * @param[in] func_sta : 任务id
 **/
void latest_task_add(u8 func_sta)
{
    u8 i;
    u8 id = FUNC_NULL;
    u8 tail = latest_list.num;
    for (i = 0; i < tail; i++) {    //查重
        if (func_sta == latest_list.task_tbl[i]) {
            while (i > 0) {
                latest_list.task_tbl[i] = latest_list.task_tbl[i - 1];
                i--;
            }
            latest_list.task_tbl[0] = func_sta;
            TRACE("%s[%d]\n", __func__, func_sta);
            return;
        }
    }
    for (i = 0; i < LATEST_ALLOWED_CNT; i++) {   //过滤
        if (func_sta == latest_allowed_id[i]) {
            id = func_sta;
            break;
        }
    }
    if (id != FUNC_NULL) {
        if (latest_list.num >= LATEST_TASK_MAX) {  //添加
            tail = LATEST_TASK_MAX - 1;
        }
        for ( ; tail > 0; tail--) {
            latest_list.task_tbl[tail] = latest_list.task_tbl[tail - 1];
        }
        latest_list.task_tbl[0] = func_sta;
        latest_list.num = MIN(latest_list.num + 1, LATEST_TASK_MAX);
#if (TRACE_EN == 0)
        printf("%s[sta:%d num:%d]\n", __func__, func_sta, latest_list.num);
#endif
    }

#if TRACE_EN
    TRACE("%s[%d]:", __func__, func_sta);
    if (latest_list.num) {
        for (i = 0; i < (latest_list.num - 1); i++) {
            TRACE("%d,", latest_list.task_tbl[i]);
        }
        TRACE("%d", latest_list.task_tbl[i]);
    }
    TRACE("\n");
#endif
}

/**
 * @brief 通过下标（0最新）删除最近任务列表中的任务
 * @param[in] idx : 任务在列表中的序号
 **/
void latest_task_del(u8 idx)
{
    u8 i = idx;
    if (i < latest_list.num) {
        latest_list.num--;
        for ( ; i < latest_list.num; i++) {
            latest_list.task_tbl[i] = latest_list.task_tbl[i + 1];
        }
#if (TRACE_EN == 0)
        printf("%s[idx:%d num:%d]\n", __func__, idx, latest_list.num);
#endif
    }

#if TRACE_EN
    TRACE("%s[%d]:", __func__, idx);
    if (latest_list.num) {
        for (i = 0; i < (latest_list.num - 1); i++) {
            TRACE("%d,", latest_list.task_tbl[i]);
        }
        TRACE("%d", latest_list.task_tbl[i]);
    }
    TRACE("\n");
#endif // TRACE_EN
}

/**
 * @brief 通过下标（0最新）获取最近任务列表中的任务id
 * @param[in] idx : 任务在列表中的序号
 * @return 任务idl
 **/
u8 latest_task_get(u8 idx) {
    if (idx < latest_list.num) {
        return latest_list.task_tbl[idx];
    }
    return FUNC_NULL;
}

/**
 * @brief 最近任务列表中查找任务
 * @param[in] func_sta : 任务id
 * @return 任务在列表中的序号（没有则返回-1）
 **/
s8 latest_task_find(u8 func_sta) {
    for (u8 i = 0; i < (latest_list.num); i++) {
        if (latest_list.task_tbl[i] == func_sta) {
            return i;
        }
    }
    return -1;
}
