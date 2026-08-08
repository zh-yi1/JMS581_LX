#include "include.h"

#if BSP_UART_EN

#if BSP_UART1_EN
#define UART1_BAUD              115200
#define UART1_RXBUF_SIZE        128      //接收环形缓冲区大小, >256需将w_cnt/r_cnt改为u16

static u8 uart1_rxbuf[UART1_RXBUF_SIZE];
bsp_uart_t bsp_uart1;

AT(.com_text.uart)
void bsp_uart1_isr(uint8_t *buf, uint32_t len)
{
    if (tick_check_expire(bsp_uart1.ticks, 300)) {  //超时处理数据, 直接清空buf
        bsp_uart1.w_cnt = bsp_uart1.r_cnt = 0;
    }
    bsp_uart1.ticks = tick_get();

    for (u8 i = 0; i < len; i++) {
        bsp_uart1.rxbuf[bsp_uart1.w_cnt] = *buf;
        bsp_uart1.w_cnt = (bsp_uart1.w_cnt + 1) % bsp_uart1.rxbuf_len;
        buf++;
    }
}

///TX
void bsp_uart1_str_tx(char *str)
{
    uart_str_tx(UART_TYPE_1, str);
}

void bsp_uart1_putchar(char ch)
{
    uart_buf_tx(UART_TYPE_1, ch);
}

///RX
AT(.com_text.uart)
u8 bsp_uart1_get_char(u8 *ch)
{
    if (bsp_uart1.r_cnt != bsp_uart1.w_cnt) {
        *ch = bsp_uart1.rxbuf[bsp_uart1.r_cnt];
        bsp_uart1.r_cnt = (bsp_uart1.r_cnt + 1) % bsp_uart1.rxbuf_len;
        return 1;
    }
    return 0;
}

AT(.com_text.uart)
u16 bsp_uart1_rxcnt_get(void)
{
    return (bsp_uart1.w_cnt + bsp_uart1.rxbuf_len - bsp_uart1.r_cnt) % bsp_uart1.rxbuf_len;
}

u16 bsp_uart1_read(u8 *buf, u16 len)
{
    u16 rx_len = bsp_uart1_rxcnt_get();

    if (len > rx_len) {
        len = rx_len;
    }
    if (len) {
        u16 tail = bsp_uart1.rxbuf_len - bsp_uart1.r_cnt;   //读指针到缓冲区末尾的连续段
        if (len <= tail) {
            memcpy(buf, &bsp_uart1.rxbuf[bsp_uart1.r_cnt], len);
        } else {
            memcpy(buf, &bsp_uart1.rxbuf[bsp_uart1.r_cnt], tail);
            memcpy(buf + tail, bsp_uart1.rxbuf, len - tail);
        }
        bsp_uart1.r_cnt = (bsp_uart1.r_cnt + len) % bsp_uart1.rxbuf_len;
    }
    return len;
}

void bsp_uart1_rxclr(void)
{
    bsp_uart1.w_cnt = bsp_uart1.r_cnt = 0;
}

///config
void bsp_uart1_set_baud(u32 baud)
{
    uart_baud_set(UART_TYPE_1, baud);
}

Err_Uart bsp_uart1_init(uart_t *uart, u8 *rxbuf, u16 rxbuf_len)
{
    memset(&bsp_uart1, 0, sizeof(bsp_uart1));
    uart->rx_isr = bsp_uart1_isr;
    if (rxbuf && rxbuf_len) {
        bsp_uart1.rxbuf = rxbuf;
        bsp_uart1.rxbuf_len = rxbuf_len;
    }
    return uart_init(uart);
}

//测试: 收到"12345"回复"67890"
void bsp_uart1_process(void)
{
    static const char match_str[] = "12345";
    static u8 match_idx = 0;
    u8 ch;

    while (bsp_uart1_get_char(&ch)) {
        WDT_CLR();
        if (ch == match_str[match_idx]) {
            match_idx++;
            if (match_str[match_idx] == '\0') {
                match_idx = 0;
                bsp_uart1_str_tx("67890");
            }
        } else {
            match_idx = (ch == match_str[0]) ? 1 : 0;
        }
    }
}

//串口1初始化: TX: PB8, RX: PB9, 中断接收
static void uart1_port_init(void)
{
    uart_t uart1;
    memset(&uart1, 0x00, sizeof(uart_t));

    uart1.type = UART_TYPE_1;
    uart1.tx_map = UT1TXMAP_G2_PB8;
    uart1.rx_map = UT1RXMAP_G2_PB9;
    uart1.baud = UART1_BAUD;
    bsp_uart1_init(&uart1, uart1_rxbuf, sizeof(uart1_rxbuf));
}
#endif  // BSP_UART1_EN

void bsp_uart_init(void)
{
#if BSP_UART1_EN
    uart1_port_init();
#endif
}
#endif  //BSP_UART_EN
