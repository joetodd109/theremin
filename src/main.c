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

#define _2PI            6.283185307f
#define _PI             3.14159265f
#define _INVPI          0.3183098861f

#define MIN_FREQ        65
#define MAX_FREQ        3000
#define SAMPLE_RATE     48000
#define WAVETABLE_LEN   4096
#define BUF_LEN         1024

#define AMPLITUDE       3000.0f

typedef enum {
    mems_idle,
    mems_read,
    mems_reading,
} mems_status_t;

static float phase;
static float phase_delta;
static float wavetable[WAVETABLE_LEN];
static int16_t spi_tx_buffer_zero[BUF_LEN];
static int16_t spi_tx_buffer_one[BUF_LEN];
static uint8_t x_axis;
static uint8_t y_axis;
static uint8_t x_axis_prev;
static uint8_t x_axis_drift;
static uint32_t count;
static bool led_flash;

static mems_status_t mems_status;
static spi_buf_t curr_buf;

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

static void
populate_buffer(int16_t *buffer, float frequency)
{
    phase_delta = BUF_LEN / SAMPLE_RATE * frequency;

    for (int i = 0; i < BUF_LEN; i++) {
        buffer[i] = AMPLITUDE * wavetable[(int)phase];
        phase = (int)(phase + phase_delta) % BUF_LEN;
    }
}

/* Main -----------------------------------------------------------------------*/
int main(void)
{
    int i;
    float frequency;
    int16_t *buffer;
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
    mems_status = mems_idle;
    frequency = 440.0f;
    led_flash = false;
    iox_led_on(false, false, false, false);

    /*
     * Populate the wavetable
     */
    phase = 0;
    phase_delta = _2PI / (float)WAVETABLE_LEN;

    for (i = 0; i < WAVETABLE_LEN; i++) {
        wavetable[i] = arm_sin_f32(phase);
        phase += phase_delta;
    }

    populate_buffer(spi_tx_buffer_zero, frequency);
    populate_buffer(spi_tx_buffer_one, frequency);

    /*
     * Start the I2S DMA in double buffer mode
     */
    spi_i2s_start_dma(spi_tx_buffer_zero, spi_tx_buffer_one, BUF_LEN);
    phase = 0;

    while(1)
    {
        if (x_axis > x_axis_prev && frequency < MAX_FREQ) {
            x_axis_drift++;
        }
        if (x_axis < x_axis_prev && frequency > MIN_FREQ) {
            x_axis_drift--;
        }

        frequency = 20.0f * x_axis_drift;

        /*
         * Populate the currently unused buffer with
         * the latest detected frequency
         */
        curr_buf = spi_i2s_get_current_memory();
        buffer = curr_buf == buf_one ?
            spi_tx_buffer_zero : spi_tx_buffer_one;
        populate_buffer(buffer, frequency);

        /*
         * Wait until we switch to the other buffer, then
         * copy the new values over.
         */
        while (spi_i2s_get_current_memory() == curr_buf);
        curr_buf = spi_i2s_get_current_memory();
        buffer = curr_buf == buf_one ?
            spi_tx_buffer_zero : spi_tx_buffer_one;
        populate_buffer(buffer, frequency);

        x_axis_prev = x_axis;
        if (mems_status == mems_read) {
            magneto_read();
            update_leds();
        }

        count++;
    }
}
