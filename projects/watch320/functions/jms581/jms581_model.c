/*****************************************************************************
 * Module    : JMS581数据中间层
 * File      : jms581_model.c
 * Function  : 协议回调表的唯一注册者。回调只做"拷贝存下+置valid/推进状态+版本号+1",
 *             不碰界面; 对外只给同步快照getter和版本号, 界面初始化直接读, 每圈比
 *             版本号决定重画 —— 纯轮询, 没有向界面回调, 界面退出无需注销。
 *             另含预取状态机: 脱机模式581上电后拉齐home首屏数据, 期间UI停在开机页。
 *****************************************************************************/
#include "include.h"
#include "func.h"

#if JMS581_EN

#define TRACE_EN                1
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*----------------------------------------------------------------------------
 * 常量与类型
 *--------------------------------------------------------------------------*/
///预取子状态: 串行推进, 同一时刻串口上只允许一条未决请求(否则应答对不上号)
enum {
    PF_IDLE = 0,
    PF_WAIT_BOOT,               //581刚上电, 等启动
    PF_STATUS,                  //拉0x8000 (挡门)
    PF_CAP,                     //拉0x8006 (挡门)
    PF_FW,                      //拉0x8007 (不挡门, home已经放行)
    PF_DONE,
};

///UTF-16LE目录名最大字节数 (对应JMS581_DIR_NAME_MAX-1个ASCII字符)
#define JMS581_DIR_NAME_U16_MAX     ((JMS581_DIR_NAME_MAX - 1) * 2)

typedef struct {
    //---- 缓存: 581说什么就存什么, 不加工 ----
    jms581_dev_status_t sta;
    jms581_capacity_t   cap;
    u8  fw[4];
    u16 idle_sec;

    u8  sta_valid;              //各项"拿到过有效应答"标志, 581断电时清0
    u8  cap_valid;
    u8  fw_valid;
    u8  idle_valid;             //0=还没拿到或581称自己非PC模式

    //---- 流程状态: 瞬时ACK折算成持久状态, 轮询才不丢信息 ----
    jms581_backup_sta_t bk;
    jms581_format_sta_t fmt;

    //---- 0x8008 目录列表 (存ASCII, cursor不外泄) ----
    char dir[JMS581_DIR_CACHE_CNT][JMS581_DIR_NAME_MAX];
    u8   dir_cnt;
    u8   dir_has_more;
    u8   dir_err;

    u32 ver[JMS581_VER_MAX];    //更新计数, 界面比对用

    //---- 预取 ----
    u8  pf_sta;                 //PF_x
    u32 pf_tick;                //本步计时: 等581启动 / 重发超时, 两用
    u8  dev_list_last;          //上次的dev_list, 用于插拔卡时作废容量
} jms581_model_cb_t;

static jms581_model_cb_t model_cb;

static void model_pf_goto(u8 sta);

/*----------------------------------------------------------------------------
 * 调试打印: 收数据的关键点全打, 联调时靠这些日志就能还原581说了什么、状态怎么走的
 *--------------------------------------------------------------------------*/
#if TRACE_EN
static const char *const bk_sta_name[] = {
    "IDLE", "STARTING", "RUNNING", "CANCELING", "DONE", "FAILED", "CANCELLED"
};
static const char *const fmt_sta_name[] = {
    "IDLE", "STARTING", "RUNNING", "DONE", "FAILED"
};
static const char *const size_unit_name[] = {"B", "KB", "MB", "GB"};
static const char *const speed_unit_name[] = {"B/s", "KB/s", "MB/s", "GB/s"};

//协议里单位字段固定4档(0=B 1=K 2=M 3=G), 越界一律打"?"
#define UNIT_STR(tbl, u)        ((u) < 4 ? (tbl)[u] : "?")

//流程状态变化才打, 没变不刷屏
static void model_bk_trace(u8 old)
{
    if (old != model_cb.bk.sta) {
        TRACE("jms581 model: backup %s -> %s (err=0x%02x sub=0x%02x)\n",
              bk_sta_name[old], bk_sta_name[model_cb.bk.sta],
              model_cb.bk.err_code, model_cb.bk.sub_err);
    }
}

static void model_fmt_trace(u8 old)
{
    if (old != model_cb.fmt.sta) {
        TRACE("jms581 model: format %s -> %s (err=0x%02x dev=%d)\n",
              fmt_sta_name[old], fmt_sta_name[model_cb.fmt.sta],
              model_cb.fmt.err_code, model_cb.fmt.dev_id);
    }
}
#else
#define model_bk_trace(old)     (void)(old)
#define model_fmt_trace(old)    (void)(old)
#endif

///流程状态是否"在飞行中": 只有在飞时才允许581的上报推进到终态,
///避免IDLE态收到一帧陈旧应答就凭空变成DONE
static u8 model_bk_inflight(void)
{
    return (model_cb.bk.sta == JMS581_BK_STARTING
            || model_cb.bk.sta == JMS581_BK_RUNNING
            || model_cb.bk.sta == JMS581_BK_CANCELING);
}

/*----------------------------------------------------------------------------
 * 协议回调 (主循环上下文, 由jms581_proto_frame_input分发)
 * 一律只做三件事: 拷贝存下 / 置valid或推进状态 / 版本号+1。不画屏, 不切界面。
 * err_code非0的应答(含11B短ACK)不当有效数据: 字段全是0, 存了就是假数据。
 * 预取期间不置valid即视为"没拿到", 状态机会继续重发。
 *--------------------------------------------------------------------------*/
static void model_dev_status_cb(const jms581_dev_status_t *sta)
{
    TRACE("jms581 model: dev_status err=%d sta=%d dev=0x%x\n",
          sta->err_code, sta->status, sta->dev_list);
    if (sta->err_code != JMS581_ERR_NONE) {
        return;
    }

    //插拔卡: 容量必然变了, 作废并让预取状态机回去重拉一次
    if (model_cb.sta_valid && model_cb.dev_list_last != sta->dev_list
        && model_cb.pf_sta > PF_STATUS) {
        TRACE("jms581 model: dev_list 0x%x->0x%x, cap invalid\n",
              model_cb.dev_list_last, sta->dev_list);
        model_cb.cap_valid = 0;
        model_pf_goto(PF_CAP);                  //退回容量步重拉, 拉到后自己走回PF_DONE
    }

    model_cb.sta           = *sta;
    model_cb.dev_list_last = sta->dev_list;
    model_cb.sta_valid     = 1;
    model_cb.ver[JMS581_VER_STATUS]++;
}

static void model_capacity_cb(const jms581_capacity_t *cap)
{
    TRACE("jms581 model: capacity err=%d slot_num=%d\n", cap->err_code, cap->slot_num);
    if (cap->err_code != JMS581_ERR_NONE || cap->slot_num == 0) {
        return;                                 //短ACK: 槽数组全0, 存下来就是"容量全0"的假数据
    }
#if TRACE_EN
    //槽序固定 M.2/CFA/CFB/SD; total/used协议规定单位KB, raw为原始值低32位, 用来
    //核对581给的到底是不是KB (显示数字对不上时先看这一行)
    for (u8 i = 0; i < 4; i++) {
        const jms581_slot_cap_t *s = &cap->slot[i];

        if (!s->present) {
            continue;
        }
        TRACE("jms581 model:   slot%d err=%d unit=%d total=%uMB used=%uMB uvalid=%d "
              "rawtot=%u rawuse=%u\n", i, s->err_code, s->total_unit,
              (u32)(s->total_size >> 10), (u32)(s->used_size >> 10), s->used_valid,
              (u32)s->total_size, (u32)s->used_size);
    }
#endif
    model_cb.cap       = *cap;
    model_cb.cap_valid = 1;
    model_cb.ver[JMS581_VER_CAP]++;
}

static void model_fw_ver_cb(const jms581_fw_ver_t *ver)
{
    TRACE("jms581 model: fw_ver err=%d %d.%d.%d.%d\n", ver->err_code,
          ver->fw_ver[0], ver->fw_ver[1], ver->fw_ver[2], ver->fw_ver[3]);
    if (ver->err_code != JMS581_ERR_NONE) {
        return;
    }
    memcpy(model_cb.fw, ver->fw_ver, sizeof(model_cb.fw));
    model_cb.fw_valid = 1;
    model_cb.ver[JMS581_VER_FW]++;
}

static void model_pc_idle_cb(const jms581_pc_idle_t *idle)
{
    TRACE("jms581 model: pc_idle err=%d idle=%ds\n", idle->err_code, idle->idle_sec);
    if (idle->err_code == JMS581_ERR_NONE) {
        model_cb.idle_sec   = idle->idle_sec;
        model_cb.idle_valid = 1;
    } else {
        model_cb.idle_valid = 0;                //err=1: 581称自己非PC模式
    }
}

/*---------------------------- 备份流程 (0x8001/0x8002/0x8005) ----------------------------*/
//0x8001 启动ACK: 0=接受 1=未就绪 5=参数错
static void model_backup_ack_cb(u8 err_code)
{
    u8 old = model_cb.bk.sta;

    TRACE("jms581 model: backup_ack err=%d (0=接受 1=未就绪 5=参数错)\n", err_code);
    if (old != JMS581_BK_STARTING) {
        TRACE("jms581 model: backup_ack dropped, sta=%d\n", old);
        return;                                 //没在等ACK, 陈旧应答, 丢弃
    }
    if (err_code == JMS581_ERR_NONE) {
        model_cb.bk.sta = JMS581_BK_RUNNING;    //581接受了, 之后靠0x8002推进度
    } else {
        model_cb.bk.sta      = JMS581_BK_FAILED;
        model_cb.bk.err_code = err_code;        //启动被拒
    }
    model_bk_trace(old);
    model_cb.ver[JMS581_VER_BACKUP]++;
}

//0x8002 备份报告: 应答+备份中主动上报共用
static void model_backup_report_cb(const jms581_backup_report_t *rpt)
{
    u8 terminal;
    u8 old = model_cb.bk.sta;

    TRACE("jms581 model: backup_report err=0x%02x sub=0x%02x ext=%d phase=%d prog=%d%%\n",
          rpt->err_code, rpt->sub_err, rpt->is_ext, rpt->phase, rpt->progress);
    TRACE("jms581 model:   file=%u folder=%u total=%u%s speed=%u%s time=%us dev %d->%d\n",
          rpt->file_done_cnt, rpt->folder_done_cnt,
          rpt->total_size, UNIT_STR(size_unit_name, rpt->total_unit),
          rpt->speed_val, UNIT_STR(speed_unit_name, rpt->speed_unit),
          rpt->time_sec, rpt->src_dev, rpt->dst_dev);

    //数据字段无条件更新: 报告内容本身总是有用的, 与状态机是否采纳无关
    model_cb.bk.err_code        = rpt->err_code;
    model_cb.bk.sub_err         = rpt->sub_err;
    model_cb.bk.has_ext         = rpt->is_ext;
    model_cb.bk.file_done_cnt   = rpt->file_done_cnt;
    model_cb.bk.folder_done_cnt = rpt->folder_done_cnt;
    model_cb.bk.total_unit      = rpt->total_unit;
    model_cb.bk.total_size      = rpt->total_size;
    model_cb.bk.speed_unit      = rpt->speed_unit;
    model_cb.bk.speed_val       = rpt->speed_val;
    model_cb.bk.time_sec        = rpt->time_sec;
    model_cb.bk.src_dev         = rpt->src_dev;
    model_cb.bk.dst_dev         = rpt->dst_dev;
    if (rpt->is_ext) {
        model_cb.bk.phase    = rpt->phase;
        model_cb.bk.progress = rpt->progress;
    }
    //l3_name指向帧缓冲, 回调返回即失效, 必须在这里拷走; 顺便转成界面直接能用的ASCII
    if (rpt->l3_name_len) {
        u16 nlen = rpt->l3_name_len;

        if (nlen > JMS581_DIR_NAME_U16_MAX) {
            nlen = JMS581_DIR_NAME_U16_MAX;
        }
        jms581_utf16le_to_ascii(rpt->l3_name, (u8)nlen,
                                model_cb.bk.l3_name, JMS581_DIR_NAME_MAX);
        TRACE("jms581 model:   l3_name(%dB) = \"%s\"\n",
              rpt->l3_name_len, model_cb.bk.l3_name);
    }

    //--- 状态机推进 ---
    //协议没给"这帧是11B短ACK还是完整报告"的标志, 而短ACK的字段全是0, 看起来
    //和"err=0且phase=0的终态报告"一模一样。所以终态只在以下情况成立:
    //  err_code带终态语义(0x09失败/0x0B取消), 或
    //  315扩展明确给了phase=0, 或
    //  309报告里有实打实的统计数据(不可能是全0的短ACK)
    terminal = (rpt->err_code == JMS581_ERR_BACKUP_FAILED
                || rpt->err_code == JMS581_ERR_BACKUP_CANCELLED
                || (rpt->is_ext && rpt->phase == 0)
                || (!rpt->is_ext && (rpt->file_done_cnt || rpt->folder_done_cnt
                                     || rpt->total_size || rpt->time_sec)));

    if (model_bk_inflight()) {
        if (rpt->err_code == JMS581_ERR_BACKUP_CANCELLED) {
            model_cb.bk.sta = JMS581_BK_CANCELLED;
        } else if (terminal) {
            model_cb.bk.sta = (rpt->err_code == JMS581_ERR_NONE)
                            ? JMS581_BK_DONE : JMS581_BK_FAILED;
        } else if (model_cb.bk.sta == JMS581_BK_STARTING) {
            model_cb.bk.sta = JMS581_BK_RUNNING;    //进度先于ACK到达也算已开跑
        }
    } else if (model_cb.bk.sta == JMS581_BK_IDLE && rpt->is_ext && rpt->phase != 0) {
        model_cb.bk.sta = JMS581_BK_RUNNING;        //防御: 581自称在跑, 采纳
    }

    model_bk_trace(old);
    model_cb.ver[JMS581_VER_BACKUP]++;
}

//0x8005 取消ACK: 0=已接受 1=无可取消 0x0C=收尾阶段拒绝
static void model_cancel_ack_cb(u8 err_code)
{
    u8 old = model_cb.bk.sta;

    TRACE("jms581 model: cancel_ack err=0x%02x (0=已接受 1=无可取消 0x0C=收尾拒绝)\n",
          err_code);
    model_cb.bk.err_code = err_code;
    //ACK=0只代表"取消请求被接受", 不代表已停止: 真终态要等0x8002推err=0x0B。
    //ACK=1/0x0C不改状态, 只把码留在err_code里, 界面据此弹提示。
    if (err_code == JMS581_ERR_NONE && model_cb.bk.sta == JMS581_BK_RUNNING) {
        model_cb.bk.sta = JMS581_BK_CANCELING;
    }
    model_bk_trace(old);
    model_cb.ver[JMS581_VER_BACKUP]++;
}

/*---------------------------- 格式化流程 (0x8003) ----------------------------*/
//0x8003 11B启动ACK
static void model_format_ack_cb(u8 err_code)
{
    u8 old = model_cb.fmt.sta;

    TRACE("jms581 model: format_ack err=0x%02x\n", err_code);
    if (old != JMS581_FMT_STARTING) {
        TRACE("jms581 model: format_ack dropped, sta=%d\n", old);
        return;
    }
    if (err_code == JMS581_ERR_NONE) {
        model_cb.fmt.sta = JMS581_FMT_RUNNING;
    } else {
        model_cb.fmt.sta      = JMS581_FMT_FAILED;
        model_cb.fmt.err_code = err_code;
    }
    model_fmt_trace(old);
    model_cb.ver[JMS581_VER_FORMAT]++;
}

//0x8003 15B进度/终态 (多为主动上报): phase 0=终态 1=进行中; result 0=成功 1=失败
static void model_format_status_cb(const jms581_format_status_t *sta)
{
    u8 old = model_cb.fmt.sta;

    TRACE("jms581 model: format_status err=0x%02x phase=%d(0=终态 1=进行中) prog=%d%% "
          "result=%d(0=成功) dev=%d\n",
          sta->err_code, sta->phase, sta->progress, sta->result, sta->dev_id);

    model_cb.fmt.dev_id = sta->dev_id;
    if (sta->err_code != JMS581_ERR_NONE) {
        model_cb.fmt.err_code = sta->err_code;
    }

    if (sta->phase) {                           //进行中
        model_cb.fmt.progress = sta->progress;
        if (model_cb.fmt.sta == JMS581_FMT_IDLE
            || model_cb.fmt.sta == JMS581_FMT_STARTING) {
            model_cb.fmt.sta = JMS581_FMT_RUNNING;
        }
    } else if (model_cb.fmt.sta != JMS581_FMT_IDLE) {   //终态, 且确实在飞
        if (sta->result == 0 && sta->err_code == JMS581_ERR_NONE) {
            model_cb.fmt.sta      = JMS581_FMT_DONE;
            model_cb.fmt.progress = 100;
        } else {
            model_cb.fmt.sta = JMS581_FMT_FAILED;
        }
    }
    model_fmt_trace(old);
    model_cb.ver[JMS581_VER_FORMAT]++;
}

/*---------------------------- 目录列表 (0x8008) ----------------------------*/
static void model_dir_list_cb(const jms581_dir_list_t *info,
                              const jms581_dir_entry_t *entries, u8 cnt)
{
    u8 i, n;

    TRACE("jms581 model: dir_list err=0x%02x return_cnt=%d parsed=%d has_more=%d\n",
          info->err_code, info->return_count, cnt, info->has_more);

    model_cb.dir_err = info->err_code;
    if (info->err_code != JMS581_ERR_NONE) {
        model_cb.dir_cnt      = 0;
        model_cb.dir_has_more = 0;
        model_cb.ver[JMS581_VER_DIR]++;
        return;
    }

    n = (cnt > JMS581_DIR_CACHE_CNT) ? JMS581_DIR_CACHE_CNT : cnt;
    for (i = 0; i < n; i++) {
        u16 nlen = entries[i].name_len;          //指向帧缓冲, 必须在本函数内拷走

        if (nlen > JMS581_DIR_NAME_U16_MAX) {
            nlen = JMS581_DIR_NAME_U16_MAX;      //钳到缓存容量, 顺便保证u8转换安全
        }
        jms581_utf16le_to_ascii(entries[i].name, (u8)nlen,
                                model_cb.dir[i], JMS581_DIR_NAME_MAX);
        TRACE("jms581 model:   dir[%d] type=%d nlen=%dB \"%s\"\n",
              i, entries[i].file_type, entries[i].name_len, model_cb.dir[i]);
    }
    model_cb.dir_cnt = n;
    if (cnt > n) {
        TRACE("jms581 model:   %d entries dropped (cache=%d)\n",
              cnt - n, JMS581_DIR_CACHE_CNT);
    }
    //581说还有后续, 或它给的条数超出本层缓存, 对界面都是"还有更多没显示"
    //TODO: 要做SEQ模式cursor续页时, 在此保存info->next_cursor并加_dir_more()接口
    model_cb.dir_has_more = (info->has_more || cnt > n) ? 1 : 0;
    model_cb.ver[JMS581_VER_DIR]++;
}

static void model_unknown_cb(u8 cmd, u8 sub_cmd, const u8 *payload, u16 payload_len)
{
    (void)payload;
    TRACE("jms581 model: unknown cmd=0x%02x sub=0x%02x len=%d\n", cmd, sub_cmd, payload_len);
}

//全工程唯一一张回调表
static const jms581_cb_t jms581_model_cbs = {
    .dev_status    = model_dev_status_cb,
    .backup_ack    = model_backup_ack_cb,
    .backup_report = model_backup_report_cb,
    .format_ack    = model_format_ack_cb,
    .format_status = model_format_status_cb,
    .pc_idle       = model_pc_idle_cb,
    .cancel_ack    = model_cancel_ack_cb,
    .capacity      = model_capacity_cb,
    .fw_ver        = model_fw_ver_cb,
    .dir_list      = model_dir_list_cb,
    .unknown       = model_unknown_cb,
};

/*----------------------------------------------------------------------------
 * 同步快照: 界面初始化直接调, 立刻返回, 永不阻塞
 *--------------------------------------------------------------------------*/
u8 jms581_model_dev_status(jms581_dev_status_t *out)
{
    if (!model_cb.sta_valid) {
        return 0;
    }
    *out = model_cb.sta;
    return 1;
}

u8 jms581_model_capacity(jms581_capacity_t *out)
{
    if (!model_cb.cap_valid) {
        return 0;
    }
    *out = model_cb.cap;
    return 1;
}

u8 jms581_model_fw_ver(u8 ver[4])
{
    if (!model_cb.fw_valid) {
        return 0;
    }
    memcpy(ver, model_cb.fw, sizeof(model_cb.fw));
    return 1;
}

u8 jms581_model_pc_idle(u16 *idle_sec)
{
    if (!model_cb.idle_valid) {
        return 0;
    }
    *idle_sec = model_cb.idle_sec;
    return 1;
}

//流程快照返回状态值本身: IDLE也是有效状态, 没有"无效"这回事
u8 jms581_model_backup_sta(jms581_backup_sta_t *out)
{
    if (out) {
        *out = model_cb.bk;
    }
    return model_cb.bk.sta;
}

u8 jms581_model_format_sta(jms581_format_sta_t *out)
{
    if (out) {
        *out = model_cb.fmt;
    }
    return model_cb.fmt.sta;
}

u8 jms581_model_dir_count(void)
{
    return model_cb.dir_cnt;
}

const char *jms581_model_dir_name(u8 idx)
{
    if (idx >= model_cb.dir_cnt) {
        return NULL;
    }
    return model_cb.dir[idx];
}

u8 jms581_model_dir_has_more(void)
{
    return model_cb.dir_has_more;
}

u8 jms581_model_dir_err(void)
{
    return model_cb.dir_err;
}

u32 jms581_model_ver(u8 item)
{
    if (item >= JMS581_VER_MAX) {
        return 0;
    }
    return model_cb.ver[item];
}

/*----------------------------------------------------------------------------
 * 请求发送: 发出去的同时把流程状态推到"等应答", 界面才分得清"还没发"和"发了在等"
 *--------------------------------------------------------------------------*/
u8 jms581_model_backup_start(u8 src_dev, u8 dst_dev, u8 mode,
                             const char *name, u8 days)
{
    u8 ok;

    if (model_bk_inflight()) {
        TRACE("jms581 model: backup already inflight (sta=%d)\n", model_cb.bk.sta);
        return 0;
    }

    memset(&model_cb.bk, 0, sizeof(model_cb.bk));   //新一轮备份, 旧报告全部清掉
    model_cb.bk.src_dev = src_dev;
    model_cb.bk.dst_dev = dst_dev;

    ok = name ? jms581_backup_start_ascii_req(src_dev, dst_dev, mode, name, days)
              : jms581_backup_start_req(src_dev, dst_dev, mode);
    //状态在发送结果确定之后才置: 发失败就留在IDLE, 界面不会误等一个永不到来的ACK
    model_cb.bk.sta = ok ? JMS581_BK_STARTING : JMS581_BK_IDLE;
    model_cb.ver[JMS581_VER_BACKUP]++;
    TRACE("jms581 model: backup_start %d->%d mode=%d ok=%d\n", src_dev, dst_dev, mode, ok);
    return ok;
}

u8 jms581_model_backup_cancel(void)
{
    if (model_cb.bk.sta != JMS581_BK_RUNNING) {
        TRACE("jms581 model: cancel ignored, sta=%d\n", model_cb.bk.sta);
        return 0;                               //没在跑/已在取消/已终态, 没什么可取消
    }
    {
        u8 ok = jms581_backup_cancel_req();     //状态由0x8005 ACK推进, 这里不预判

        TRACE("jms581 model: backup_cancel req ok=%d\n", ok);
        return ok;
    }
}

u8 jms581_model_format_start(u8 dev_id)
{
    u8 ok;

    if (model_cb.fmt.sta == JMS581_FMT_STARTING
        || model_cb.fmt.sta == JMS581_FMT_RUNNING) {
        TRACE("jms581 model: format already inflight (sta=%d)\n", model_cb.fmt.sta);
        return 0;
    }

    memset(&model_cb.fmt, 0, sizeof(model_cb.fmt));
    model_cb.fmt.dev_id = dev_id;
    ok = jms581_format_start_req(dev_id);
    model_cb.fmt.sta = ok ? JMS581_FMT_STARTING : JMS581_FMT_IDLE;
    model_cb.ver[JMS581_VER_FORMAT]++;
    TRACE("jms581 model: format_start dev=%d ok=%d\n", dev_id, ok);
    return ok;
}

u8 jms581_model_dir_req(u8 root_type)
{
    //Top-N: 581按编号从大到小排, 直接给最新的一批, 正好对上界面"从新到旧"的展示;
    //一次拉满缓存就不需要翻页 —— SEQ模式的cursor只能往前, 翻回去没数据
    u8 ok = jms581_dir_list_req(root_type, JMS581_DIR_CACHE_CNT,
                                JMS581_LIST_MODE_TOPN, NULL);

    TRACE("jms581 model: dir_req root=%d count=%d ok=%d\n",
          root_type, JMS581_DIR_CACHE_CNT, ok);
    return ok;
}

/*----------------------------------------------------------------------------
 * 预取: 581上电后主动拉齐home首屏要的数据
 *
 *   PF_WAIT_BOOT --200ms--> PF_STATUS(0x8000) --> PF_CAP(0x8006) --> 门开
 *                                                                --> PF_FW(0x8007) --> PF_DONE
 *
 * 每步500ms没回就重发, 无限重试: 丢一帧就永久卡在开机页等于变砖。
 * 581始终不回 = 产品异常, 设计上就卡在开机页; 长按关机(jms581_mode.c)和10分钟
 * 无操作关机照常生效, 用户按得出去, 电池也不会耗干。
 *--------------------------------------------------------------------------*/
//进下一步并立刻发出该步的请求
static void model_pf_goto(u8 sta)
{
    model_cb.pf_sta  = sta;
    model_cb.pf_tick = tick_get();

    switch (sta) {
    case PF_STATUS:
        TRACE("jms581 model: prefetch status req\n");
        jms581_dev_status_req();
        break;
    case PF_CAP:
        TRACE("jms581 model: prefetch cap req (gate closed)\n");
        jms581_capacity_req();
        break;
    case PF_FW:
        TRACE("jms581 model: gate open, prefetch fw req\n");
        jms581_fw_ver_req();                    //不挡门: home此时已放行
        break;
    case PF_DONE:
        TRACE("jms581 model: prefetch done\n");
        break;
    default:
        break;
    }
}

//581断电或刚上电: 所有缓存作废。流程状态一并清掉 —— 581没电, 备份/格式化必然中断
static void model_cache_drop(void)
{
    TRACE("jms581 model: cache drop (bk=%d fmt=%d dir=%d)\n",
          model_cb.bk.sta, model_cb.fmt.sta, model_cb.dir_cnt);
    model_cb.sta_valid  = 0;
    model_cb.cap_valid  = 0;
    model_cb.fw_valid   = 0;
    model_cb.idle_valid = 0;

    memset(&model_cb.bk, 0, sizeof(model_cb.bk));       //回IDLE
    memset(&model_cb.fmt, 0, sizeof(model_cb.fmt));     //回IDLE
    model_cb.dir_cnt      = 0;
    model_cb.dir_has_more = 0;
    model_cb.dir_err      = 0;

    //版本号一律递增: 界面比对时才知道"数据没了", 否则会拿着上一轮的旧值不刷
    for (u8 i = 0; i < JMS581_VER_MAX; i++) {
        model_cb.ver[i]++;
    }
}

void jms581_model_prefetch_start(void)
{
    TRACE("jms581 model: prefetch start\n");
    model_cache_drop();                         //581刚上电, 之前的全是上一轮的旧数据
    model_cb.pf_sta  = PF_WAIT_BOOT;
    model_cb.pf_tick = tick_get();
}

void jms581_model_prefetch_abort(void)
{
    if (model_cb.pf_sta != PF_IDLE) {
        TRACE("jms581 model: prefetch abort\n");
    }
    model_cache_drop();
    model_cb.pf_sta = PF_IDLE;
}

//挡门条件 = home首屏实际要画的两项, 不多不少
u8 jms581_model_gate_ready(void)
{
    return (model_cb.sta_valid && model_cb.cap_valid);
}

void jms581_model_process(void)
{
    switch (model_cb.pf_sta) {
    case PF_WAIT_BOOT:
        if (tick_check_expire(model_cb.pf_tick, JMS581_BOOT_WAIT_MS)) {
            model_pf_goto(PF_STATUS);
        }
        break;

    case PF_STATUS:
        if (model_cb.sta_valid) {
            model_pf_goto(PF_CAP);
        } else if (tick_check_expire(model_cb.pf_tick, JMS581_PREFETCH_RETRY_MS)) {
            TRACE("jms581 model: status retry\n");
            model_pf_goto(PF_STATUS);
        }
        break;

    case PF_CAP:
        if (model_cb.cap_valid) {               //此后gate_ready()=1, 开机页放行
            //fw是静态值, 已经有就不再问(插拔卡退回PF_CAP重拉时会走到这)
            model_pf_goto(model_cb.fw_valid ? PF_DONE : PF_FW);
        } else if (tick_check_expire(model_cb.pf_tick, JMS581_PREFETCH_RETRY_MS)) {
            TRACE("jms581 model: cap retry\n");
            model_pf_goto(PF_CAP);
        }
        break;

    case PF_FW:
        if (model_cb.fw_valid) {
            model_pf_goto(PF_DONE);
        } else if (tick_check_expire(model_cb.pf_tick, JMS581_PREFETCH_RETRY_MS)) {
            TRACE("jms581 model: fw retry\n");
            model_pf_goto(PF_FW);
        }
        break;

    default:                                    //PF_IDLE / PF_DONE: 无事
        break;
    }
}

/*----------------------------------------------------------------------------
 * 初始化
 *--------------------------------------------------------------------------*/
void jms581_model_init(void)
{
    memset(&model_cb, 0, sizeof(model_cb));
    jms581_proto_cb_reg(&jms581_model_cbs);     //全工程唯一一处cb_reg
    TRACE("jms581 model: init\n");
}

#endif // JMS581_EN
