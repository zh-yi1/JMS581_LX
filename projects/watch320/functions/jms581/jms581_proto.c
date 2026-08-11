#include "include.h"

#if JMS581_EN

#define TRACE_EN 1
#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

static const jms581_cb_t *jms581_cbs;                        // 回调表指针, NULL=未注册
static jms581_dir_entry_t dir_entries[JMS581_DIR_ENTRY_MAX]; // 0x8008 单页条目解析缓存

void jms581_proto_cb_reg(const jms581_cb_t *cb)
{
    jms581_cbs = cb;
}

// payload多字节字段常在非对齐偏移, 逐字节拼接取值 (macro.h的GET_LE16/32是裸指针强转, 不安全)
static u16 pget_le16(const u8 *p)
{
    return (u16)p[0] | ((u16)p[1] << 8);
}

static u32 pget_le32(const u8 *p)
{
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}

static u64 pget_le64(const u8 *p)
{
    return (u64)pget_le32(p) | ((u64)pget_le32(p + 4) << 32);
}

///---------------------------------------- 请求发送API ----------------------------------------

// 0x8000 查询设备状态 (10B请求, 应答13B: status/dev_list)
u8 jms581_dev_status_req(void)
{
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_DEV_STATUS, NULL, 0);
}

// 0x8001 开始备份-短帧 (13B请求, 309兼容, L2目录名由581自管)
u8 jms581_backup_start_req(u8 src_dev, u8 dst_dev, u8 mode)
{
    u8 pl[3];

    if (src_dev > JMS581_DEV_MAX || dst_dev > JMS581_DEV_MAX || src_dev == dst_dev)
    {
        TRACE("backup_start_req: invalid dev %d -> %d, mode %d\n", src_dev, dst_dev, mode);
        return 0;
    }
    pl[0] = src_dev;
    pl[1] = dst_dev;
    pl[2] = mode;
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_BACKUP_START, pl, sizeof(pl));
}

// 0x8001 开始备份-长帧 (315扩展: 携带L2目录名target_folder_name, mode=3时带days)
u8 jms581_backup_start_ex_req(u8 src_dev, u8 dst_dev, u8 mode,
                              const u8 *name_utf16, u8 name_len, u8 days)
{
    u8 pl[4 + JMS581_NAME_LEN_MAX + 1];
    u8 pos;

    if (src_dev > JMS581_DEV_MAX || dst_dev > JMS581_DEV_MAX || src_dev == dst_dev)
    {
        return 0;
    }
    if (!name_utf16 || name_len < 2 || name_len > JMS581_NAME_LEN_MAX || (name_len & 1))
    {
        return 0; // 长帧target_folder_name必填, 2~64偶数
    }
    if (mode == JMS581_MODE_RECENT && (days < 1 || days > 31))
    {
        return 0; // mode=3须携带days 1~31
    }
    pl[0] = src_dev;
    pl[1] = dst_dev;
    pl[2] = mode;
    pl[3] = name_len;
    memcpy(&pl[4], name_utf16, name_len);
    pos = 4 + name_len;
    if (mode == JMS581_MODE_RECENT)
    {
        pl[pos++] = days;
    }
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_BACKUP_START, pl, pos);
}

// 0x8001 开始备份-长帧便捷包装 (ASCII目录名内部转UTF-16LE)
u8 jms581_backup_start_ascii_req(u8 src_dev, u8 dst_dev, u8 mode, const char *name, u8 days)
{
    u8 name_utf16[JMS581_NAME_LEN_MAX];
    u8 name_len = jms581_ascii_to_utf16le(name, name_utf16, sizeof(name_utf16));

    if (!name_len)
    {
        return 0;
    }
    return jms581_backup_start_ex_req(src_dev, dst_dev, mode, name_utf16, name_len, days);
}

// 0x8002 查询备份报告 (10B请求, 应答36B/≥39B; 备份中581也主动推同结构包)
u8 jms581_backup_report_req(void)
{
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_BACKUP_REPORT, NULL, 0);
}

// 0x8003 开始格式化 (11B请求, 应答11B短ACK; 进度/终态由581主动推15B包)
u8 jms581_format_start_req(u8 dev_id)
{
    if (dev_id > JMS581_DEV_MAX)
    {
        return 0;
    }
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_FORMAT, &dev_id, 1);
}

// 0x8003 被动查询格式化状态 (10B请求, 应答15B状态包)
u8 jms581_format_status_req(void)
{
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_FORMAT, NULL, 0);
}

// 0x8004 查询PC端无读写空闲时长 (10B请求, 应答13B)
u8 jms581_pc_idle_req(void)
{
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_PC_IDLE, NULL, 0);
}

// 0x8005 取消备份 (10B请求, 应答11B短ACK; 终态经0x8002推err_code=0x0B)
u8 jms581_backup_cancel_req(void)
{
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_BACKUP_CANCEL, NULL, 0);
}

// 0x8006 查询存储槽容量 (10B请求, 应答92B四槽: M.2/CFA/CFB/SD)
u8 jms581_capacity_req(void)
{
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_CAPACITY, NULL, 0);
}

// 0x8007 查询581固件版本号 (10B请求, 应答15B: A.B.C.D四段)
u8 jms581_fw_ver_req(void)
{
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_FW_VER, NULL, 0);
}

// 0x8008 列出备份目录列表 (20B请求, cursor续读分页, 应答变长)
u8 jms581_dir_list_req(u8 root_type, u8 count, const u8 *cursor)
{
    u8 pl[4 + JMS581_CURSOR_LEN];

    if (root_type > JMS581_ROOT_RECENT_BACKUP || count == 0)
    {
        return 0;
    }
    if (count > JMS581_DIR_ENTRY_MAX)
    {
        count = JMS581_DIR_ENTRY_MAX; // 钳到单页解析上限, 防条目静默丢弃
    }
    pl[0] = root_type;
    pl[1] = count;
    pl[2] = cursor ? 0x01 : 0x00; // flags.bit0: 0=首页 1=续页
    pl[3] = 0;                    // reserved
    if (cursor)
    {
        memcpy(&pl[4], cursor, JMS581_CURSOR_LEN);
    }
    else
    {
        memset(&pl[4], 0, JMS581_CURSOR_LEN); // 首页cursor全0
    }
    return jms581_frame_tx(JMS581_CMD_STORAGE, JMS581_SUB_DIR_LIST, pl, sizeof(pl));
}

///---------------------------------------- 应答/上报解析 ----------------------------------------

// 0x8000: 3B状态包 / 1B短ACK
static void parse_dev_status(const u8 *p, u16 len)
{
    jms581_dev_status_t sta;

    memset(&sta, 0, sizeof(sta));
    sta.err_code = p[0];
    if (len == 3)
    {
        sta.status = p[1];
        sta.dev_list = p[2];
    }
    else if (len != 1)
    {
        TRACE("[jms581] 0x8000 bad len %d\n", len);
        return;
    }
    if (jms581_cbs && jms581_cbs->dev_status)
    {
        jms581_cbs->dev_status(&sta);
    }
}

// 0x8001: 1B启动ACK (0x00=已接受 0x01=未就绪/忙 0x05=参数错)
static void parse_backup_ack(const u8 *p, u16 len)
{
    if (len != 1)
    {
        TRACE("[jms581] 0x8001 bad len %d\n", len);
        return;
    }
    if (jms581_cbs && jms581_cbs->backup_ack)
    {
        jms581_cbs->backup_ack(p[0]);
    }
}

// 0x8002: 26B=309报告 / >=29B=315扩展(含变长l3_name) / 1B短ACK
static void parse_backup_report(const u8 *p, u16 len)
{
    jms581_backup_report_t rpt;

    memset(&rpt, 0, sizeof(rpt));
    rpt.err_code = p[0];
    if (len >= 26)
    {
        rpt.file_done_cnt = pget_le32(&p[1]);
        rpt.folder_done_cnt = pget_le32(&p[5]);
        rpt.total_unit = p[9];
        rpt.total_size = pget_le32(&p[10]);
        rpt.speed_unit = p[14];
        rpt.speed_val = pget_le32(&p[15]);
        rpt.time_sec = pget_le32(&p[19]);
        rpt.sub_err = p[23];
        rpt.src_dev = p[24];
        rpt.dst_dev = p[25];
    }
    if (len >= 29)
    { // 315扩展: phase/progress/l3_name
        rpt.is_ext = 1;
        rpt.phase = p[26];
        rpt.progress = p[27];
        u8 l3_len = p[28];
        if ((l3_len & 1) || l3_len > JMS581_NAME_LEN_MAX || (29u + l3_len) > len)
        {
            TRACE("[jms581] 0x8002 bad l3_name_len %d\n", l3_len); // 畸形l3按无名交付
        }
        else if (l3_len)
        {
            rpt.l3_name_len = l3_len;
            rpt.l3_name = &p[29];
        }
    }
    else if (len != 26 && len != 1)
    {
        TRACE("[jms581] 0x8002 bad len %d\n", len);
        return;
    }
    if (jms581_cbs && jms581_cbs->backup_report)
    {
        jms581_cbs->backup_report(&rpt);
    }
}

// 0x8003: 1B启动ACK / 5B进度终态状态包
static void parse_format(const u8 *p, u16 len)
{
    if (len == 1)
    {
        if (jms581_cbs && jms581_cbs->format_ack)
        {
            jms581_cbs->format_ack(p[0]);
        }
    }
    else if (len == 5)
    {
        jms581_format_status_t sta;
        sta.err_code = p[0];
        sta.phase = p[1];
        sta.progress = p[2];
        sta.result = p[3];
        sta.dev_id = p[4];
        if (jms581_cbs && jms581_cbs->format_status)
        {
            jms581_cbs->format_status(&sta);
        }
    }
    else
    {
        TRACE("[jms581] 0x8003 bad len %d\n", len);
    }
}

// 0x8004: 3B / 1B短ACK
static void parse_pc_idle(const u8 *p, u16 len)
{
    jms581_pc_idle_t idle;

    memset(&idle, 0, sizeof(idle));
    idle.err_code = p[0];
    if (len == 3)
    {
        idle.idle_sec = pget_le16(&p[1]);
    }
    else if (len != 1)
    {
        TRACE("[jms581] 0x8004 bad len %d\n", len);
        return;
    }
    if (jms581_cbs && jms581_cbs->pc_idle)
    {
        jms581_cbs->pc_idle(&idle);
    }
}

// 0x8005: 1B取消ACK (0x00=已接受 0x01=无可取消 0x0C=收尾阶段拒绝)
static void parse_cancel_ack(const u8 *p, u16 len)
{
    if (len != 1)
    {
        TRACE("[jms581] 0x8005 bad len %d\n", len);
        return;
    }
    if (jms581_cbs && jms581_cbs->cancel_ack)
    {
        jms581_cbs->cancel_ack(p[0]);
    }
}

// 0x8006: 82B四槽容量 / 1B短ACK
static void parse_capacity(const u8 *p, u16 len)
{
    jms581_capacity_t cap;

    memset(&cap, 0, sizeof(cap));
    cap.err_code = p[0];
    if (len == 82)
    {
        cap.slot_num = p[1];
        for (u8 i = 0; i < 4; i++)
        {
            const u8 *s = &p[2 + 20 * i]; // 槽序固定 M.2/CFA/CFB/SD
            cap.slot[i].present = s[0];
            cap.slot[i].err_code = s[1];
            cap.slot[i].total_unit = s[2];
            cap.slot[i].total_size = pget_le64(&s[3]);
            cap.slot[i].used_valid = s[11];
            cap.slot[i].used_size = pget_le64(&s[12]);
        }
    }
    else if (len != 1)
    {
        TRACE("[jms581] 0x8006 bad len %d\n", len);
        return;
    }
    if (jms581_cbs && jms581_cbs->capacity)
    {
        jms581_cbs->capacity(&cap);
    }
}

// 0x8007: 5B版本包 / 1B短ACK
static void parse_fw_ver(const u8 *p, u16 len)
{
    jms581_fw_ver_t ver;

    memset(&ver, 0, sizeof(ver));
    ver.err_code = p[0];
    if (len == 5)
    {
        memcpy(ver.fw_ver, &p[1], 4);
    }
    else if (len != 1)
    {
        TRACE("[jms581] 0x8007 bad len %d\n", len);
        return;
    }
    if (jms581_cbs && jms581_cbs->fw_ver)
    {
        jms581_cbs->fw_ver(&ver);
    }
}

// 0x8008: >=10B页头+变长条目 / 1B短ACK
static void parse_dir_list(const u8 *p, u16 len)
{
    jms581_dir_list_t info;
    u8 cnt = 0;

    memset(&info, 0, sizeof(info));
    info.err_code = p[0];
    if (len >= 10)
    {
        u8 return_count = p[1];
        info.has_more = p[2] & 0x01;
        memcpy(info.next_cursor, &p[4], JMS581_CURSOR_LEN);
        u16 pos = 10; // 条目区起点
        while (cnt < return_count && cnt < JMS581_DIR_ENTRY_MAX && (pos + 3u) <= len)
        {
            u16 nlen = pget_le16(&p[pos + 1]);
            if ((nlen & 1) || nlen == 0 || (pos + 3u + nlen) > len)
            {
                TRACE("[jms581] 0x8008 bad entry@%d nlen %d\n", pos, nlen);
                break; // 畸形条目, 截断交付
            }
            dir_entries[cnt].file_type = p[pos];
            dir_entries[cnt].name_len = nlen;
            dir_entries[cnt].name = &p[pos + 3];
            pos += 3 + nlen;
            cnt++;
        }
        info.return_count = cnt; // 以实际解析出的条数为准
    }
    else if (len != 1)
    {
        TRACE("[jms581] 0x8008 bad len %d\n", len);
        return;
    }
    if (jms581_cbs && jms581_cbs->dir_list)
    {
        jms581_cbs->dir_list(&info, cnt ? dir_entries : NULL, cnt);
    }
}

// 帧层唯一上交入口 (完整合法帧, direction=0x00, 主循环上下文)
void jms581_proto_frame_input(u8 cmd, u8 sub_cmd, const u8 *payload, u16 len)
{
    // ZH TODO: 确定指令是否至少包含一条数据
    if (cmd == JMS581_CMD_STORAGE && len >= 1)
    { // 应答体至少含err_code
        switch (sub_cmd)
        {
        case JMS581_SUB_DEV_STATUS:
            parse_dev_status(payload, len);
            return;
        case JMS581_SUB_BACKUP_START:
            parse_backup_ack(payload, len);
            return;
        case JMS581_SUB_BACKUP_REPORT:
            parse_backup_report(payload, len);
            return;
        case JMS581_SUB_FORMAT:
            parse_format(payload, len);
            return;
        case JMS581_SUB_PC_IDLE:
            parse_pc_idle(payload, len);
            return;
        case JMS581_SUB_BACKUP_CANCEL:
            parse_cancel_ack(payload, len);
            return;
        case JMS581_SUB_CAPACITY:
            parse_capacity(payload, len);
            return;
        case JMS581_SUB_FW_VER:
            parse_fw_ver(payload, len);
            return;
        case JMS581_SUB_DIR_LIST:
            parse_dir_list(payload, len);
            return;
        default:
            break;
        }
    }
    // 未实现命令兜底 (0x81xx升级组等, 后续扩展在此加分支)
    if (jms581_cbs && jms581_cbs->unknown)
    {
        jms581_cbs->unknown(cmd, sub_cmd, payload, len);
    }
    else
    {
        TRACE("[jms581] unknown cmd %02x%02x len %d\n", cmd, sub_cmd, len);
    }
}

///---------------------------------------- UTF-16LE辅助 ----------------------------------------

u8 jms581_ascii_to_utf16le(const char *ascii, u8 *out, u8 out_size)
{
    u8 n = 0;

    if (!ascii || !out)
    {
        return 0;
    }
    while (*ascii)
    {
        if (n + 2 > out_size)
        {
            return 0; // 容量不足视为失败, 不交付截断名
        }
        out[n++] = (u8)*ascii++;
        out[n++] = 0;
    }
    return n;
}

u8 jms581_utf16le_to_ascii(const u8 *utf16, u8 utf16_len, char *out, u8 out_size)
{
    u8 cnt = 0;

    if (!out || !out_size)
    {
        return 0;
    }
    if (utf16)
    {
        for (u8 i = 0; i + 1 < utf16_len; i += 2)
        {
            if (cnt + 1 >= out_size)
            {
                break;
            }
            u16 code = pget_le16(&utf16[i]);
            out[cnt++] = (code && code <= 0x7F) ? (char)code : '?';
        }
    }
    out[cnt] = '\0';
    return cnt;
}

///---------------------------------------- 注入测试 ----------------------------------------
#if JMS581_TEST_EN

extern void bsp_uart1_isr(uint8_t *buf, uint32_t len); // 驱动层接收ISR, 测试借用注入环形缓冲

static u8 test_fbuf[128]; // 测试构帧缓冲

// 构帧: 填头+算校验和, 返回整帧长
static u16 test_frame_build(u8 *out, u8 dir, u8 cmd, u8 sub_cmd, const u8 *payload, u16 payload_len)
{
    u16 flen = JMS581_HEADER_LEN + payload_len;
    u8 sum = 0;

    memcpy(out, "I4S0", 4);
    out[4] = (u8)flen;
    out[5] = (u8)(flen >> 8);
    out[6] = dir;
    out[7] = cmd;
    out[8] = sub_cmd;
    if (payload_len)
    {
        memcpy(&out[JMS581_HEADER_LEN], payload, payload_len);
    }
    for (u16 i = 0; i < flen; i++)
    {
        if (i != 9)
        {
            sum += out[i];
        }
    }
    out[9] = sum;
    return flen;
}

// 注入环形缓冲并跑一轮帧层处理 (穿透 环形缓冲→搬运→拆帧→分发→回调 全链路)
static void test_inject(const u8 *data, u16 len)
{
    bsp_uart1_isr((uint8_t *)data, (uint32_t)len);
    jms581_frame_process();
}

//----- printf调试回调表 -----
static void tcb_dev_status(const jms581_dev_status_t *sta)
{
    printf("[t] 0x8000 err=%02x status=%d dev_list=%02x\n", sta->err_code, sta->status, sta->dev_list);
}

static void tcb_backup_ack(u8 err_code)
{
    printf("[t] 0x8001 ack err=%02x\n", err_code);
}

static void tcb_backup_report(const jms581_backup_report_t *rpt)
{
    char name[JMS581_NAME_LEN_MAX / 2 + 1];

    jms581_utf16le_to_ascii(rpt->l3_name, rpt->l3_name_len, name, sizeof(name));
    printf("[t] 0x8002 err=%02x files=%d dirs=%d size=%d(u%d) t=%ds ext=%d phase=%d prog=%d l3='%s'\n",
           rpt->err_code, (int)rpt->file_done_cnt, (int)rpt->folder_done_cnt,
           (int)rpt->total_size, rpt->total_unit, (int)rpt->time_sec,
           rpt->is_ext, rpt->phase, rpt->progress, name);
}

static void tcb_format_ack(u8 err_code)
{
    printf("[t] 0x8003 ack err=%02x\n", err_code);
}

static void tcb_format_status(const jms581_format_status_t *sta)
{
    printf("[t] 0x8003 sta err=%02x phase=%d prog=%d result=%d dev=%d\n",
           sta->err_code, sta->phase, sta->progress, sta->result, sta->dev_id);
}

static void tcb_pc_idle(const jms581_pc_idle_t *idle)
{
    printf("[t] 0x8004 err=%02x idle=%ds\n", idle->err_code, idle->idle_sec);
}

static void tcb_cancel_ack(u8 err_code)
{
    printf("[t] 0x8005 ack err=%02x\n", err_code);
}

static void tcb_capacity(const jms581_capacity_t *cap)
{
    printf("[t] 0x8006 err=%02x slots=%d\n", cap->err_code, cap->slot_num);
    for (u8 i = 0; i < 4; i++)
    {
        printf("    slot%d present=%d err=%02x total=%dKB used_valid=%d used=%dKB\n", i,
               cap->slot[i].present, cap->slot[i].err_code, (int)(u32)cap->slot[i].total_size,
               cap->slot[i].used_valid, (int)(u32)cap->slot[i].used_size);
    }
}

static void tcb_fw_ver(const jms581_fw_ver_t *ver)
{
    printf("[t] 0x8007 err=%02x ver=%d.%d.%d.%d\n", ver->err_code,
           ver->fw_ver[0], ver->fw_ver[1], ver->fw_ver[2], ver->fw_ver[3]);
}

static void tcb_dir_list(const jms581_dir_list_t *info, const jms581_dir_entry_t *entries, u8 cnt)
{
    char name[JMS581_NAME_LEN_MAX / 2 + 1];

    printf("[t] 0x8008 err=%02x cnt=%d more=%d\n", info->err_code, info->return_count, info->has_more);
    for (u8 i = 0; i < cnt; i++)
    {
        jms581_utf16le_to_ascii(entries[i].name, (u8)entries[i].name_len, name, sizeof(name));
        printf("    [%d] type=%d '%s'\n", i, entries[i].file_type, name);
    }
}

static void tcb_unknown(u8 cmd, u8 sub_cmd, const u8 *payload, u16 payload_len)
{
    printf("[t] unknown %02x%02x len=%d\n", cmd, sub_cmd, payload_len);
}

static const jms581_cb_t test_cbs = {
    .dev_status = tcb_dev_status,
    .backup_ack = tcb_backup_ack,
    .backup_report = tcb_backup_report,
    .format_ack = tcb_format_ack,
    .format_status = tcb_format_status,
    .pc_idle = tcb_pc_idle,
    .cancel_ack = tcb_cancel_ack,
    .capacity = tcb_capacity,
    .fw_ver = tcb_fw_ver,
    .dir_list = tcb_dir_list,
    .unknown = tcb_unknown,
};

void jms581_test_run(void)
{
    u16 flen;

    jms581_proto_cb_reg(&test_cbs); // 测试后仍保持注册, 真机联调可继续用; 上层接管时重新注册

    // 1. 合法0x8000状态包
    printf("[t]--1 legal 0x8000 (expect status=4 dev_list=05)\n");
    static const u8 pl_sta[3] = {0x00, 4, 0x05};
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x00, pl_sta, 3);
    test_inject(test_fbuf, flen);

    // 2. 前置3字节垃圾+合法帧 (重同步)
    printf("[t]--2 garbage prefix (expect 1 frame)\n");
    test_fbuf[0] = 0xAA;
    test_fbuf[1] = 'I';
    test_fbuf[2] = 0x55;
    flen = test_frame_build(&test_fbuf[3], JMS581_DIR_DEV2MCU, 0x80, 0x00, pl_sta, 3);
    test_inject(test_fbuf, flen + 3);

    // 3. 一帧劈两半分两次注入 (半帧续收)
    printf("[t]--3 split frame (expect 1 frame after 2nd inject)\n");
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x00, pl_sta, 3);
    test_inject(test_fbuf, 6);
    test_inject(&test_fbuf[6], flen - 6);

    // 4. 坏校验和帧+紧跟合法帧 (滑窗重同步不殃及后帧)
    printf("[t]--4 bad sum + good frame (expect only 1 frame)\n");
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x00, pl_sta, 3);
    test_fbuf[9] ^= 0xFF; // 破坏校验和
    flen += test_frame_build(&test_fbuf[flen], JMS581_DIR_DEV2MCU, 0x80, 0x00, pl_sta, 3);
    test_inject(test_fbuf, flen);

    // 5. 两帧粘包一次注入 (0x8000 + 0x8007)
    printf("[t]--5 two frames stuck (expect 0x8000 + 0x8007)\n");
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x00, pl_sta, 3);
    static const u8 pl_ver[5] = {0x00, 255, 9, 2, 6};
    flen += test_frame_build(&test_fbuf[flen], JMS581_DIR_DEV2MCU, 0x80, 0x07, pl_ver, 5);
    test_inject(test_fbuf, flen);

    // 6. direction=0x01帧 (应静默丢弃, 无输出)
    printf("[t]--6 direction=0x01 (expect nothing)\n");
    flen = test_frame_build(test_fbuf, JMS581_DIR_MCU2DEV, 0x80, 0x00, pl_sta, 3);
    test_inject(test_fbuf, flen);

    // 7. data_len=300非法+紧跟合法帧 (长度界限)
    printf("[t]--7 bad data_len=300 + good frame (expect only 1 frame)\n");
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x00, pl_sta, 3);
    test_fbuf[4] = (u8)300;
    test_fbuf[5] = (u8)(300 >> 8); // 改长度不重算校验和, 长度检查在校验和之前
    flen += test_frame_build(&test_fbuf[flen], JMS581_DIR_DEV2MCU, 0x80, 0x00, pl_sta, 3);
    test_inject(test_fbuf, flen);

    // 8. 0x8002备份报告: 26B(309兼容) 与 315扩展phase=3带l3_name
    printf("[t]--8 0x8002 309 26B + 315 ext l3='SD_128G_A1B2'\n");
    u8 pl_rpt[26 + 3 + 24];
    memset(pl_rpt, 0, sizeof(pl_rpt));
    pl_rpt[0] = 0x00; // err_code
    pl_rpt[1] = 120;  // file_done_cnt=120
    pl_rpt[9] = 2;    // total_unit=MB
    pl_rpt[10] = 0x00;
    pl_rpt[11] = 0x04; // total_size=1024
    pl_rpt[24] = JMS581_DEV_SD;
    pl_rpt[25] = JMS581_DEV_PCIE;
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x02, pl_rpt, 26);
    test_inject(test_fbuf, flen);
    pl_rpt[26] = 3;                                                        // phase=拷贝
    pl_rpt[27] = 66;                                                       // progress
    pl_rpt[28] = jms581_ascii_to_utf16le("SD_128G_A1B2", &pl_rpt[29], 24); // l3_name_len=24
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x02, pl_rpt, 29 + 24);
    test_inject(test_fbuf, flen);

    // 9. 0x8003双包型: 11B启动ACK 与 15B进度状态 (按data_len分流)
    printf("[t]--9 0x8003 ack + status\n");
    static const u8 pl_fmt_ack[1] = {0x00};
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x03, pl_fmt_ack, 1);
    test_inject(test_fbuf, flen);
    static const u8 pl_fmt_sta[5] = {0x00, 1, 60, 0, JMS581_DEV_SD};
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x03, pl_fmt_sta, 5);
    test_inject(test_fbuf, flen);

    // 10. 0x8004小端idle_sec=300
    printf("[t]--10 0x8004 (expect idle=300)\n");
    static const u8 pl_idle[3] = {0x00, 0x2C, 0x01};
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x04, pl_idle, 3);
    test_inject(test_fbuf, flen);

    // 11. 0x8008两条目页
    printf("[t]--11 0x8008 page (expect CARD_001/CARD_002 more=1)\n");
    u8 pl_dir[10 + 2 * (3 + 16)];
    memset(pl_dir, 0, sizeof(pl_dir));
    pl_dir[1] = 2;    // return_count
    pl_dir[2] = 0x01; // has_more
    pl_dir[4] = 0x40;
    pl_dir[8] = 0xA5; // next_cursor示例
    pl_dir[10] = 0;   // entry0: type=目录
    pl_dir[11] = 16;
    pl_dir[12] = 0; // name_len=16
    jms581_ascii_to_utf16le("CARD_001", &pl_dir[13], 16);
    pl_dir[29] = 0;
    pl_dir[30] = 16;
    pl_dir[31] = 0;
    jms581_ascii_to_utf16le("CARD_002", &pl_dir[32], 16);
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x80, 0x08, pl_dir, sizeof(pl_dir));
    test_inject(test_fbuf, flen);

    // 12. 未实现命令组0x81 (unknown兜底)
    printf("[t]--12 0x8100 (expect unknown)\n");
    static const u8 pl_upg[1] = {0x00};
    flen = test_frame_build(test_fbuf, JMS581_DIR_DEV2MCU, 0x81, 0x00, pl_upg, 1);
    test_inject(test_fbuf, flen);

    // 13. TX链路: 发一条0x8000查询 (可用逻辑分析仪/对端抓包核对帧头与校验和)
    printf("[t]--13 tx 0x8000 req ret=%d\n", jms581_dev_status_req());

    printf("[t] test done\n");
}

// 串口模拟联调轮询: 每3s发一条0x8000查询; PC端应收到 49 34 53 30 0A 00 01 80 00 8B,
// PC回任意合法应答帧, 调试口即打印解析结果
void jms581_test_poll(void)
{
    static u32 poll_tick = 0;

    if (tick_check_expire(poll_tick, 3000))
    {
        poll_tick = tick_get();
        jms581_dev_status_req();
    }
}
#endif // JMS581_TEST_EN
#endif // JMS581_EN
