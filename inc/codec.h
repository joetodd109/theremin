/**
  ******************************************************************************
  * @file    codec.h 
  * @author  
  * @version 
  * @date    
  * @brief   header for codec.c
  ******************************************************************************
*/
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CODEC_H
#define __CODEC_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "iox.h"
#include "spi.h"
#include "i2c.h"

 /* Global Defines ---------------------------------------------------------- */

/* 
 * Pins to DAC
 */
#define I2S3_WS_PIN 	GPIO_Pin_4   //port A
#define I2S3_MCLK_PIN 	GPIO_Pin_7   //port C
#define I2S3_SCLK_PIN 	GPIO_Pin_10  //port C
#define I2S3_SD_PIN 	GPIO_Pin_12  //port C

#define CODEC_RESET_PIN GPIO_Pin_4  //port D

#define CODEC_BUFFER_SIZE	1024
#define CODEC_DMA			DMA1_Stream5    // dma1 stream 5 ch0 = spi3_tx
#define CODEC_I2S_TX_DMA	(1u << 1) 		// bit 1

#define CODEC_I2C I2C1
#define CODEC_I2S SPI3

#define CORE_I2C_ADDRESS 0x33
#define CODEC_I2C_ADDRESS 0x4A

#define CODEC_MAPBYTE_INC 0x80

/* 
 * Register map bytes for CS42L22 (see page 35) 
 */
#define CODEC_MAP_CHIP_ID 0x01
#define CODEC_MAP_PWR_CTRL1 0x02
#define CODEC_MAP_PWR_CTRL2 0x04
#define CODEC_MAP_CLK_CTRL  0x05
#define CODEC_MAP_IF_CTRL1  0x06
#define CODEC_MAP_IF_CTRL2  0x07
#define CODEC_MAP_PASSTHROUGH_A_SELECT 0x08
#define CODEC_MAP_PASSTHROUGH_B_SELECT 0x09
#define CODEC_MAP_ANALOG_SET 0x0A
#define CODEC_MAP_PASSTHROUGH_GANG_CTRL 0x0C
#define CODEC_MAP_PLAYBACK_CTRL1 0x0D
#define CODEC_MAP_MISC_CTRL 0x0E
#define CODEC_MAP_PLAYBACK_CTRL2 0x0F
#define CODEC_MAP_PASSTHROUGH_A_VOL 0x14
#define CODEC_MAP_PASSTHROUGH_B_VOL 0x15
#define CODEC_MAP_PCMA_VOL 0x1A
#define CODEC_MAP_PCMB_VOL 0x1B
#define CODEC_MAP_BEEP_FREQ_ONTIME 0x1C
#define CODEC_MAP_BEEP_VOL_OFFTIME 0x1D
#define CODEC_MAP_BEEP_TONE_CFG 0x1E
#define CODEC_MAP_TONE_CTRL 0x1F
#define CODEC_MAP_MASTER_A_VOL 0x20
#define CODEC_MAP_MASTER_B_VOL 0x21
#define CODEC_MAP_HP_A_VOL 0x22
#define CODEC_MAP_HP_B_VOL 0x23
#define CODEC_MAP_SPEAK_A_VOL 0x24
#define CODEC_MAP_SPEAK_B_VOL 0x25
#define CODEC_MAP_CH_MIX_SWAP 0x26
#define CODEC_MAP_LIMIT_CTRL1 0x27
#define CODEC_MAP_LIMIT_CTRL2 0x28
#define CODEC_MAP_LIMIT_ATTACK 0x29
#define CODEC_MAP_OVFL_CLK_STATUS 0x2E
#define CODEC_MAP_BATT_COMP 0x2F
#define CODEC_MAP_VP_BATT_LEVEL 0x30
#define CODEC_MAP_SPEAK_STATUS 0x31
#define CODEC_MAP_CHARGE_PUMP_FREQ 0x34


/* Exported functions ------------------------------------------------------- */

extern void codec_init(void);

#endif /* __CODEC_H */
