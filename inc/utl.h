/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UTL_H
#define UTL_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32f4xx.h"

/**
 * Number of elements in an array.
 */
#define NELEMS(x) (sizeof (x) / sizeof (x)[0])

/**
 * The maximum order for a median filter.
 */
#define MEDIAN_FILTER_ORDER_MAX 7

typedef struct {
    bool initialised;    /**< flag to indicate that the filter is initialised. */
    uint8_t order;       /**< the order, must not exceed UTL_MEDIAN_FILTER_ORDER_MAX. */
    float input[MEDIAN_FILTER_ORDER_MAX]; /**< array of previous input values. */
    float output;      /**< current output value. */
} utl_median_filter_t;

/**
 * Enable an interrupt.
 */
extern void utl_enable_irq(IRQn_Type irq);

/**
 * Disable an interrupt.
 */
extern void utl_disable_irq(IRQn_Type irq);

/**
 * Initialise a median filter.
 */
extern void utl_median_filter_init(utl_median_filter_t * mf, float default_value, uint8_t filter_order);

/**
 * Read the latest value from the filter.
 */
extern float utl_median_filter_get_value(utl_median_filter_t const *mf);

/**
 * Put a new value into the filter.
 */
extern void utl_median_filter_update(utl_median_filter_t * mf, float value);

#endif /* __UTL_H */