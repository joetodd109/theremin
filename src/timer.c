/**
 ******************************************************************************
 * @file    timer.c
 * @author  Joe Todd
 * @version
 * @date    January 2014
 * @brief   MIDI
 *
  ******************************************************************************/

#include "timer.h"

static volatile uint32_t count = 0;

/* 
 * SYSCLK = 16MHz
 * TIM2CLK = 16MHz / 16000 = 1kHz
 */
extern void 
timer_init(void) 
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    
    TIM2->PSC = 0x3E80;             /* 16000 prescalar */
    TIM2->DIER |= TIM_DIER_UIE;     /* enable update interrupt */
    TIM2->ARR = 0x0030;             /* count to 48 */
    TIM2->CR1 |= TIM_CR1_ARPE       /* autoreload on */
        | (TIM_CR1_CEN);            /* counter enabled */
    TIM2->EGR = 1;                  /* trigger update event */

    utl_enable_irq(TIM2_IRQn);
}

extern uint32_t 
timer_get(void)
{
    return count;
}

extern void 
timer_delay(uint32_t time) {
    uint32_t start;
    uint32_t timer;
    uint32_t end;

    start = timer_get();
    end = start + time;

    while (timer < end) {
        timer = timer_get();
    }
}

void TIM2_IRQHandler(void) 
{
    /* 
     * Read data from Magnetometer
     * and reset the status register.
     */
    set_mems_read();
    TIM2->SR = 0x0;
}
