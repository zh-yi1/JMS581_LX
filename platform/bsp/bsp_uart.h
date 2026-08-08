#ifndef _BSP_UART_H
#define _BSP_UART_H

// 是否打开串口1收发管理器 (通信串口, TX: PB8, RX: PB9)
#define BSP_UART1_EN    1
#define BSP_UART_EN     (BSP_UART1_EN)

typedef struct {
    volatile u8 w_cnt;
    volatile u8 r_cnt;
    u32 ticks;
    u8 *rxbuf;
    u16 rxbuf_len;
} bsp_uart_t;
extern bsp_uart_t bsp_uart1;

/**
 * @brief 从UART1接收数据缓冲区中获取字节数据
 * @param[in] ch: 字节指针
 * @return 成功与否
 **/
u8 bsp_uart1_get_char(u8 *ch);

/**
 * @brief 获取UART1接收数据缓冲区中未读字节数
 **/
u16 bsp_uart1_rxcnt_get(void);

/**
 * @brief 从UART1接收缓冲区批量读取数据
 * @param[in] buf: 目标缓冲区
 * @param[in] len: 期望读取字节数
 * @return 实际读取字节数 (缓冲区不足len时返回实际可读数)
 **/
u16 bsp_uart1_read(u8 *buf, u16 len);

/**
 * @brief 清空UART1接收数据缓冲区
 **/
void bsp_uart1_rxclr(void);

/**
 * @brief UART1设置波特率
 **/
void bsp_uart1_set_baud(u32 baud);

/**
 * @brief UART1发送单字节
 * @param[in] ch 数据
 **/
void bsp_uart1_putchar(char ch);

/**
 * @brief UART1发送字符串
 * @param[in] str 需发送的字符串
 **/
void bsp_uart1_str_tx(char *str);

/**
 * @brief 初始化UART1
 * @param[in] uart: 初始化结构体
 * @param[in] rxbuf: 接收数据缓冲区, 若有则使用默认的接收管理器
 * @param[in] rxbuf_len: 接收数据缓冲区长度
 **/
Err_Uart bsp_uart1_init(uart_t *uart, u8 *rxbuf, u16 rxbuf_len);

/**
 * @brief UART1接收处理, 在主循环中轮询调用
 **/
void bsp_uart1_process(void);

void bsp_uart_init(void);
#endif
