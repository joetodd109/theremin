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

#define MIN_FREQ    65
#define MAX_FREQ    3000
#define SAMPLE_RATE 48000
#define BUF_LEN     4096

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

static float phase;
static float phase_delta;
static float wavetable[SAMPLE_RATE];
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
    float *buffer;
    uint32_t current_buffer;
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

    /*
     * Populate the wavetable
     */
    phase = 0;
    phase_delta = 2PI / (float)SAMPLE_RATE;

    for (i = 0; i < SAMPLE_RATE; i++) {
        wavetable[i] = arm_sin_f32(phase);
        phase += phase_delta;
    }

    /*
     * Start the I2S DMA in double buffer mode
     */
    spi_i2s_start_dma(spi_tx_buffer_zero, spi_tx_buffer_one, BUF_LEN);

    while(1)
    {
        phase = 0;
        phase_delta = BUF_LEN / SAMPLE_RATE * x_axis;

        /*
         * Populate the currently unused buffer with
         * the latest detected frequency
         */
        current_buffer = spi_i2s_get_current_memory();
        buffer = current_buffer == 1 ?
            spi_tx_buffer_zero : spi_tx_buffer_one;

        for (i = 0; i < BUF_LEN; i++) {
            buffer[i] = wavetable[(int)phase];
            phase = (phase + phase_delta) % BUF_LEN;
        }

        /*
         * Wait until we switch to the other buffer, then
         * copy the new values over.
         */
        while (spi_i2s_get_current_memory() == current_buffer);
        current_buffer = spi_i2s_get_current_memory();
        buffer = current_buffer == 1 ?
            spi_tx_buffer_zero : spi_tx_buffer_one;

        for (i = 0; i < BUF_LEN; i++) {
            buffer[i] = wavetable[(int)phase];
            phase = (phase + phase_delta) % BUF_LEN;
        }

        if (mems_status == mems_read) {
            magneto_read();
            update_leds();
        }

        count++;
    }
}
