/**
 ******************************************************************************
 * @file    iox.c
 * @author  Joe Todd
 * @version
 * @date    January 2014
 * @brief   Theremin
 *
  ******************************************************************************/


/* Includes -------------------------------------------------------------------*/
#include "iox.h"


GPIO_TypeDef *const iox_gpios[] = {
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
};

static uint32_t const io_enables[] = {
    [iox_port_a] = RCC_AHB1ENR_GPIOAEN,
    [iox_port_b] = RCC_AHB1ENR_GPIOBEN,
    [iox_port_c] = RCC_AHB1ENR_GPIOCEN,
    [iox_port_d] = RCC_AHB1ENR_GPIODEN,
    [iox_port_e] = RCC_AHB1ENR_GPIOEEN,
};


/**
 * Configure a single pin.
 */
extern void
iox_configure_pin(iox_port_t port, uint32_t pin,
                  iox_mode_t mode, iox_type_t type, 
                  iox_speed_t speed, iox_pupd_t pupd)
{
	GPIO_TypeDef *gpio;
    volatile uint32_t *moder;
    volatile uint32_t *typer;
    volatile uint32_t *speedr;
    volatile uint32_t *pupdr;
    uint32_t shift;
    uint32_t mask;

	/*
     * Ensure clock for this port is enabled.
     */
    RCC->AHB1ENR |= io_enables[port];

    /*
     * Now configure it.
     */
    gpio = iox_gpios[port];
    shift = 2 * pin;
    mask = 0x3 << shift;

    moder = &gpio->MODER;
	*moder = (*moder & ~mask) | (mode << shift);

	typer = &gpio->OTYPER;
	*typer = (*typer & ~(1 << pin)) | (type << pin);

	speedr = &gpio->OSPEEDR;
	*speedr = (*speedr & ~mask) | (speed << shift);

	pupdr = &gpio->PUPDR;
	*pupdr = (*pupdr & ~mask) | (pupd << shift);
}

/*
 * Configure a pin for an alternate function
 */
extern void
iox_alternate_func(iox_port_t port, uint32_t pin,
			  			uint32_t af) 
{
	GPIO_TypeDef *gpio;
    volatile uint32_t *afr;
    uint32_t shift;
    uint32_t mask;

    /*
     * Ensure clock for this port is enabled.
     */
    RCC->AHB1ENR |= io_enables[port];

    /*
     * Now configure it.
     */
    gpio = iox_gpios[port];
    if (pin > 7) {
        afr = &gpio->AFRH;
        pin -= 8;
    }
    else {
        afr = &gpio->AFRL;
    }

    shift = 4 * pin;
    mask = 0xf << shift;

    *afr = (*afr & ~mask) | (af << shift);
}

/**
 * Change an output pin state.
 *
 * Just call the macro defined in iox.h.
 */
extern void
 (iox_set_pin_state) (iox_port_t port, uint32_t pin, bool state) {
    iox_set_pin_state(port, pin, state);
}

/**
 * Read the state of an input pin.
 *
 * Just call the macro defined in iox.h.
 */
extern bool(iox_get_pin_state) (iox_port_t port, uint32_t pin) {
    return iox_get_pin_state(port, pin);
}

/* 
 * Sets up GPIO's for LED's
 */
extern void
iox_led_init(void)
{
	/* 
     * Enable DAC RESET/LED clocks
     */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    /*
     * Set pin 12, 13, 14, 15 as output 
     */
    GPIOD->MODER |= GPIO_MODER_MODER12_0
        | (GPIO_MODER_MODER13_0)
        | (GPIO_MODER_MODER14_0)
        | (GPIO_MODER_MODER15_0)
        ;
}

/**
 * Turn on an LED
 */
extern void
iox_led_on(bool green, bool amber, bool red, bool blue)
{
    iox_leds_off();

    if (green) {
        GPIOD->ODR |= GPIO_ODR_ODR_12;
    }
    if (amber) {
        GPIOD->ODR |= GPIO_ODR_ODR_13;
    }
    if (red) {
        GPIOD->ODR |= GPIO_ODR_ODR_14;
    }
    if (blue) {
        GPIOD->ODR |= GPIO_ODR_ODR_15;
    }
}

/**
 * Turn off all LEDs
 */
extern void
iox_leds_off(void) 
{
    GPIOD->ODR &= ~(GPIO_ODR_ODR_12
    | (GPIO_ODR_ODR_13)
    | (GPIO_ODR_ODR_14)
    | (GPIO_ODR_ODR_15));   
}
