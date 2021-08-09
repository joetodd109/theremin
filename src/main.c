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

#define MIN_FREQ        200
#define MAX_FREQ        1000
#define SAMPLE_RATE     48000
#define WAVETABLE_LEN   4096
#define BUF_LEN         109

#define AMPLITUDE       500.0f

typedef enum {
    mems_idle,
    mems_read,
    mems_reading,
} mems_status_t;

typedef enum {
    dma_idle,
    dma_buf_zero_write,
    dma_buf_one_write,
} dma_status_t;

static uint16_t phase;
static float wavetable[WAVETABLE_LEN];
static int16_t spi_tx_buffer_zero[BUF_LEN];
static int16_t spi_tx_buffer_one[BUF_LEN];
static uint16_t current_length;
static int16_t x_angle;
static int16_t y_angle;
static uint32_t count;

static volatile dma_status_t dma_status;
static mems_status_t mems_status;
static spi_buf_t curr_buf;

static bool
axis_read(void)
{
    int16_t xyz[1];

    mems_status = mems_reading;
    mems_magneto_read(xyz);

    // if (xyz[0] == 0 && xyz[1] == 0 && xyz[2] == 0) {
    //     return false;
    // }

    x_angle = xyz[0];
    // double y_axis = (double)xyz[1];
    // double z_axis = (double)xyz[2];

    // https://www.hobbytronics.co.uk/accelerometer-info
    // x_angle = atan(x_axis / sqrt(pow(y_axis, 2) + pow(z_axis, 2)));
    // y_angle = atan(y_axis / sqrt(pow(x_axis, 2) + pow(z_axis, 2)));

    mems_status = mems_idle;
    return true;
}

extern uint16_t
set_buffer_zero_write(void)
{
    if (dma_status != dma_idle) {
        iox_led_on(false, false, true, false);
    }
    dma_status = dma_buf_zero_write;
    return current_length;
}

extern uint16_t
set_buffer_one_write(void)
{
    if (dma_status != dma_idle) {
        iox_led_on(false, false, true, false);
    }
    dma_status = dma_buf_one_write;
    return current_length;
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
populate_wavetable_buffer(int16_t *buffer, float frequency)
{
    float phase_delta = (float) WAVETABLE_LEN / SAMPLE_RATE * frequency;

    for (int i = 0; i < BUF_LEN; i++) {
        buffer[i] = (int16_t)(AMPLITUDE * wavetable[phase]);
        phase = (uint16_t)(phase + phase_delta) % WAVETABLE_LEN;
    }
}

static void
populate_buffer(int16_t *buffer, float frequency)
{
    float tc = 0.0;
    float tcs = _2PI * frequency / SAMPLE_RATE;
    uint16_t length = SAMPLE_RATE / frequency;

    for (int i = 0; i < length; i++) {
        buffer[i] = (int16_t)AMPLITUDE * arm_sin_f32(tc);
        tc += tcs;
        if (tc > _2PI) {
            tc -= _2PI;
        }
    }
}

/* Main -----------------------------------------------------------------------*/
int main(void)
{
    float frequency;
    float previous_frequency;
    int16_t *buffer;
    curr_buf = buf_zero;
    utl_median_filter_t median_filter;

    fpu_on();
    clk_init();
    i2s_clk_init();

    iox_led_init();
    spi_i2s_init();
    codec_init();
    dma_init();
    mems_magneto_init();
    mems_accel_init();
    uart_init(31250);
    timer_init();

    phase = 0;
    count = 0;
    mems_status = mems_idle;
    dma_status = dma_idle;
    frequency = 440.0f;
    previous_frequency = 440.0f;
    current_length = SAMPLE_RATE / frequency;

    iox_led_on(false, false, false, false);

    populate_buffer(spi_tx_buffer_zero, frequency);
    populate_buffer(spi_tx_buffer_one, frequency);

    /*
     * Start the I2S DMA in double buffer mode
     */
    spi_i2s_start_dma(spi_tx_buffer_zero, spi_tx_buffer_one, BUF_LEN);

    while(1)
    {
        /*
         * Read the value from the magnetometer
         * when it's ready for reading.
         */
        uint8_t status[1];
        (void) i2c_read(MAG_I2C_ADDRESS, LSM303DLHC_SR_REG_M, status, 1);

        // if (status[0] & 1u) {
        //     iox_led_on(true, false, false, false);
        //     axis_read();
        //     frequency = x_angle;
        // }

        if (dma_status != dma_idle) {
            iox_led_on(false, true, false, false);
            buffer = dma_status == dma_buf_zero_write ?
                spi_tx_buffer_zero : spi_tx_buffer_one;
            if (frequency < 1000) {
                frequency += 1.0f;
            } else {
                frequency = 440.0f;
            }
            populate_buffer(buffer, frequency);

            current_length = SAMPLE_RATE / frequency;
            dma_status = dma_idle;
            iox_led_on(false, false, false, false);
        }
    }

    iox_led_on(false, false, false, false);
    count++;
}
