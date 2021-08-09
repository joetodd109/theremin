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
#include "maths/arm_math.h"
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

#define MIN_FREQ        200
#define MAX_FREQ        1200
#define SAMPLE_RATE     48000
#define WAVETABLE_LEN   8192
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
static double x_angle;
static double y_angle;
static uint32_t count;

static mems_status_t mems_status;
static spi_buf_t curr_buf;

static void
magneto_read(void)
{
    int16_t xyz[3];

    mems_status = mems_reading;
    mems_magneto_read(xyz);

    // https://www.hobbytronics.co.uk/accelerometer-info
    x_angle = atan(xyz[0] / sqrt(pow(xyz[1], 2) + pow(xyz[2], 2)));
    y_angle = atan(xyz[1] / sqrt(pow(xyz[0], 2) + pow(xyz[2], 2)));

    mems_status = mems_idle;
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
        buffer[i] = (int16_t)(y_angle * AMPLITUDE * wavetable[phase]);
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
    frequency = 440.0f;
    mems_status = mems_idle;
    iox_led_on(false, false, false, false);

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
        uint8_t status[1];
        (void) i2c_read(MAG_I2C_ADDRESS, LSM303DLHC_SR_REG_M, status, 1);
        if (status[0] & 1u) {
            iox_led_on(true, false, false, false);
            magneto_read();

            float new_frequency = MIN_FREQ + (MAX_FREQ * x_angle);
            if (new_frequency > MIN_FREQ && new_frequency < MAX_FREQ) {
                frequency = new_frequency;
            }
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

        iox_led_on(false, false, false, false);
        count++;
    }
}
