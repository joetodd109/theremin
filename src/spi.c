/**
 ******************************************************************************
 * @file    spi.c
 * @author  Joe Todd
 * @version
 * @date    January 2014
 * @brief   Theremin
 *
  ******************************************************************************/


/* Includes -------------------------------------------------------------------*/
#include "spi.h"

/* ------- SPI_I2SCFGR ------- */

#define I2S_MODE_EN			(1u << 11)
#define I2S_PERIPH_EN		(1u << 10)
#define I2S_MASTER_TX		(2u << 8)
#define I2S_CKPOL_HIGH		(1u << 3)

/* ------- SPI_I2SPR -------- */

#define I2S_MCLK_EN			(1u << 9)

typedef enum {
    i2s_ch_left,
    i2s_ch_right,
} i2s_chan_t;

typedef enum {
    spi_st_busy,
    spi_st_idle
} spi_status_t;

static uint32_t spi_dma_tc_cnt;
static uint32_t spi_dma_err_cnt;
static uint32_t spi_dma_ht_cnt;
static uint32_t spi_dma_fifo_err_cnt;
static uint32_t spi_dma_int_cnt;
static i2s_chan_t i2s_chan;

static void spi_i2s_configure_dma1_str(int16_t * const src1, int16_t * const src2, uint16_t nbytes);

/**
 * Initialise the I2S.
 */
extern void
spi_i2s_init(void)
{
	uint8_t i2s_div_prescalar;

    i2s_div_prescalar = 6;
    i2s_chan = i2s_ch_left;
    spi_dma_tc_cnt = 0;
    spi_dma_err_cnt = 0;
    spi_dma_ht_cnt = 0;
    spi_dma_fifo_err_cnt = 0;
    spi_dma_int_cnt = 0;

    /*
     * Setup CS43L22 RESET pin on PD4
     */
    iox_configure_pin(iox_port_d, PIN4, iox_mode_out,
                    iox_type_pp, iox_speed_fast, iox_pupd_down);
    /* Keep DAC off for now */
    GPIOD->ODR = ~GPIO_ODR_ODR_4;

	/*
	 * I2S pins
	 */
	iox_configure_pin(iox_port_c, PIN7, iox_mode_af,
						iox_type_pp, iox_speed_fast, iox_pupd_none);
	iox_configure_pin(iox_port_c, PIN10, iox_mode_af,
						iox_type_pp, iox_speed_fast, iox_pupd_none);
	iox_configure_pin(iox_port_c, PIN12, iox_mode_af,
						iox_type_pp, iox_speed_fast, iox_pupd_none);
	iox_configure_pin(iox_port_a, PIN4, iox_mode_af,
						iox_type_pp, iox_speed_fast, iox_pupd_none);

	/* Prepare output ports for alternate function */
	iox_alternate_func(iox_port_a, PIN4, AF6);
	iox_alternate_func(iox_port_c, PIN7, AF6);
	iox_alternate_func(iox_port_c, PIN10, AF6);
	iox_alternate_func(iox_port_c, PIN12, AF6);

	RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;

	SPI3->I2SCFGR = (I2S_MODE_EN
		|	(I2S_PERIPH_EN)
		|	(I2S_MASTER_TX)
		|	(I2S_CKPOL_HIGH));

	SPI3->I2SPR = (I2S_MCLK_EN
		|	(i2s_div_prescalar));

    SPI3->CR2 |= SPI_CR2_TXEIE;
}

/**
 * Function to kick off DMA operations.
 */
extern void
spi_i2s_start_dma(int16_t * const txdata1, int16_t * const txdata2, uint16_t len)
{
    DMA1->HISR = 0x00000000;
    DMA1->LISR = 0x00000000;

    I2S_DMA1->CR &= ~(1u << DMA_CR_EN_Pos);
    while ((I2S_DMA1->CR & (1u << DMA_CR_EN_Pos)) == 1) {
    };

    if ((txdata1 != NULL) &&
        (txdata2 != NULL)) {
        spi_i2s_configure_dma1_str(txdata1, txdata2, len);
    }

    SPI3->CR2 |= (SPI_CR2_TXDMAEN);          /* Enable DMA in SPI */
    I2S_DMA1->CR |= (1u << DMA_CR_EN_Pos);   /* Enable DMA */
}

/**
 * Configure I2S DMA channel for transmission
 */
static void
spi_i2s_configure_dma1_str(int16_t * const src1, int16_t * const src2, uint16_t nbytes)
{
    DMA_Stream_TypeDef cfg = {
        .CR = (1u << DMA_CR_TCIE_Pos) /* Transfer complete interrupt enabled. */
            |(0u << DMA_CR_HTIE_Pos)  /* No half-transfer interrupt. */
            |(1u << DMA_CR_TEIE_Pos)  /* Transfer error interrupt enabled. */
            |(1u << DMA_CR_DIR_Pos)   /* Read from memory to peripheral. */
            |(1u << DMA_CR_CIRC_Pos)  /* Circular mode. */
            |(0u << DMA_CR_PINC_Pos)  /* Don't increment peripheral address. */
            |(1u << DMA_CR_MINC_Pos)  /* Do increment memory address. */
            |(1u << DMA_CR_PSIZE_Pos) /* 16-bit peripheral size. */
            |(0u << DMA_CR_PFCTRL_Pos) /* DMA flow controller */
            |(1u << DMA_CR_MSIZE_Pos) /* 16-bit memory size. */
            |(2u << DMA_CR_PL_Pos)    /* High priority. */
            |(1u << DMA_CR_DBM_Pos)   /* Double buffer mode enabled */
            |(0u << DMA_CR_CHSEL_Pos),      /* Channel Selection. */
        .NDTR = nbytes,
        .PAR = (uint32_t) & SPI3->DR,
        .M0AR = (uint32_t) src1,
        .M1AR = (uint32_t) src2,
    };

    dma_init_dma1_chx(7u, (DMA_Stream_TypeDef const *) &cfg);
    utl_enable_irq(DMA1_Stream7_IRQn);
}

/**
 * Configure NDTR register to current buffer length.
 */
extern void
spi_i2s_reconfigure(uint16_t nbytes)
{
    DMA1->HISR = 0x00000000;
    DMA1->LISR = 0x00000000;

    /* Disable DMA */
    I2S_DMA1->CR &= ~(1u << DMA_CR_EN_Pos);
    while ((I2S_DMA1->CR & (1u << DMA_CR_EN_Pos)) == 1) {
    /* The I2S_DMA1 must be disabled before we can write the NDTR register */
    };
    I2S_DMA1->NDTR = nbytes;
    /* Enable DMA */
    I2S_DMA1->CR |= (1u << DMA_CR_EN_Pos);
}

/**
 * Get current DMA memory target from the double buffer
 */
extern uint32_t
spi_i2s_get_current_memory(void)
{
    return dma_get_current_memory(7u);
}

void DMA1_Stream7_IRQHandler(void)
{
    uint32_t hisr;
    hisr = DMA1->HISR & (DMA_HIFCR_CTCIF7
        | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CHTIF7
        | DMA_HIFCR_CFEIF7);

    /*
     * Test if DMA Stream Transfer Complete
     */
    if (hisr & DMA_HISR_TCIF7) {
        spi_dma_tc_cnt++;
    }
    if (hisr & DMA_HISR_TEIF7) {
        spi_dma_err_cnt++;
    }
    if (hisr & DMA_HISR_HTIF7) {
        spi_dma_ht_cnt++;
    }
    if (hisr & DMA_HISR_FEIF7) {
        spi_dma_fifo_err_cnt++;
    }

    /* Clear interrupt flags */
    DMA1->HIFCR = hisr;

    spi_dma_int_cnt++;
}