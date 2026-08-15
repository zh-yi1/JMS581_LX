#ifndef _JMS581_MODEL_H
#define _JMS581_MODEL_H

// JMS581数据中间层: 协议回调表的唯一注册者, 缓存581上报数据供UI同步读取
//
// jms581_proto的回调表全局唯一(jms581_proto_cb_reg), 谁注册谁独占, 且协议层明确
// 不做状态缓存(见jms581_proto.h开头)。本模块独占注册并承担缓存, 界面一律不注册。
//
// 界面怎么用 (纯轮询, 两件事, 没有第三件):
//   初始化: jms581_model_xxx(&out) 同步取快照, 立刻返回, 永不阻塞
//   刷新:   每圈比对 jms581_model_ver(项), 变了才重画; 界面退出无需任何注销
//
// 纯轮询成立的前提: 没有抢占。
//   jms581_frame_process()(解析并调回调)和界面process在同一个主循环里一前一后
//   (见func.c), UART中断只往环形缓冲塞字节、不在ISR里调回调。所以结构体赋值不会
//   被读打断, 界面一帧内连读多项也彼此一致。
//   ★若将来改成ISR内解析或上RTOS多任务, 这个前提失效, 需重新评估。
//
// 建模原则: 存"状态", 不存"事件"。
//   瞬时的东西(启动ACK/取消被拒)一律折算进持久的流程状态并递增版本号, 界面早进
//   晚进读到的都对, 轮询不会丢信息。
//
// 预取: 脱机模式581上电后主动拉齐home首屏要的数据, 期间UI停在开机页,
//       gate_ready()为1才放行进home, 使home首帧即真值, 无需占位符。
//       581不回复即产品异常, 设计上无限重发并卡在开机页(长按关机/10分钟超时可退出)。

#if JMS581_EN

///版本号项: 该项每更新一次+1, 界面比对判断要不要重画
#define JMS581_VER_STATUS           0       //0x8000 设备状态/卡槽在位
#define JMS581_VER_CAP              1       //0x8006 容量
#define JMS581_VER_FW               2       //0x8007 固件版本
#define JMS581_VER_BACKUP           3       //备份流程 (0x8001+0x8002+0x8005合成)
#define JMS581_VER_FORMAT           4       //格式化流程 (0x8003)
#define JMS581_VER_DIR              5       //0x8008 目录列表
#define JMS581_VER_MAX              6

///目录名缓存规格: 协议给UTF-16LE最长64字节=32字符, 本层转ASCII存 (RAM减半,
///且compo_textbox_set()本来就吃char*, 省掉每次显示再转)
#define JMS581_DIR_NAME_MAX         33      //32字符 + 结尾'\0'
#define JMS581_DIR_TOPN_CNT         30      //Top-N请求条数 (协议允许8~32)
#define JMS581_DIR_WIN_CNT          40      //目录名窗口容量(>一帧30条, 留10条上下翻页余量)
#define JMS581_DIR_SEQ_CNT          30      //SEQ模式每帧条数(帧缓冲2048, 30条×67B≈2020B)
///cursor索引(动态分配): 初始100帧, 建满每次扩50帧, 软上限1024帧(覆盖10240目录,
///索引区最多6KB)。realloc失败或到上限就停止记索引, 更早的目录翻不到(优雅降级)
#define JMS581_DIR_IDX_INIT         100
#define JMS581_DIR_IDX_GROW         50
#define JMS581_DIR_IDX_MAX          1024
///边界预取提前量: 命中位置离窗口两端这么近就后台预取相邻页, 翻页边界处零等待不闪屏
#define JMS581_DIR_PREFETCH_AHEAD   4

/*----------------------------------------------------------------------------
 * 备份流程状态 (0x8001启动ACK / 0x8002报告 / 0x8005取消ACK 三条合成一个状态)
 *
 *   IDLE --backup_start--> STARTING --ACK=0--> RUNNING --0x8002终态--> DONE
 *                              |                  |                 \-> FAILED
 *                          ACK!=0                 |                  \-> CANCELLED
 *                              v            backup_cancel且ACK=0
 *                           FAILED                v
 *                                             CANCELING --0x8002终态--> CANCELLED
 *
 * 注意: 0x8005 ACK=0 只代表"取消请求被接受", 不代表已停止。真正的终态要等
 *       0x8002 推 err_code=0x0B。ACK=1(无可取消)/0x0C(收尾阶段拒绝)不改状态,
 *       只把码记在err_code里并递增版本号, 界面据此弹提示。
 *--------------------------------------------------------------------------*/
#define JMS581_BK_IDLE              0       //没在备份
#define JMS581_BK_STARTING          1       //已发0x8001, 等ACK
#define JMS581_BK_RUNNING           2       //ACK=0已接受, 0x8002推进度中
#define JMS581_BK_CANCELING         3       //取消已被接受, 等0x8002终态
#define JMS581_BK_DONE              4       //终态: 成功
#define JMS581_BK_FAILED            5       //终态: 失败或启动被拒 (err_code有效)
#define JMS581_BK_CANCELLED         6       //终态: 用户取消

typedef struct {
    u8   sta;                       //JMS581_BK_x
    u8   err_code;                  //最近一次非0错误码: 启动被拒/0x09备份失败/0x0C取消被拒
    u8   sub_err;                   //备份引擎SubErrorCode
    u8   phase;                     //0=终态 1=挂载盘 2=扫描文件 3=拷贝 (仅315扩展有效)
    u8   progress;                  //0~100, 仅phase=3有效
    u8   has_ext;                   //1=581给了315扩展(phase/progress/l3_name才有意义)
    u32  file_done_cnt;             //已处理文件数
    u32  folder_done_cnt;           //已处理文件夹数
    u8   total_unit;                //0=B 1=KB 2=MB 3=GB
    u32  total_size;                //文件总大小
    u8   speed_unit;                //0=B/s 1=KB/s 2=MB/s 3=GB/s
    u32  speed_val;                 //速度值
    u32  time_sec;                  //耗时, 秒
    u8   src_dev;                   //源盘协议ID, 0xFF=未知
    u8   dst_dev;                   //目标盘协议ID, 0xFF=未知
    char l3_name[JMS581_DIR_NAME_MAX];  //L3目录名, 已转ASCII (协议给的指针回调返回即失效)
} jms581_backup_sta_t;

/*----------------------------------------------------------------------------
 * 格式化流程状态 (0x8003: 11B启动ACK + 15B进度/终态)
 *
 *   IDLE --format_start--> STARTING --ACK=0--> RUNNING --phase=0--> DONE / FAILED
 *                              |
 *                          ACK!=0 --> FAILED
 *--------------------------------------------------------------------------*/
#define JMS581_FMT_IDLE             0
#define JMS581_FMT_STARTING         1       //已发0x8003(带dev_id), 等11B ACK
#define JMS581_FMT_RUNNING          2       //进行中, progress有效
#define JMS581_FMT_DONE             3       //终态: 成功
#define JMS581_FMT_FAILED           4       //终态: 失败 (err_code有效)

typedef struct {
    u8 sta;                         //JMS581_FMT_x
    u8 err_code;                    //最近一次非0错误码 (如0x0A格式化失败)
    u8 progress;                    //0~100
    u8 dev_id;                       //正在格式化的设备(协议设备ID)
} jms581_format_sta_t;

/**
 * @brief 初始化: 注册协议回调表 (全工程唯一一处cb_reg), jms581_mode_init()内调用
 **/
void jms581_model_init(void);

/**
 * @brief 周期任务: 推进预取状态机, jms581_mode_process()每圈调用
 **/
void jms581_model_process(void);

/*----------------------------------------------------------------------------
 * 同步快照: 界面初始化直接调, 立刻返回, 返回1=有有效值 / 0=还没拿到过
 *--------------------------------------------------------------------------*/
u8 jms581_model_dev_status(jms581_dev_status_t *out);       //0x8000
u8 jms581_model_capacity(jms581_capacity_t *out);           //0x8006
u8 jms581_model_fw_ver(u8 ver[4]);                          //0x8007
u8 jms581_model_pc_idle(u16 *idle_sec);                     //0x8004

///备份/格式化流程: 无条件填出参并返回当前状态值(IDLE也是有效状态, 不返回0/1)
u8 jms581_model_backup_sta(jms581_backup_sta_t *out);
u8 jms581_model_format_sta(jms581_format_sta_t *out);

/*----------------------------------------------------------------------------
 * 0x8008 目录列表: 对界面呈现为"一条按编号从大到小的完整序列", pos=0是最新的。
 * cursor、分页、两种list_mode全关在本层内, 界面只管 open 一次然后按位置取名字。
 *
 * 内部分两段:
 *   进页 Top-N(list_mode=1, count=30) 一帧拿到最新30条, 立刻能显示;
 *   若拿满30条说明可能还有更早的, 后台另开 list_mode=0 会话每帧10条扫到 has_more=0,
 *   沿途只记每帧的起始cursor(不存名字)。之后翻到哪一段就按对应cursor现拉哪一段。
 *
 * 名字缓冲是一块30条的滑动窗口: Top-N阶段装最新30条; 翻出这30条后, 向下翻页时
 * 一次连拉当前帧+更旧一帧(共20条)并入窗口、丢弃最新端, 整页下移10条, 翻到下一页
 * 边界处零等待; 向上翻页按单帧拉取。合并后丢弃远离当前访问位置的一端, 可视区始终
 * 命中, 不会在跨页边界处来回抖动。
 *--------------------------------------------------------------------------*/
/**
 * @brief 开一个目录会话: 复位状态并发出Top-N首帧。root_type见JMS581_ROOT_x
 **/
u8 jms581_model_dir_open(u8 root_type);

/**
 * @brief Top-N首帧是否已回。界面据此决定能不能跳页显示, 保证首帧即真值
 **/
u8 jms581_model_dir_ready(void);

/**
 * @brief 已知总条数。后台扫描未完成时返回Top-N拿到的条数(界面先只能翻这么多)
 **/
u16 jms581_model_dir_total(void);

/**
 * @brief 取逻辑第pos条目录名(pos=0是最新的)
 * @return 命中缓冲返回名字; 未命中返回NULL并在内部发起拉取, 界面下一圈再问
 *         (纯轮询, 本层不向界面回调)
 **/
const char *jms581_model_dir_at(u16 pos);

/**
 * @brief 边界预取: UI滚动后调用。访问位置接近窗口两端时提前拉相邻页,
 *        使翻到下一页边界处零等待不闪屏。预取在后台串行推进, 本调用非阻塞。
 **/
void jms581_model_dir_prefetch(u16 pos);

u8 jms581_model_dir_err(void);              //最近一次0x8008的err_code, 0=正常

/**
 * @brief 取某项的更新计数 (JMS581_VER_x), 界面记住上次值, 不等就重画
 **/
u32 jms581_model_ver(u8 item);

/*----------------------------------------------------------------------------
 * 请求发送: 必须走本层而不是直接调jms581_proto的req
 *
 * 因为这几条要同步推进流程状态 —— 界面直接调proto的话, model不知道"已经发出去了",
 * 状态卡在IDLE, 界面就分不清"还没发"和"发了在等"。0x8000/0x8004/0x8006/0x8007
 * 没有流程状态, 不需要包装(且都由本层预取或jms581_mode.c自行发起)。
 *
 * 返回1=已交驱动发送, 0=参数非法/驱动错误/当前状态不允许
 *--------------------------------------------------------------------------*/
///开始备份: name传NULL=短帧(581自管目录名), 非NULL=长帧(ASCII内部转UTF-16LE);
///mode=JMS581_MODE_RECENT时days必填1~31, 其余mode传0
u8 jms581_model_backup_start(u8 src_dev, u8 dst_dev, u8 mode,
                             const char *name, u8 days);
u8 jms581_model_backup_cancel(void);        //取消备份, 仅RUNNING态允许
u8 jms581_model_format_start(u8 dev_id);    //开始格式化

/*---- 预取控制 (jms581_fsm_goto内调用, 其他地方勿调) ----*/
void jms581_model_prefetch_start(void);     //581上电进脱机: 清缓存并启动预取
void jms581_model_prefetch_abort(void);     //581断电: 清缓存并停预取
u8   jms581_model_gate_ready(void);         //1=挡门数据(0x8000+0x8006)已到齐, 可放行进home

#endif // JMS581_EN
#endif // _JMS581_MODEL_H
