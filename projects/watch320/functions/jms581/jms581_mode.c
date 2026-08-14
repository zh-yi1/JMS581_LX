/*****************************************************************************
 * Module    : JMS581使用流程主状态机 (关机/PC/充电/脱机四模式)
 * File      : jms581_mode.c
 * Function  : 大switch状态机, 唯一入口jms581_mode_process()挂func_process每圈执行;
 *             事件裸机采集(按键标志位/vbus插入中断+拔出电平), 不依赖消息队列;
 *             模式切换经jms581_fsm_goto统一收敛引脚并指派UI任务, UI纯显示
 *****************************************************************************/
#include "include.h"
#include "func.h"

#if JMS581_MODE_EN

#define TRACE_EN                1
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*----------------------------------------------------------------------------
 * 常量与类型
 *--------------------------------------------------------------------------*/
///0x8000应答status字段值 (协议§5.3)
#define JMS581_STA_NOT_PC       0       //空闲/充电器/脱机
#define JMS581_STA_PC           4       //PC模式
#define JMS581_STA_JUDGING      6       //判定中

///判模式序列子状态 (仅关机模式插入USB后判581用途时使用)
enum {
    SEQ_IDLE = 0,
    SEQ_PE4_SETTLE,             //PE4已拉高, 等稳定
    SEQ_581_BOOT,               //PB11已上电, 等581启动
    SEQ_STATUS_POLL,            //轮询0x8000等应答
};

///vbus事件 (jms581_vbus_evt_take返回值)
enum {
    VBUS_EVT_NONE = 0,
    VBUS_EVT_IN,                //USB插入
    VBUS_EVT_OUT,               //USB拔出
};

typedef struct {
    u8  mode;                   //当前模式 JMS581_MODE_x
    u8  vbus_sta;               //vbus电平记录

    u8  seq_sta;                //序列走到哪一步 SEQ_x, SEQ_IDLE=未在判,即关机插入时581模式判断的状态机
    u8  sta_flag;               //收到0x8000应答标志
    u8  sta_val;                //0x8000应答status
    u32 seq_tick;               //序列本状态计时(轮询周期共用)

    //PC模式空闲检测: 每5秒发0x8004问581"多久没读写了", 用于10分钟无读写关机
    //和长按转充电的忙判断
    u8  idle_valid;             //1=拿到过581的有效回答(err=0), 0=还没有或581说自己非PC
    u16 idle_sec;               //581回答的"已经多少秒没读写", 每次应答刷新
    u32 idle_poll_tick;         //上次发0x8004的时刻, 用来控制5秒一问

    u32 offline_tick;           //脱机无操作计时

    u8  chg_on;                 //PE0充电开关缓存(去重写SFR)
    u8  bat_show_req;           //关机模式短按显示电量请求(UI轮询取走)
} jms581_mode_cb_t;

static jms581_mode_cb_t mode_cb;
static volatile u8 vbus_irq;    //插入沿中断标志: ISR置1, 主循环取走清零

static void jms581_fsm_goto(u8 mode);

/*----------------------------------------------------------------------------
 * GPIO底层 (SFR直写, DIR置位=输入)
 *--------------------------------------------------------------------------*/
//PB11 581供电
static void jms581_io_581_pwr(u8 on)
{
    GPIOBDE  |= BIT(11);
    GPIOBDIR &= ~BIT(11);
    if (on) {
        GPIOBSET = BIT(11);
    } else {
        GPIOBCLR = BIT(11);
    }
    TRACE("jms581: PB11(581 pwr)=%d\n", on);
}

//PE4 VBUS_OUT: 1=允许581进PC模式
static void jms581_io_vbus_out(u8 hi)
{
    GPIOEDE  |= BIT(4);
    GPIOEDIR &= ~BIT(4);
    if (hi) {
        GPIOESET = BIT(4);
    } else {
        GPIOECLR = BIT(4);
    }
    TRACE("jms581: PE4(vbus_out)=%d\n", hi);
}

//PE7 VBUS_DET: 浮空输入, 电平靠板上外部下拉/USB插入拉高
static void jms581_io_vbus_det_init(void)
{
    GPIOEDE  |= BIT(7);                 //数字功能使能
    GPIOEDIR |= BIT(7);                 //方向=输入(1=输入)
    GPIOEPU  &= ~BIT(7);                //关内部上拉
    GPIOEPD  &= ~BIT(7);                //关内部下拉: 板上有外部下拉, 内部并联会拉低高电平
}

//读PE7原始电平(未消抖): 1=USB在位, 0=不在; GPIOE每位对应一个引脚实时电平
static u8 jms581_io_vbus_raw(void)
{
    return (GPIOE >> 7) & 1;            //bit7移到最低位, 只留1位
}

//PE0 整机充电控制(电池充电, 非581): 开=输出低; 关=释放为输入, 由外部上拉拉高
static void jms581_io_charge(u8 on)
{
    if (mode_cb.chg_on == on) {
        return;
    }
    mode_cb.chg_on = on;
    GPIOEDE  |= BIT(0);
    GPIOEPU  &= ~BIT(0);
    GPIOEPD  &= ~BIT(0);
    if (on) {
        GPIOEDIR &= ~BIT(0);
        GPIOECLR  = BIT(0);
    } else {
        GPIOEDIR |= BIT(0);
    }
    TRACE("jms581: PE0(charge)=%s\n", on ? "on" : "off");
}

/*----------------------------------------------------------------------------
 * IO语义封装: 调用处只表达意图, 电平方向只存在于上面的io函数内部
 *--------------------------------------------------------------------------*/
#define JMS581_581_PWR_ON()         jms581_io_581_pwr(1)    //打开581供电
#define JMS581_581_PWR_OFF()        jms581_io_581_pwr(0)    //关闭581供电
#define JMS581_PC_MODE_ON()         jms581_io_vbus_out(1)   //允许581进PC模式
#define JMS581_PC_MODE_OFF()        jms581_io_vbus_out(0)   //禁止581进PC模式
#define BAT_CHARGE_ON()             jms581_io_charge(1)     //打开整机充电(电池)
#define BAT_CHARGE_OFF()            jms581_io_charge(0)     //关闭整机充电

///全关: 关机/充电模式引脚状态 (581断电+禁PC)
static void jms581_pin_all_off(void)
{
    JMS581_PC_MODE_OFF();
    JMS581_581_PWR_OFF();
}

///脱机供电: 581开且禁PC (PC模式开581走fsm_goto的PC分支: PE4高+PB11开)
static void jms581_pin_offline(void)
{
    JMS581_PC_MODE_OFF();
    JMS581_581_PWR_ON();
}

/*----------------------------------------------------------------------------
 * vbus检测: 插入=PE7上升沿中断置标志(一次注册, 兼做低功耗唤醒源), 拔出=电平轮询
 *--------------------------------------------------------------------------*/
AT(.com_text.jms581.vbus)
static void jms581_vbus_rise_isr(void)
{
    vbus_irq = 1;
}

///取一次vbus事件(取走即清), 返回VBUS_EVT_x
///插入 = 有上升沿标志 且 电平高 (沿标志与电平不符视为毛刺丢弃)
///拔出 = 记录在位 但 电平低 (纯轮询, 不用中断)
static u8 jms581_vbus_evt_take(void)
{
    u8 raw = jms581_io_vbus_raw();      //当前电平
    u8 irq = vbus_irq;                  //插入沿标志, 取走即清
    vbus_irq = 0;

    if (irq && raw && !mode_cb.vbus_sta) {
        mode_cb.vbus_sta = 1;
        TRACE("jms581: vbus in\n");
        return VBUS_EVT_IN;
    }
    if (!raw && mode_cb.vbus_sta) {
        mode_cb.vbus_sta = 0;
        TRACE("jms581: vbus out\n");
        return VBUS_EVT_OUT;
    }
    return VBUS_EVT_NONE;
}

u8 jms581_vbus_in(void)
{
    return mode_cb.vbus_sta;
}

/*----------------------------------------------------------------------------
 * 协议回调 (主循环上下文, 由jms581_frame_process分发)
 *--------------------------------------------------------------------------*/
static void jms581_mode_dev_status_cb(const jms581_dev_status_t *sta)
{
    TRACE("jms581: dev_status err=%d sta=%d dev=0x%x\n", sta->err_code, sta->status, sta->dev_list);
    if (sta->err_code == JMS581_ERR_NONE) {
        mode_cb.sta_val  = sta->status;
        mode_cb.sta_flag = 1;
    }
}

static void jms581_mode_pc_idle_cb(const jms581_pc_idle_t *idle)
{
    if (idle->err_code == JMS581_ERR_NONE) {
        mode_cb.idle_valid = 1;
        mode_cb.idle_sec   = idle->idle_sec;
    } else {
        mode_cb.idle_valid = 0;                     //err=1: 581称非PC模式
        TRACE("jms581: pc_idle err=%d\n", idle->err_code);
    }
}

static const jms581_cb_t jms581_mode_cbs = {
    .dev_status = jms581_mode_dev_status_cb,
    .pc_idle    = jms581_mode_pc_idle_cb,
};

/*----------------------------------------------------------------------------
 * 判模式序列引擎: PE4拉高 -> >=30ms -> PB11上电 -> 等启动 -> 1秒一次查0x8000
 * 无超时(581必回复一个状态): 答PC进PC模式, 非PC进充电模式, "判定中"继续查
 *--------------------------------------------------------------------------*/
static void jms581_seq_start(void)
{
    TRACE("jms581: seq start\n");
    BAT_CHARGE_OFF();                               //判模式期间关充电, 不影响USB通信
    JMS581_581_PWR_OFF();
    JMS581_PC_MODE_ON();                            //PE4先行, 确保581上电前电平已稳定
    mode_cb.seq_sta  = SEQ_PE4_SETTLE;
    mode_cb.sta_flag = 0;
    mode_cb.seq_tick = tick_get();
}

static void jms581_seq_abort(void)
{
    if (mode_cb.seq_sta != SEQ_IDLE) {
        TRACE("jms581: seq abort\n");
    }
    mode_cb.seq_sta = SEQ_IDLE;
}

//判定非PC: 结束序列进充电模式 (充电模式不开581, 引脚由fsm_goto收敛: 581断电+PE4拉低+开充电)
static void jms581_seq_to_charge(void)
{
    TRACE("jms581: seq result = CHARGE\n");
    mode_cb.seq_sta = SEQ_IDLE;
    jms581_fsm_goto(JMS581_MODE_CHARGE);
}

//序列推进一步; 完成时直接切模式
static void jms581_seq_process(void)
{
    switch (mode_cb.seq_sta) {
    case SEQ_PE4_SETTLE:
        if (tick_check_expire(mode_cb.seq_tick, JMS581_PE4_SETTLE_MS)) {
            JMS581_581_PWR_ON();
            mode_cb.seq_sta  = SEQ_581_BOOT;
            mode_cb.seq_tick = tick_get();
        }
        break;

    case SEQ_581_BOOT:
        if (tick_check_expire(mode_cb.seq_tick, JMS581_BOOT_WAIT_MS)) {
            mode_cb.sta_flag = 0;
            jms581_dev_status_req();                //首次查询
            mode_cb.seq_sta  = SEQ_STATUS_POLL;
            mode_cb.seq_tick = tick_get();
        }
        break;

    case SEQ_STATUS_POLL:
        if (mode_cb.sta_flag) {                     //收到0x8000应答
            mode_cb.sta_flag = 0;
            if (mode_cb.sta_val == JMS581_STA_PC) {
                TRACE("jms581: seq result = PC\n");
                mode_cb.seq_sta = SEQ_IDLE;
                jms581_fsm_goto(JMS581_MODE_PC);
            } else if (mode_cb.sta_val == JMS581_STA_NOT_PC) {
                jms581_seq_to_charge();
            }
            //判定中/其他值: 留在本状态继续查
        } else if (tick_check_expire(mode_cb.seq_tick, JMS581_STATUS_POLL_MS)) {
            jms581_dev_status_req();                //1秒一次, 无超时: 581必回复
            mode_cb.seq_tick = tick_get();
        }
        break;

    default:
        break;
    }
}

/*----------------------------------------------------------------------------
 * 模式切换唯一出口: 引脚在切换当下收敛, 不等任务enter; UI只是被指派的显示
 *--------------------------------------------------------------------------*/
static const u8 tbl_mode_func[] = {
    [JMS581_MODE_SHUTDOWN] = FUNC_PWRBLACK,
    [JMS581_MODE_PC]       = FUNC_COMPUTER_PAGE,
    [JMS581_MODE_CHARGE]   = FUNC_JMSCHARGE,
    [JMS581_MODE_OFFLINE]  = FUNC_HOME_PAGE,
};

static void jms581_fsm_goto(u8 mode)
{
    TRACE("jms581: fsm goto mode %d -> %d\n", mode_cb.mode, mode);
    mode_cb.mode = mode;
    mode_cb.idle_valid     = 0;
    mode_cb.idle_poll_tick = tick_get();
    mode_cb.offline_tick   = tick_get();
    mode_cb.bat_show_req   = 0;

    switch (mode) {
    case JMS581_MODE_SHUTDOWN:
        jms581_seq_abort();
        jms581_pin_all_off();
        jms581_io_charge(mode_cb.vbus_sta);         //USB仍在位则后台继续充电(581已断电, 不涉及USB通信)
        break;
    case JMS581_MODE_PC:
        JMS581_PC_MODE_ON();
        JMS581_581_PWR_ON();
        BAT_CHARGE_OFF();                           //PC模式不充电, 不影响USB通信
        break;
    case JMS581_MODE_CHARGE:
        jms581_pin_all_off();                       //充电模式不开581, 长按进脱机才上电
        BAT_CHARGE_ON();                            //确认非PC才开充电
        break;
    case JMS581_MODE_OFFLINE:
        jms581_pin_offline();
        jms581_io_charge(mode_cb.vbus_sta);         //跟随USB在位与否
        break;
    default:
        break;
    }

    func_cb.sta = tbl_mode_func[mode];
}

/*----------------------------------------------------------------------------
 * 主状态机
 *
 * 状态跳转总览 (跳转只出现在jms581_fsm_goto调用处, 搜"fsm_goto"即全部):
 *
 *   [关机] --vbus_in-----------------> 挂开机页+启判模式序列 --581答PC--> [PC]
 *                                                          --非PC------> [充电]
 *                                      (判定中拔出USB: 开机页退回关机黑屏)
 *   [关机] --长按--------------------> [脱机] (待改: 也应先过开机页, 后续处理)
 *   [关机] --短按--------------------> 显示电量3s (不跳转)
 *
 *   [PC]   --vbus_out / 空闲>=10min--> [关机]
 *   [PC]   --空闲时长按--------------> [充电]
 *
 *   [充电] --vbus_out----------------> [关机]
 *   [充电] --长按--------------------> [脱机]
 *
 *   [脱机] --长按 / 无操作>=10min----> [关机]
 *   [脱机] --插拔USB不切模式 (PE4保持低, 仅控PE0充电)
 *
 *   注: 插入是沿事件不是电平, 超时关机后USB仍在位不会重启判模式, 拔出重插才再判;
 *       首次上电USB在位由init补发一次vbus_in事件;
 *       581供电(PB11)仅PC/脱机开: 充电模式不开581, 长按进脱机才上电;
 *       进关机时USB仍在位则PE0保持充电(后台充), 拔出才关
 *--------------------------------------------------------------------------*/
void jms581_mode_process(void)
{
    u8 key, vbus;

    vbus = jms581_vbus_evt_take();                  //入口只采集事件, 各模式case自行消化
    key  = key_on_event_get();

    switch (mode_cb.mode) {
    /*======================================================================*/
    case JMS581_MODE_SHUTDOWN:
        if (key == KEY_ON_EVT_CLICK) {              //短按: 请求UI显示电量3秒
            mode_cb.bat_show_req = 1;
        } else if (key == KEY_ON_EVT_LONG) {        //长按: 开机进脱机
            jms581_seq_abort();
            jms581_fsm_goto(JMS581_MODE_OFFLINE);
            break;
        }

        if (vbus == VBUS_EVT_IN) {                  //插入: 挂开机页等待, 启判模式序列
            func_cb.sta = FUNC_TURN_ON_PAGE;
            jms581_seq_start();
        } else if (vbus == VBUS_EVT_OUT) {          //拔出: 中止序列回全关, 停后台充电
            jms581_seq_abort();
            jms581_pin_all_off();
            BAT_CHARGE_OFF();
            if (func_cb.sta == FUNC_TURN_ON_PAGE) {
                func_cb.sta = FUNC_PWRBLACK;        //判定中拔出: 开机页退回关机黑屏
            }
        }
        jms581_seq_process();
        break;

    /*======================================================================*/
    case JMS581_MODE_PC:
        if (key == KEY_ON_EVT_LONG) {               //无读写时长按: 581断电转充电
            if (mode_cb.idle_valid && mode_cb.idle_sec >= JMS581_PC_BUSY_THRESH_S) {
                jms581_fsm_goto(JMS581_MODE_CHARGE);
                break;
            }
            TRACE("jms581: pc busy, long press ignored\n");
        }

        if (vbus == VBUS_EVT_OUT) {                 //拔出: 关机
            jms581_fsm_goto(JMS581_MODE_SHUTDOWN);
            break;
        }

        if (tick_check_expire(mode_cb.idle_poll_tick, JMS581_PC_IDLE_POLL_MS)) {
            mode_cb.idle_poll_tick = tick_get();    //0x8004空闲轮询
            jms581_pc_idle_req();
        }
        if (mode_cb.idle_valid && mode_cb.idle_sec >= JMS581_PC_IDLE_OFF_S) {
            TRACE("jms581: pc idle %ds, timeout\n", mode_cb.idle_sec);
            jms581_fsm_goto(JMS581_MODE_SHUTDOWN);  //无读写超10分钟: 关机
        }
        break;

    /*======================================================================*/
    case JMS581_MODE_CHARGE:
        if (key == KEY_ON_EVT_LONG) {               //长按: 开机进脱机
            jms581_fsm_goto(JMS581_MODE_OFFLINE);
            break;
        }

        if (vbus == VBUS_EVT_OUT) {                 //拔出: 关机
            jms581_fsm_goto(JMS581_MODE_SHUTDOWN);
        }
        break;

    /*======================================================================*/
    case JMS581_MODE_OFFLINE:
        if (key == KEY_ON_EVT_CLICK || key == KEY_ON_EVT_DOUBLE) {
            mode_cb.offline_tick = tick_get();      //任意按键复位无操作计时
        } else if (key == KEY_ON_EVT_LONG) {        //长按: 关机
            jms581_fsm_goto(JMS581_MODE_SHUTDOWN);
            break;
        }

        jms581_io_charge(mode_cb.vbus_sta);         //插拔USB不切模式, 仅跟随开关充电

        if (tick_check_expire(mode_cb.offline_tick, JMS581_OFFLINE_IDLE_OFF_MS)) {
            TRACE("jms581: offline idle timeout\n");
            jms581_fsm_goto(JMS581_MODE_SHUTDOWN);  //10分钟无操作: 关机
        }
        break;

    /*======================================================================*/
    default:
        break;
    }
}

/*----------------------------------------------------------------------------
 * 查询/初始化
 *--------------------------------------------------------------------------*/
u8 jms581_mode_get(void)
{
    return mode_cb.mode;
}

u8 jms581_mode_bat_show_take(void)
{
    u8 req = mode_cb.bat_show_req;
    mode_cb.bat_show_req = 0;
    return req;
}

void jms581_mode_init(void)
{
    memset(&mode_cb, 0, sizeof(mode_cb));
    JMS581_SYS_PWR_LATCH();                         //SDK初始化跑完, 拉高PB12锁存系统供电(仅此一处)
    sys_cb.sleep_time  = -1L;                       //禁用SDK自动休眠/自动关机计时, 全部由主状态机接管
    sys_cb.pwroff_time = -1L;
    jms581_io_vbus_det_init();
    jms581_pin_all_off();
    mode_cb.chg_on = 0xFF;                          //非法值, 强制首次写生效
    BAT_CHARGE_OFF();
    jms581_proto_cb_reg(&jms581_mode_cbs);

    mode_cb.mode = JMS581_MODE_SHUTDOWN;            //与DEFAULE_START_FUNC=FUNC_PWRBLACK对应
    extab_user_isr_set(IO_PE7, RISE_EDGE, IOUD_SEL_NULL, jms581_vbus_rise_isr);   //板上有外部下拉, 不开内部

    if (jms581_io_vbus_raw()) {
        vbus_irq = 1;                               //首次上电USB在位: 补发一次插入沿
    }
    mode_cb.offline_tick = tick_get();
    TRACE("jms581: mode init, vbus=%d\n", jms581_io_vbus_raw());
}

#endif // JMS581_MODE_EN
