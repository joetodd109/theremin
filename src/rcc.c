/**
 ******************************************************************************
 * @file    rcc.c
 * @author  Joe Todd
 * @version
 * @date    November 2014
 * @brief   Theremin
 *
  ******************************************************************************/


/* Includes -------------------------------------------------------------------*/
#include "rcc.h"

/* Defines --------------------------------------------------------------------*/
#define VECT_TAB_OFFSET  0x00

#define PLL_M      8
#define PLL_N      336
#define PLL_VCO    ((HSE_VALUE / PLL_M) * PLL_N)    /* 672MHz */

#define PLL_P      4
#define PLL_Q      7
#define SYSCLK     (PLL_VCO / PLL_P)    /* 168MHz */

#define PLLI2S_N   192
#define PLLI2S_R   5
#define PLLI2S_VCO ((HSE_VALUE / PLL_M) * PLLI2S_N)  /* 516MHz */
#define I2SCLK     (PLLI2S_VCO / PLLI2S_R)    /* 172MHz */

extern void
clk_init(void)
{
    uint32_t timeout = HSE_STARTUP_TIMEOUT;

  RCC->CR |= (uint32_t)0x00000001;

  /* Reset CFGR register */
  RCC->CFGR = 0x00000000;

  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset PLLCFGR register */
  RCC->PLLCFGR = 0x24003010;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Disable all interrupts */
  RCC->CIR = 0x00000000;

    RCC->CR |= RCC_CR_HSEON;

    while (((RCC->CR & RCC_CR_HSERDY) == 0) && (timeout > 0)) {
        if (timeout != 0) {
            timeout--;
        }
    }

    /* Configure the main PLL */
    RCC->PLLCFGR = PLL_M
        | (PLL_N << 6)
        | (((PLL_P >> 1) - 1) << 16)
        | (RCC_PLLCFGR_PLLSRC_HSE)
        | (PLL_Q << 24);

     /* Select regulator voltage output Scale 2 mode, System frequency up to 84 MHz */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR &= ~PWR_CR_VOS;

    /* HCLK = SYSCLK / 1*/
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

    /* PCLK2 = HCLK / 1*/
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

    /* PCLK1 = HCLK / 2*/
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    /* Enable the main PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till the main PLL is ready */
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);

    /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
    FLASH->ACR = (FLASH_ACR_PRFTEN
        | FLASH_ACR_ICEN
        | FLASH_ACR_DCEN
        | FLASH_ACR_LATENCY_2WS);

    /* Select the main PLL as system clock source */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    /* Wait till the main PLL is used as system clock source */
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

}

extern void
fpu_on(void)
{
    /* set CP10 and CP11 Full Access */
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));
}

extern void
i2s_clk_init(void)
{
    /* PLLI2S clock used as I2S clock source */
    RCC->CFGR &= ~RCC_CFGR_I2SSRC;

    /* Configure PLLI2S */
    RCC->PLLI2SCFGR = (PLLI2S_N << 6) | (PLLI2S_R << 28);

    /* Enable PLLI2S */
    RCC->CR |= RCC_CR_PLLI2SON;

    /* Wait till PLLI2S is ready */
    while ((RCC->CR & RCC_CR_PLLI2SRDY) == 0);
}