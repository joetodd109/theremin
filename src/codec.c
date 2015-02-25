/**
 ******************************************************************************
 * @file    codec.c
 * @author  Joe Todd
 * @version
 * @date    January 2014
 * @brief   Theremin
 *
  ******************************************************************************/


/* Includes -------------------------------------------------------------------*/
#include "codec.h"

static uint32_t i2c_tx_error_cnt = 0;
static uint32_t i2c_rx_error_cnt = 0;

extern void 
codec_init(void)
{
    i2c_codec_init();

    uint32_t delaycount;
    uint8_t addr;
    uint8_t txdata[5] = {0};
    uint8_t rxdata[2] = {0xFF, 0xFF};

    GPIOD->ODR |= GPIO_ODR_ODR_4;
    delaycount = 1000000u;
    while (delaycount > 0)
    {
        delaycount--;
    }
    /* keep codec off */
    addr = CODEC_MAP_PLAYBACK_CTRL1;
    txdata[0] = 0x01;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;

    /* begin initialization sequence (p. 32) */
    addr = 0x00;
    txdata[0] = 0x99;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;    

    addr = 0x47;
    txdata[0] = 0x80;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++; 

    i2c_read(CODEC_I2C_ADDRESS, 0x32, rxdata, 1) ? false : i2c_rx_error_cnt++;

    addr = 0x32;
    txdata[0] = rxdata[0] | 0x80;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;  

    i2c_read(CODEC_I2C_ADDRESS, 0x32, rxdata, 1) ? false : i2c_rx_error_cnt++;

    addr = 0x32;
    txdata[0] = rxdata[0] & (~0x80);
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;

    addr = 0x00;
    txdata[0] = 0x00;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;
    /* end of initialization sequence */

    addr = CODEC_MAP_PWR_CTRL2;
    txdata[1] = 0xAF;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;

    addr = CODEC_MAP_PLAYBACK_CTRL1;
    txdata[0] = 0x70;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;

    addr = CODEC_MAP_CLK_CTRL;
    txdata[0] = 0x81; 
    /* auto detect clock */
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++; 

    addr = CODEC_MAP_IF_CTRL1;
    txdata[0] = 0x07;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;
 
    addr = CODEC_MAP_ANALOG_SET;
    txdata[0] = 0x00;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;

    addr = CODEC_MAP_LIMIT_CTRL1;
    txdata[0] = 0x00;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;

    addr = CODEC_MAP_PCMA_VOL | CODEC_MAPBYTE_INC;
    txdata[0] = 0x0A;
    txdata[1] = 0x0A;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 2) ? false : i2c_tx_error_cnt++;

    addr = CODEC_MAP_TONE_CTRL;
    txdata[0] = 0x0F;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++;

    addr = CODEC_MAP_PWR_CTRL1;
    txdata[0] = 0x9E;
    i2c_write(CODEC_I2C_ADDRESS, addr, txdata, 1) ? false : i2c_tx_error_cnt++; 
}
