/**
  ******************************************************************************
  * @file    spi.h 
  * @author  Joe Todd
  * @version 
  * @date    
  * @brief   Header for spi.c
  *
  ******************************************************************************
*/
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SPI_H
#define SPI_H

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stm32f4xx.h"
#include "iox.h"
#include "dma.h"
#include "codec.h"
#include "utl.h"

#define I2S_DMA0         DMA1_Stream5 
#define I2S_DMA1         DMA1_Stream7

/**
 * Return codes
 */
typedef enum {
    spi_ok,
    spi_busy,
    spi_finished,
    spi_error
} spi_rc_t;


/**
 * Callback function type - the argument passed will indicate
 * success (true) or failure (false) of the operation.
 */
typedef void (*spi_dma_callback_fn) (bool, void *);


/**
 * Initialise the I2S.
 */
extern void spi_i2s_init(void);

/**
 * Function to kick off DMA operations.
 */
extern void spi_i2s_start_dma(int16_t * const txdata1, int16_t * const txdata2, uint16_t len);

/* 
 * Configure NDTR register to current buffer length.
 */
extern void spi_i2s_reconfigure(uint16_t nbytes);

extern void spi_i2s1_reconfigure(uint16_t nbytes);


#endif
