/*******************************************************************************
 * @file    main.c
 * @author  Joe Todd
 * @version
 * @date    January 2014
 * @brief   Theremin
 *
  ******************************************************************************/


/* Includes -------------------------------------------------------------------*/
#include "math.h"
#include "arm_math.h"
#include "rcc.h"
#include "iox.h"
#include "spi.h"
#include "dma.h"
#include "codec.h"
#include "mems.h"
#include "i2c.h"
#include "utl.h"
#include "timer.h"
#include "uart.h"

/* Prototypes -----------------------------------------------------------------*/

#define _2PI        6.283185307f
#define _PI         3.14159265f
#define _INVPI      0.3183098861f

#define MIN_FREQ    65.0f
#define MAX_FREQ    3000.0f
#define SAMPLE_RATE 48000.0f
#define BUF_LEN     512

#define AMPLITUDE   3000.0f

typedef enum {
    buf_zero,
    buf_one,
} pbuf_t;

typedef enum {
    mems_idle,
    mems_read,
    mems_reading,
} mems_status_t;

static float32_t wave;
static float32_t tc;
static float32_t tcs;
static float32_t smpr;

static int16_t spi_tx_buffer_zero[BUF_LEN];
static int16_t spi_tx_buffer_one[BUF_LEN];
static uint8_t x_axis;
static uint8_t y_axis;
static uint8_t x_axis_prev;
static uint32_t count;
static bool led_flash;

static mems_status_t mems_status;
static pbuf_t curr_buf;

static void magneto_read(void);
static void update_leds(void);

static void
magneto_read(void)
{
    uint8_t mems_buffer[6];

    mems_status = mems_reading;
    i2c_read(MAG_I2C_ADDRESS, LSM303DLHC_OUT_X_H_M, mems_buffer, 6);
    x_axis = mems_buffer[1];
    y_axis = mems_buffer[3];

    if (x_axis > 128) {
        x_axis -= 128;
    }
    else if (x_axis < 128) {
        x_axis += 128;
    }
    if (y_axis > 128) {
        y_axis -= 128;
    }
    else if (y_axis < 128) {
        y_axis += 128;
    }
    mems_status = mems_idle;
}

static void
update_leds(void)
{
    if ((x_axis >= 0) && (x_axis < 64)) {
        iox_led_on(true, false, false, false);
    }
    else if ((x_axis >= 64) && (x_axis < 128)) {
        iox_led_on(true, true, false, false);
    }
    else if ((x_axis >= 128) && (x_axis < 192)) {
        iox_led_on(true, true, true, false);
    }
    else if ((x_axis >= 192) && (x_axis < 256)) {
        iox_led_on(true, true, true, true);
    }
    else {
        iox_leds_off();
    }
}

extern void
set_mems_read(void)
{
    mems_status = mems_read;
}

/* Main -----------------------------------------------------------------------*/
int main(void)
{
    int i;
    curr_buf = buf_zero;

    fpu_on();
    clk_init();
    i2s_clk_init();

    iox_led_init();
    spi_i2s_init();
    codec_init();
    dma_init();
    mems_init();
    uart_init(31250);
    timer_init();

    count = 0;
    wave = 0.0f;
    tc = 0.0f;
    x_axis_prev = 1;
    mems_status = mems_idle;
    led_flash = false;
    iox_led_on(false, false, false, false);

    spi_i2s_start_dma(spi_tx_buffer_zero, spi_tx_buffer_one, BUF_LEN);

    while(1)
    {
        if (x_axis_prev != x_axis) {
            tc = tcs;
            smpr = 300.0f - x_axis;
            tcs = _2PI / smpr;

            led_flash = !led_flash;
            iox_led_on(false, false, false, led_flash);

            /*
             * Write new wavetable to the DMA buffer
             * that is not currently being read from
             */
            // uint32_t current_buffer = spi_i2s_get_current_memory();
            // int16_t *buffer = current_buffer == 1 ?
            //     spi_tx_buffer_zero : spi_tx_buffer_one;

            for (i = 0; i < (uint16_t)smpr; i++) {

                wave = arm_sin_f32(tc);
                spi_tx_buffer_one[i] = (int16_t)(AMPLITUDE * wave);

                tc += tcs;
                if (tc > _2PI) {
                    tc -= _2PI;
                }
            }

            /*
             * Wait until current DMA cycle finished,
             * and set the length of the new buffer.
             */
            // if (current_buffer == 1) {
            while (I2S_DMA1->NDTR > 1);
            spi_i2s1_reconfigure((uint16_t)smpr);
            // } else {
                // while (I2S_DMA0->NDTR > 1);
                // spi_i2s_reconfigure((uint16_t)smpr);
            // }

            /*
             * Write new wavetable to the other DMA buffer
             */
            // for (i = 0; i < (uint16_t)smpr; i++) {
            //     if (current_buffer == 1) {
            //         spi_tx_buffer_one[i] = spi_tx_buffer_zero[i];
            //     } else {
            //         spi_tx_buffer_zero[i] = spi_tx_buffer_one[i];
            //     }
            // }

            // if (current_buffer == 1) {
            //     while (I2S_DMA0->NDTR > 1);
            //     spi_i2s_reconfigure((uint16_t)smpr);
            // } else {
            //     while (I2S_DMA1->NDTR > 1);
            //     spi_i2s1_reconfigure((uint16_t)smpr);
            // }

            x_axis_prev = x_axis;
            update_leds();
        }

        if (mems_status == mems_read) {
            magneto_read();
        }

        count++;
    }
}
