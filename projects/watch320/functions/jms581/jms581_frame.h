#ifndef _JMS581_FRAME_H
#define _JMS581_FRAME_H

// JMS581存储带屏串口协议 帧层: 组帧/拆帧/校验/重同步
// 协议依据: doc/JMS581存储带屏串口通讯协议 §2 通用帧格式

// 是否打开JMS581串口协议 (依赖UART1)
#define JMS581_EN               (BSP_UART1_EN)

#define JMS581_HEADER_LEN       10      //帧头长度: "I4S0"+data_len(LE16)+direction+cmd+subCmd+sum_crc
#define JMS581_FRAME_MAX        2048    //协议单帧上限 (V1.17起0x8008应答缓冲约2048字节, §13.3)
#define JMS581_RX_BUF_SIZE      2048    //帧组装缓冲大小 (>=JMS581_FRAME_MAX)
#define JMS581_TX_BUF_SIZE      80      //发送缓冲大小, 当前最长请求为0x8001长帧79字节

#define JMS581_DIR_MCU2DEV      0x01    //方向: 显示MCU→581
#define JMS581_DIR_DEV2MCU      0x00    //方向: 581→显示MCU

/**
 * @brief 组帧发送: 自动填"I4S0"/data_len/direction=0x01/cmd/subCmd/校验和
 * @param[in] cmd: 命令组, 如0x80
 * @param[in] sub_cmd: 子命令, 如0x01
 * @param[in] payload: 负载数据, 无负载可传NULL
 * @param[in] payload_len: 负载字节数
 * @return 1=已交驱动发送, 0=长度超限或驱动错误
 **/
u8 jms581_frame_tx(u8 cmd, u8 sub_cmd, const u8 *payload, u16 payload_len);

/**
 * @brief 帧层接收处理: 搬运环形缓冲+拆帧+校验+上交协议层, 主循环轮询调用
 **/
void jms581_frame_process(void);
#endif
