#ifndef _TFT_H
#define _TFT_H

#include "tft_drv.h"

typedef struct {
    //TE�������
    bool tft_bglight_kick;      //�������
    u8   tft_bglight_duty;      //����pwmռ�ձ�
    u8   tft_bglight_last_duty; //����pwm��һ��ռ�ձ�
    u8 te_mode;
    u8 te_mode_next;
    bool tft_bglight_first_set;

    u8 te_bglight_cnt;          //���յ���Ҫ�򿪱������ʱ�������һ֡���ݺ���ʱ�򿪱���
    u8 despi_baud;
    u8 despi_baud1;
    u8 despi_baud2;
    bool flag_in_frame;
    bool tft_set_baud_kick;     //��Ҫ�л�ʱ�ӣ���TFT_END������
} tft_cb_t;


typedef enum {
    LCD_IO_3WIRE_9BIT,
    LCD_IO_3WIRE_9BIT_2LINE,
    LCD_IO_4WIRE_8BIT,
    LCD_IO_QSPI,
    LCD_IO_I8080,
    LCD_IO_SRGB8,
    LCD_IO_PRGB16,
}lcd_io_type;

#define GUI_COLOR_DEPTH                 2                     //gui �ײ���ɫ֧�����, �п����GUI��ɫ����565ɫ, �����Ը��

enum {
    LCD_SELECT_XOSC_CLK     = 0,
    LCD_SELECT_XOSC_X2CLK,
    LCD_SELECT_PLL0_DIV2,
    LCD_SELECT_PLL0_DIV3,
    LCD_SELECT_PLL1_DIV1,
    LCD_SELECT_PLL1_DIV2,
};

enum {
    LCD_SELECT_RGB_8BIT = 0,            //SRGBʱ��: VsyncΪ֡ͬ���ź�, ��ʾ֡��ʼ. Hsync Ϊ��ͬ���ź�, ��ʾ�п�ʼ. DE Ϊ������Чʹ��, ��ʾ������Ч, ÿһ��CLK����, ����һ����������D[7:0], ÿ3��CLKΪһ������RGB, �Դ����һ֡ͼ��������ʾ.
    LCD_SELECT_RGB_16BIT,               //PRGBʱ��: VsyncΪ֡ͬ���ź�, ��ʾ֡��ʼ. Hsync Ϊ��ͬ���ź�, ��ʾ�п�ʼ. DE Ϊ������Чʹ��, ��ʾ������Ч, ÿһ��CLK����, ����һ����������D[23:0], �Դ����һ֡ͼ��������ʾ
};

enum {
    LCD_IN_RGB565,
    LCD_IN_RGB888,
    LCD_IN_ARGB8565,
    LCD_IN_ARGB8888,
};

enum {
    LCD_OUT_RGB565,
    LCD_OUT_BGR565,
    LCD_OUT_RGB888,
    LCD_OUT_BGR888,
};

#if (GUI_COLOR_DEPTH == 2)
typedef union {
    struct {
        u16 b : 5;
        u16 g : 6;
        u16 r : 5;
    } PACKED ch;
    u16 full;
} PACKED lcd_color_t;
#elif (GUI_COLOR_DEPTH == 3)
typedef union rgb888_t_ {
    struct {
        u8 b;
        u8 g;
        u8 r;
    } PACKED ch;
    u32 full : 24;
} PACKED lcd_color_t;
#else
#error GUI_COLOR_DEPTH undefine !!!
#endif

typedef union {
    struct {
        union {
            struct {
                u32 dc;
                u32 cs;
                u32 rst;
                u32 te;
                u32 scl;
            } cio;
        } control;


        union {
            struct {
                u32 d0;
                u32 d1;
                u32 d2;
                u32 d3;
                u32 d4;
                u32 d5;
                u32 d6;
                u32 d7;
            } dio;
            u32 group[8];
        } data;

//        u32 rev[7];
    } spi_drv_io;

    struct {
        union {
            struct {
                u32 d0;
                u32 d1;
                u32 d2;
                u32 d3;
                u32 d4;
                u32 d5;
                u32 d6;
                u32 d7;
                u32 d8;
                u32 d9;
                u32 d10;
                u32 d11;
                u32 d12;
                u32 d13;
                u32 d14;
                u32 d15;
            } dio;
            u32 group[16];
        } data;

        union {
            struct {
                u32 display;
                u32 de;
                u32 clk;
                u32 vsync;
                u32 hsync;
            } cio;
        } control;
    } rgb_drv_io;
} lcd_drv_io_t;

typedef struct _lcd_drv_t {
    lcd_io_type     io_type;
    lcd_drv_io_t    io;
    union {
        struct {
            u32 clk_select;
            u32 clk_div;
            u32 speed1_mhz;
            u32 speed2_mhz;
        } spi_drv_param;

        struct {
            u32 clk_select;
            u32 clk_div;
            u32 in_rgb;
            u32 out_rgb;
            u32 vsync_width;
            u32 vfront_proch;
            u32 vback_porch;
            u32 hsync_width;
            u32 hfront_porch;
            u32 hback_porch;
            lcd_color_t *refersh_dma_buf;       //������ɫ��ʾ�ڴ�
            u32 refersh_dma_buf_size;           //������ɫ��ʾ�ڴ��С
        } rgb_drv_param;
    } param;
    void (*tft_te_isr)(void);
    void (*tft_reg_init)(void);
    void (*tft_set_window)(u16 x0, u16 y0, u16 x1, u16 y1);
    u32  (*tft_read_id)(void);
    void (*tft_set_brightness)(u8 brightness);
}lcd_drv_t;

/*ϵͳʹ��*/
void tft_spi_send(void *buf, uint wid, uint hei);
void tft_frame_start(void);
void tft_frame_end(void);
void tft_write_data_start(void);
void tft_write_end();
void tft_write_cmd(u8 cmd);
void tft_write_data(u8 data);
void tft_write_end(void);

void tft_write_cmd32(u8 cmd);
void tft_write_cmd42(u8 cmd);
void tft_write_cmd52(u8 cmd);

void tft_spi_sendbyte(u8 val);
u32 tft_spi_getbyte(void);

#define WriteComm(v)        tft_write_cmd(v)
#define WriteData(v)        tft_write_data(v)
#define CommEnd(v)          tft_write_end()

#define TFT_SPI_CS_DIS()      {lcd_drv_cs_out(1);}
#define TFT_SPI_CS_EN()       {lcd_drv_cs_out(0);}
#define DC_CMD_EN()           {lcd_drv_dc_out(0);}      // DC ����
#define DC_DATA_EN()          {lcd_drv_dc_out(1);}      // DC ����

/*
 * lcd drv extern
 */
//extern lcd_drv_t lcd_240_st7789V3_i80_drv;
extern lcd_drv_t lcd_320_st77916_drv;
extern lcd_drv_t lcd_oled_466_icna3310b_drv;
//extern lcd_drv_t lcd_360_gc9c01_drv;
//extern lcd_drv_t lcd_240_jd9853_3w9bit_drv;
//extern lcd_drv_t lcd_240_st7789_4w8bit_drv;
//extern lcd_drv_t lcd_128_160_jd9853_i80_drv;
extern lcd_drv_t lcd_oled_368_st7801n_drv;
extern lcd_drv_t lcd_240_st7789V3_i80_drv;
extern lcd_drv_t lcd_vga012a_drv;
extern lcd_drv_t lcd_480_st7283_drv;
extern lcd_drv_t lcd_800_st7265_drv;
extern lcd_drv_t lcd_240_jd9853_boe20_drv;

/**
 * ��ʼ��LCD
 */
void tft_init(void);

/**
 * �ر�LCD
 */
void tft_exit(void);

/**
 * ע��LCD�������ҳ�ʼ����Ļ
 */
void lcd_drv_register(lcd_drv_t *drv);

/**
 * ע��LCD�������Ұ�IO������Ϊģ��̬
 */
void lcd_drv_deregister(void);

/**
 * ����LCD���CLK
 */
void lcd_drv_clk_deregister(void);


/**
 * ��ʼ����ע���LCD����
 */
void lcd_drv_init(void);

/**
 * ������ע��LCD��
 */
void lcd_drv_set_window(u16 x0, u16 y0, u16 x1, u16 y1);

/**
 * ��ȡ��ע��LCD ID
 */
uint32_t lcd_drv_read_id(void);

/**
 * OLED ��������
 */
void lcd_drv_set_brightness(u8 brightness);

void lcd_drv_cs_out(bool is_high);

void lcd_drv_dc_out(bool is_high);

/**
 * ����TE ģʽ
 */
void tft_set_temode(u8 mode);

/**
 * ֱ�Ӵ򿪱���
 */
void tft_bglight_en(void);

/**
 * ����TE1��TE2ģʽ�µ�LCD CLK�ٶ�
   TE1ģʽ->lcd_clk = 496Mhz/4/(baud1 + 1);
   TE2ģʽ->lcd_clk = 496Mhz/4/(baud2 + 1);
 */
void tft_set_baud(u8 baud1, u8 baud2);

/**
 * @brief ����oled����
 * @param[in] level       ���ȵȼ���ʹ���޼�����ʱ����Χ0 ~ 100������Ϊ1~5
 * @param[in] stepless_en �Ƿ�ʹ���޼�����
 *
 * @return  ��
 **/
void oled_brightness_set_level(uint8_t level, bool stepless_en);

/**
 * @brief ���ñ�������
 * @param[in] level       ���ȵȼ���ʹ���޼�����ʱ����Χ0 ~ 100������Ϊ1~5
 * @param[in] stepless_en �Ƿ�ʹ���޼�����
 *
 * @return  ��
 **/
void tft_bglight_set_level(uint8_t level, bool stepless_en);

/**
 * @brief �״��������ȼ��
 * @param ��
 *
 * @return  ��
 **/
void tft_bglight_frist_set_check(void);

void tft_te_isr(void);

#endif
