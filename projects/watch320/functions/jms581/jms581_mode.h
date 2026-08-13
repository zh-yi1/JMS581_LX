#ifndef _JMS581_MODE_H
#define _JMS581_MODE_H

// JMS581使用流程主状态机: 关机/PC/充电/脱机四模式, 大switch形态, 任意界面下都生效
// 裸机事件方式不依赖消息队列; 状态跳转总览见jms581_mode.c的jms581_mode_process()上方注释
//
// 模式与引脚归属表 (唯一真源):
//   模式   func任务         PB11(581供电)  PE4(VBUS_OUT)  PE0(整机充电)  界面
//   关机   FUNC_PWRBLACK    0              0              关             纯黑, 短按临时显示电量
//   PC     FUNC_PCMODE      1              1              关(不扰USB通信) 黑底文字"PC模式"
//   充电   FUNC_JMSCHARGE   0(不开581)     0              开             黑底文字"充电模式"+电量
//   脱机   FUNC_OFFLINE     1              0(插USB也保持0) USB在位即开    黑底文字"脱机"

// 是否打开JMS581四模式状态机 (依赖JMS581_EN即UART1协议层, 改0整体关闭)
#define JMS581_MODE_EN              (JMS581_EN && 1)

// 硬件: PB12系统供电锁存(上电拉高不再断开), PE7 VBUS_DET输入(默认下拉,USB插入拉高),
//       PE4 VBUS_OUT输出(高:允许581进PC模式), PB11 581供电(高开低关),
//       PE0 整机充电控制(电池充电, 非581; 拉低=开充电; 外部上拉, 释放为输入=关充电; 需LOUDSPEAKER_MUTE_EN=0让出PE0)
// 注: USB口属于581, MCU仅通过PE7检测插拔, 不走MCU自身VUSB检测
// PB12锁存: 输出高。仅jms581_mode_init()调用一次(SDK初始化bsp_sys_init跑完之后,
// 首次上电为USB供电, 初始化期间无需锁存; 拉高后电池供电不再断开)
#define JMS581_SYS_PWR_LATCH()      do { GPIOBDE |= BIT(12); GPIOBDIR &= ~BIT(12); GPIOBSET = BIT(12); } while (0)

///时序参数
#define JMS581_PE4_SETTLE_MS        30          //PE4拉高到581上电的稳定时间(需求>=30ms, 留余量)
#define JMS581_BOOT_WAIT_MS         200         //581上电到首次查询0x8000的等待时间
#define JMS581_STATUS_POLL_MS       1000        //0x8000查询周期, 无超时(581必回复一个状态)
#define JMS581_PC_IDLE_POLL_MS      5000        //PC模式0x8004空闲查询周期
#define JMS581_PC_IDLE_OFF_S        600         //PC空闲关机阈值(秒), 10分钟
#define JMS581_PC_BUSY_THRESH_S     60           //PC空闲小于此值视为读写中, 长按切充电被忽略
#define JMS581_OFFLINE_IDLE_OFF_MS  (600 * 1000)//脱机无操作关机阈值, 10分钟

///模式 (与func任务一一对应)
enum {
    JMS581_MODE_SHUTDOWN = 0,   //关机模式
    JMS581_MODE_PC,             //PC模式
    JMS581_MODE_CHARGE,         //充电模式
    JMS581_MODE_OFFLINE,        //脱机模式
};

#if JMS581_MODE_EN

/**
 * @brief 模块初始化: 拉PB12锁存+GPIO配置+注册协议回调表+初始vbus采样, func_run()开头调用一次
 **/
void jms581_mode_init(void);

/**
 * @brief 大switch主状态机唯一入口: 采集事件(按键标志位/vbus插入中断+拔出电平)并按
 *        模式分支处理, 再推进本模式周期任务; func_process()每圈调用, 与当前界面无关
 **/
void jms581_mode_process(void);

///当前模式查询 (只读, 模式变化全在本模块内部)
u8 jms581_mode_get(void);

///消抖后的vbus状态: 1=USB在位
u8 jms581_vbus_in(void);

///关机模式短按请求显示电量: UI轮询取走(返回1表示有新请求, 取走即清)
u8 jms581_mode_bat_show_take(void);

#endif // JMS581_MODE_EN
#endif // _JMS581_MODE_H
