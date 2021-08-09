/*******************************************************************************
 * @file    utl.c
 * @author  Joe Todd
 * @version
 * @date    January 2014
 * @brief   Theremin
 *
  ******************************************************************************/


/* Includes -------------------------------------------------------------------*/
#include "utl.h"

/**
 * Enable an interrupt.
 */
extern void
utl_enable_irq(IRQn_Type irq)
{
	uint32_t i;

	i = irq / 32;
	NVIC->ISER[i] = 1 << (irq - (i * 32));
}

/**
 * Disable an interrupt.
 */
extern void
utl_disable_irq(IRQn_Type irq)
{
	uint32_t i;

	i = irq / 32;
	NVIC->ICER[i] = 1 << (irq - (i * 32));
}

extern void
utl_median_filter_init(utl_median_filter_t *mf, float default_value, uint8_t filter_order)
{
    uint32_t i;

    for (i = 0; i < NELEMS(mf->input); i++) {
        mf->input[i] = default_value;
    }

    if (filter_order > MEDIAN_FILTER_ORDER_MAX) {
        filter_order = MEDIAN_FILTER_ORDER_MAX;
    }

    mf->order = filter_order;
    mf->output = default_value;
    mf->initialised = false;
}

/**
 * Read the latest value from the filter.
 */
extern float
utl_median_filter_get_value(utl_median_filter_t const *mf)
{
    return mf->output;
}

/**
 * Put a new value into the filter.
 */
extern void
utl_median_filter_update(utl_median_filter_t * mf, float value)
{
    uint32_t i;

    if (mf->initialised) {
        float window[MEDIAN_FILTER_ORDER_MAX];

        /*
         * Shift the data down and append the latest value.
         */
        for (i = 1; i < mf->order; i++) {
            mf->input[i - 1] = mf->input[i];
        }

        mf->input[mf->order - 1] = value;

        /*
         * Populate temporary array with input values
         */
        for (i = 0; i < mf->order; i++) {
            window[i] = mf->input[i];
        }

        /* Insertion sorting algorithm */
        for (i = 1; i < mf->order; i++) {
            uint32_t j;
            float swap;

            for (j = i; ((j > 0) && (window[j] < window[j - 1])); j--) {
                swap = window[j];
                window[j] = window[j - 1];
                window[j - 1] = swap;
            }
        }

        mf->output = window[mf->order / 2];

    }
    else {
        for (i = 0; i < NELEMS(mf->input); i++) {
            mf->input[i] = value;
        }

        mf->output = value;
        mf->initialised = true;
    }
}
