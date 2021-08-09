/**
 ******************************************************************************
 * @file    mems.c
 * @author  Joe Todd
 * @version
 * @date    January 2014
 * @brief   Theremin
 *
  ******************************************************************************/


/* Includes -------------------------------------------------------------------*/
#include "mems.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"

/**
 * Initialise the Magnetometer.
 */
extern void
mems_init(void)
{
	uint8_t cra_regm = 0x00, crb_regm = 0x00, mr_regm = 0x00;

    /* Configure the low level interface ---------------------------------------*/
    i2c_mems_init();

    /* Configure MEMS: Temp Sensor and Data rate */
    cra_regm |= (LSM303DLHC_ODR_75_HZ | LSM303DLHC_TEMPSENSOR_DISABLE);

    /* Configure MEMS: Full Scale */
    crb_regm |= LSM303DLHC_FS_8_1_GA;

    /* Configure MEMS: Working Mode */
    mr_regm |= LSM303DLHC_CONTINUOS_CONVERSION;

    /* Write value to Mag MEMS CRA_REG register */
    (void) i2c_write(MAG_I2C_ADDRESS, LSM303DLHC_CRA_REG_M, &cra_regm, 1);

    /* Write value to Mag MEMS CRB_REG register */
    (void) i2c_write(MAG_I2C_ADDRESS, LSM303DLHC_CRB_REG_M, &crb_regm, 1);

    /* Write value to Mag MEMS MR_REG register */
    (void) i2c_write(MAG_I2C_ADDRESS, LSM303DLHC_MR_REG_M, &mr_regm, 1);

}

/**
 * Read XYZ from the Magnetometer.
 */
extern void mems_magneto_read(int16_t *data)
{
  uint8_t mems_buffer[6];

  i2c_read(MAG_I2C_ADDRESS, LSM303DLHC_OUT_X_H_M, mems_buffer, 6);

  data[0] = ((int16_t)((uint16_t)mems_buffer[0] << 8) + mems_buffer[1]) * LSM303DLHC_M_SENSITIVITY_XY_8_1Ga;
  data[1] = ((int16_t)((uint16_t)mems_buffer[4] << 8) + mems_buffer[5]) * LSM303DLHC_M_SENSITIVITY_XY_8_1Ga;
  data[2] = ((int16_t)((uint16_t)mems_buffer[2] << 8) + mems_buffer[3]) * LSM303DLHC_M_SENSITIVITY_Z_8_1Ga;
}
