/**
  ******************************************************************************
  * @file    rcc.h 
  * @author  Joe Todd
  * @version 
  * @date    
  * @brief   Header for rcc.c
  *
  ******************************************************************************
*/
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RCC_H
#define RCC_H

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stm32f4xx.h"

extern void clk_init(void);
extern void fpu_on(void);
extern void i2s_clk_init(void);

#endif