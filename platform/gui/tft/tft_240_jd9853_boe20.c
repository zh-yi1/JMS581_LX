#include "include.h"
// JD9853 BOE 2.0寸 IPS (GV020QVQ-N81) 240 * 320

#if (GUI_SELECT == GUI_TFT_JD9853_BOE_2IN0)

static void tft_240_jd9853_boe20_init(void)
{
    printf("tft_240_jd9853_boe20_init\n");

    // Software reset
    WriteComm(0x01);
    delay_ms(25);

    // Password
    WriteComm(0xDF);
    WriteData(0x98);
    WriteData(0x53);

    WriteComm(0xDE);
    WriteData(0x00);

    WriteComm(0xCE);
    WriteData(0x6A);
    WriteData(0x00);    // 00 default SDA IN/OUT; 20=D0 OUT

    // Vcom
    WriteComm(0xB2);
    WriteData(0x24);

    // Gamma set
    WriteComm(0xB7);
    WriteData(0x00);
    WriteData(0x21);
    WriteData(0x00);
    WriteData(0x49);

    // DCDC select
    WriteComm(0xBB);
    WriteData(0x1E);
    WriteData(0x2F);
    WriteData(0x55);
    WriteData(0x71);
    WriteData(0x73);
    WriteData(0xF0);

    // Set STBA
    WriteComm(0xC0);
    WriteData(0x24);
    WriteData(0x24);

    // Set panel
    WriteComm(0xC1);
    WriteData(0x12);

    // Set RGB cycle
    WriteComm(0xC3);
    WriteData(0x7D);
    WriteData(0x08);
    WriteData(0x0A);
    WriteData(0x0C);
    WriteData(0xC4);
    WriteData(0x73);
    WriteData(0x22);
    WriteData(0x77);

    // Set TCON (60Hz)
    WriteComm(0xC4);
    WriteData(0x00);    // 00=60Hz 04=57Hz 08=51Hz
    WriteData(0x00);
    WriteData(0xA0);    // LN=320
    WriteData(0x79);
    WriteData(0x0A);
    WriteData(0x0B);
    WriteData(0x16);
    WriteData(0x79);
    WriteData(0x0A);
    WriteData(0x0B);
    WriteData(0x16);
    WriteData(0x82);

    // Gamma G2.2
    WriteComm(0xC8);
    WriteData(0x3F);
    WriteData(0x32);
    WriteData(0x2A);
    WriteData(0x24);
    WriteData(0x29);
    WriteData(0x2B);
    WriteData(0x26);
    WriteData(0x24);
    WriteData(0x23);
    WriteData(0x22);
    WriteData(0x21);
    WriteData(0x15);
    WriteData(0x11);
    WriteData(0x0B);
    WriteData(0x04);
    WriteData(0x00);
    WriteData(0x3F);
    WriteData(0x32);
    WriteData(0x2A);
    WriteData(0x24);
    WriteData(0x29);
    WriteData(0x2B);
    WriteData(0x26);
    WriteData(0x24);
    WriteData(0x23);
    WriteData(0x22);
    WriteData(0x21);
    WriteData(0x15);
    WriteData(0x11);
    WriteData(0x0B);
    WriteData(0x04);
    WriteData(0x00);

    // Set GD
    WriteComm(0xD0);
    WriteData(0x04);
    WriteData(0x04);
    WriteData(0x6C);
    WriteData(0x1C);
    WriteData(0x03);

    // RAM control (3WIRE SPI mode)
    WriteComm(0xD7);
    WriteData(0x00);    // SPI3W2DL_OPT=0x00
    WriteData(0x30);

    WriteComm(0xE6);
    WriteData(0x10);

    // Page1
    WriteComm(0xDE);
    WriteData(0x01);

    // DCDC option
    WriteComm(0xB7);
    WriteData(0x03);
    WriteData(0x13);
    WriteData(0xE5);
    WriteData(0x38);
    WriteData(0x38);

    // Set RGB cycle 2
    WriteComm(0xC1);
    WriteData(0x14);
    WriteData(0x15);
    WriteData(0xC0);

    WriteComm(0xC2);
    WriteData(0x06);
    WriteData(0x3A);

    // Set gamma option
    WriteComm(0xC4);
    WriteData(0x72);
    WriteData(0x12);

    // Gamma power test
    WriteComm(0xBE);
    WriteData(0x00);

    // Page0
    WriteComm(0xDE);
    WriteData(0x00);

    // TE ON
    WriteComm(0x35);
    WriteData(0x00);

    // MADCTL: landscape, 9 o'clock (90° counter-clockwise)
    WriteComm(0x36);
    WriteData(0xA0);    // 00=竖屏; 60=横屏3点; A0=横屏9点

    // Color format: RGB565
    WriteComm(0x3A);
    WriteData(0x05);    // 06=RGB666; 05=RGB565

    // Column address: 0 ~ 239
    WriteComm(0x2A);
    WriteData(0x00);
    WriteData(0x00);    // Start_X = 0
    WriteData(0x00);
    WriteData(0xEF);    // End_X = 239

    // Row address: 0 ~ 319
    WriteComm(0x2B);
    WriteData(0x00);
    WriteData(0x00);    // Start_Y = 0
    WriteData(0x01);
    WriteData(0x3F);    // End_Y = 319

    // Sleep out
    WriteComm(0x11);
    CommEnd();
    delay_ms(120);

    // Display on
    WriteComm(0x29);
    CommEnd();
    delay_ms(10);

    // Lock password
    WriteComm(0xDF);
    WriteData(0x00);
    WriteData(0x00);
    CommEnd();
}

AT(.com_text.tft_spi)
static void tft_240_jd9853_boe20_set_window(u16 x0, u16 y0, u16 x1, u16 y1)
{
    x0 += GUI_SCREEN_OFS_X;
    x1 += GUI_SCREEN_OFS_X;
    y0 += GUI_SCREEN_OFS_Y;
    y1 += GUI_SCREEN_OFS_Y;

    // MADCTL MV=1: hardware swaps X/Y, column range becomes 0..319, row range 0..239
    tft_write_cmd(0x2A);        // TFT_CASET
    tft_write_data(BYTE1(x0));
    tft_write_data(BYTE0(x0));
    tft_write_data(BYTE1(x1));
    tft_write_data(BYTE0(x1));

    tft_write_cmd(0x2B);        // TFT_PASET
    tft_write_data(BYTE1(y0));
    tft_write_data(BYTE0(y0));
    tft_write_data(BYTE1(y1));
    tft_write_data(BYTE0(y1));
    tft_write_end();
}

static void tft_read_id_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    LCDSPICON = (LCDSPICON & ~(0x03<<2)) | (0x01<<2);       //1data in 1data out
    tft_spi_sendbyte(0x03);
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

static uint32_t tft_read_id(void)
{
    u32 lcdcon_bak = LCDCON;
    u32 lcdspicon_bak = LCDSPICON;
    uint32_t id = 0;

    tft_read_id_cmd(0x04);
    id = tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    tft_write_end();
    LCDCON = lcdcon_bak;
    LCDSPICON = lcdspicon_bak;
    return id;
}

static void tft_240_jd9853_boe20_set_brightness(uint8_t brightness)
{
    tft_cb_t* tft_get_tft_cb(void);
    tft_cb_t *tft_cb = tft_get_tft_cb();

    int8_t  base_duty = 0;
    int8_t  duty = 0;

    if (100 < brightness) {
        brightness = 100;
    }
    duty = base_duty + brightness;
    tft_cb->tft_bglight_duty = duty;

    if (tft_cb->tft_bglight_last_duty != tft_cb->tft_bglight_duty) {
        bsp_pwm_duty_set(PORT_TFT_BL, tft_cb->tft_bglight_duty, false);
        tft_cb->tft_bglight_last_duty = tft_cb->tft_bglight_duty;
    }
}

lcd_drv_t lcd_240_jd9853_boe20_drv = {
    .io = {
        .spi_drv_io = {
            .data = {
                .dio = {
                    .d0 = PORT_TFT_LCD_D0,
                    .d1 = PORT_TFT_LCD_D1,
                    .d2 = PORT_TFT_LCD_D2,
                    .d3 = PORT_TFT_LCD_D3,
                    .d4 = PORT_TFT_LCD_D4,
                    .d5 = PORT_TFT_LCD_D5,
                    .d6 = PORT_TFT_LCD_D6,
                    .d7 = PORT_TFT_LCD_D7,
                }
            },

            .control = {
                .cio = {
                    .dc = PORT_TFT_DC,
                    .cs = PORT_TFT_CS,
                    .rst = PORT_TFT_RST,
                    .te = PORT_TFT_INT,
                    .scl = PORT_TFT_LCD_SCL,
                }
            },
        },
    },

    .io_type        = LCD_IO_QSPI,
    .param          = {
        .spi_drv_param = {
            .clk_select = LCD_SELECT_PLL0_DIV2,
            .clk_div = 0,
            .speed1_mhz = 27,
            .speed2_mhz = 20,
        },
    },
    .tft_reg_init   = tft_240_jd9853_boe20_init,
    .tft_set_window = tft_240_jd9853_boe20_set_window,
    .tft_read_id    = tft_read_id,
    .tft_set_brightness = tft_240_jd9853_boe20_set_brightness,
    .tft_te_isr     = tft_te_isr,
};
#endif
