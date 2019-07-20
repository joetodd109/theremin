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

#define MIN_FREQ        65
#define MAX_FREQ        3000
#define SAMPLE_RATE     48000
#define WAVETABLE_LEN   4096
#define BUF_LEN         512

#define AMPLITUDE       1000.0f
#define MEAN_LENGTH     5

typedef enum {
    mems_idle,
    mems_read,
    mems_reading,
} mems_status_t;

static uint16_t phase;
static float wavetable[WAVETABLE_LEN];
static int16_t spi_tx_buffer_zero[BUF_LEN];
static int16_t spi_tx_buffer_one[BUF_LEN];
static float average_frequency[MEAN_LENGTH];
static uint8_t x_axis;
static uint8_t y_axis;
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
populate_wavetable(void)
{
    float phase = 0;
    float phase_delta = _2PI / (float)WAVETABLE_LEN;

    for (int i = 0; i < WAVETABLE_LEN; i++) {
        wavetable[i] = arm_sin_f32(phase);
        phase += phase_delta;
    }
}

static void
populate_buffer(int16_t *buffer, float frequency)
{
    float phase_delta = (float) WAVETABLE_LEN / SAMPLE_RATE * frequency;

    for (int i = 0; i < BUF_LEN; i++) {
        buffer[i] = (int16_t)(AMPLITUDE * wavetable[phase]);
        phase = (uint16_t)(phase + phase_delta) % WAVETABLE_LEN;
    }
}

/* Main -----------------------------------------------------------------------*/
int main(void)
{
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

    phase = 0;
    count = 0;
    mems_status = mems_idle;
    frequency = 440.0f;
    led_flash = false;
    iox_led_on(false, false, false, false);

    for (int i = 0; i < MEAN_LENGTH; i++) {
        average_frequency[i] = frequency;
    }

    populate_wavetable();
    populate_buffer(spi_tx_buffer_zero, frequency);
    populate_buffer(spi_tx_buffer_one, frequency);

    /*
     * Start the I2S DMA in double buffer mode
     */
    spi_i2s_start_dma(spi_tx_buffer_zero, spi_tx_buffer_one, BUF_LEN);

    while(1)
    {
        /*
         * Shift down the frequency history,
         * and add add latest value.
         */
        for (int i = 0; i < MEAN_LENGTH - 1; i++) {
            average_frequency[i] = average_frequency[i + 1];
        }
        average_frequency[MEAN_LENGTH - 1] = 100.0 + 10.0 * x_axis;;
        /*
         * Smooth out the values.
         */
        frequency = 0;
        for (int i = 0; i < MEAN_LENGTH; i++) {
            frequency += average_frequency[i];
        }
        frequency /= MEAN_LENGTH;

        /*
         * Populate the currently unused buffer with
         * the latest detected frequency
         */
        curr_buf = spi_i2s_get_current_memory();
        buffer = curr_buf == buf_one ?
            spi_tx_buffer_one : spi_tx_buffer_zero;
        while (spi_i2s_get_current_memory() == curr_buf);
        populate_buffer(buffer, frequency);

        /*
         * Read the value from the magnetometer.
         */
        if (mems_status == mems_read) {
            magneto_read();
            update_leds();
        }

        /*
         * Wait until we switch to the other buffer, then
         * copy the new values over.
         */
        curr_buf = spi_i2s_get_current_memory();
        buffer = curr_buf == buf_one ?
            spi_tx_buffer_one : spi_tx_buffer_zero;
        while (spi_i2s_get_current_memory() == curr_buf);
        populate_buffer(buffer, frequency);

        count++;
    }
}
