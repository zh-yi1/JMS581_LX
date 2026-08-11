#include "include.h"

#if JMS581_EN

#define JMS581_FRAME_TIMEOUT_MS 100 // 半帧超时: 组装缓冲有残帧且超时无新字节则丢弃

typedef struct
{
    u16 wr;    // 组装缓冲已累积字节数
    u32 ticks; // 最近收到字节时刻
    u8 buf[JMS581_RX_BUF_SIZE];
} jms581_frame_rx_t;

static jms581_frame_rx_t frame_rx;
static u8 frame_txbuf[JMS581_TX_BUF_SIZE];

static const char frame_sign[4] = {'I', '4', 'S', '0'};

// 8位累加校验和: Byte0~8 + Byte10~末尾, Byte9自身不参与
static u8 jms581_sum_calc(const u8 *frame, u16 frame_len)
{
    u8 sum = 0;

    for (u16 i = 0; i < frame_len; i++)
    {
        if (i != 9)
        {
            sum += frame[i];
        }
    }
    return sum;
}

u8 jms581_frame_tx(u8 cmd, u8 sub_cmd, const u8 *payload, u16 payload_len)
{
    u16 flen = JMS581_HEADER_LEN + payload_len;

    if (flen > JMS581_TX_BUF_SIZE)
    {
        return 0;
    }
    memcpy(frame_txbuf, frame_sign, 4);
    frame_txbuf[4] = (u8)flen;
    frame_txbuf[5] = (u8)(flen >> 8);
    frame_txbuf[6] = JMS581_DIR_MCU2DEV;
    frame_txbuf[7] = cmd;
    frame_txbuf[8] = sub_cmd;
    if (payload_len)
    {
        memcpy(&frame_txbuf[JMS581_HEADER_LEN], payload, payload_len);
    }
    frame_txbuf[9] = jms581_sum_calc(frame_txbuf, flen);
    return (bsp_uart1_bufs_tx(frame_txbuf, flen) == ERR_UART_SUCCESS);
}

// 重同步: 坏帧头/坏长度/坏校验统一走这里, 丢弃1字节后找下一个"I4S0"(完整或尾部前缀命中)
static void jms581_frame_resync(void)
{
    u16 i;

    for (i = 1; i < frame_rx.wr; i++)
    {
        if (memcmp(&frame_rx.buf[i], frame_sign, MIN(frame_rx.wr - i, 4)) == 0)
        {
            break;
        }
    }
    memmove(frame_rx.buf, &frame_rx.buf[i], frame_rx.wr - i); // i==wr时清空
    frame_rx.wr -= i;
}

void jms581_frame_process(void)
{
    // 环形缓冲 → 组装缓冲
    if (frame_rx.wr < JMS581_RX_BUF_SIZE)
    {
        u16 n = bsp_uart1_read(&frame_rx.buf[frame_rx.wr], JMS581_RX_BUF_SIZE - frame_rx.wr);
        if (n)
        {
            frame_rx.wr += n;
            frame_rx.ticks = tick_get();
        }
    }

    // 半帧超时: 残帧长时间收不齐则丢弃
    if (frame_rx.wr && tick_check_expire(frame_rx.ticks, JMS581_FRAME_TIMEOUT_MS))
    {
        frame_rx.wr = 0;
    }

    // 拆帧循环: 一次可吐出多帧(粘包); 状态由缓冲长度隐式表达(找头/收头/收体)
    while (frame_rx.wr)
    {
        WDT_CLR();
        if (memcmp(frame_rx.buf, frame_sign, MIN(frame_rx.wr, 4)))
        { // 帧头(或前缀)不符
            jms581_frame_resync();
            continue;
        }
        if (frame_rx.wr < JMS581_HEADER_LEN)
        { // 头未收齐, 等下一轮
            break;
        }
        u16 flen = frame_rx.buf[4] | ((u16)frame_rx.buf[5] << 8); // data_len: 整帧长含头, 小端
        if (flen < JMS581_HEADER_LEN || flen > JMS581_FRAME_MAX)
        {
            jms581_frame_resync();
            continue;
        }
        if (frame_rx.wr < flen)
        { // 帧体未收齐, 等下一轮
            break;
        }
        if (jms581_sum_calc(frame_rx.buf, flen) != frame_rx.buf[9])
        {
            jms581_frame_resync();
            continue;
        }
        if (frame_rx.buf[6] == JMS581_DIR_DEV2MCU)
        { // 方向不符则整帧丢弃
            jms581_proto_frame_input(frame_rx.buf[7], frame_rx.buf[8],
                                     &frame_rx.buf[JMS581_HEADER_LEN], flen - JMS581_HEADER_LEN);
        }
        memmove(frame_rx.buf, &frame_rx.buf[flen], frame_rx.wr - flen); // 吐出本帧, 保留粘包余量
        frame_rx.wr -= flen;
    }
}
#endif // JMS581_EN
