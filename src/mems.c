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
#include "i2c.h"
#include "mems.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"

/**
 * Initialise the Accelerometer.
 */
extern void
mems_accel_init(void)
{
  uint8_t ctrl_reg1 = 0x00, ctrl_reg2 = 0x00, ctrl_reg3 = 0x00,
          ctrl_reg4 = 0x00, ctrl_reg5 = 0x00, ctrl_reg6 = 0x00;

  /* Configure the low level interface ---------------------------------------*/
  i2c_mems_init();

  /* Configure MEMS: Data rate and power mode */
  ctrl_reg1 |= (LSM303DLHC_ODR_50_HZ |
                LSM303DLHC_NORMAL_MODE |
                LSM303DLHC_AXES_ENABLE);

  /* Configure MEMS: no high pass filters */
  ctrl_reg2 |= LSM303DLHC_HIGHPASSFILTER_DISABLE;

  /* Configure MEMS: scale and resolution */
  ctrl_reg4 |= LSM303DLHC_CONTINUOUS |
               LSM303DLHC_HR_ENABLE |
               LSM303DLHC_FULLSCALE_2G;

  /* Write value to registers */
  (void) i2c_write(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG1_A, &ctrl_reg1, 1);
  // (void) i2c_write(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG2_A, &ctrl_reg2, 1);
  // (void) i2c_write(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG3_A, &ctrl_reg3, 1);
  (void) i2c_write(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG4_A, &ctrl_reg4, 1);
  // (void) i2c_write(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG5_A, &ctrl_reg5, 1);
  // (void) i2c_write(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG6_A, &ctrl_reg6, 1);
}

/**
 * Read XYZ from the Accelerometer.
 */
extern void mems_accel_read(int16_t *data)
{
  int16_t raw_data[3];
  uint8_t mems_buffer[6];
  uint8_t sensitivity = LSM303DLHC_ACC_SENSITIVITY_2G;

  i2c_read(ACC_I2C_ADDRESS, LSM303DLHC_OUT_X_L_A, mems_buffer, 6);

  for (int i = 0; i < 3; i++)
  {
    raw_data[i] = ((int16_t)((uint16_t)mems_buffer[2 * i + 1] << 8) + mems_buffer[2 * i]);
    data[i] = raw_data[i] * sensitivity;
  }
}

/**
 * Initialise the Magnetometer.
 */
extern void
mems_magneto_init(void)
{
	uint8_t cra_regm = 0x00, crb_regm = 0x00, mr_regm = 0x00;

  /* Configure the low level interface ---------------------------------------*/
  i2c_mems_init();

  /* Configure MEMS: Temp Sensor and Data rate */
  cra_regm |= (LSM303DLHC_ODR_15_HZ | LSM303DLHC_TEMPSENSOR_DISABLE);

  /* Configure MEMS: Full Scale */
  crb_regm |= LSM303DLHC_FS_8_1_GA;

  /* Configure MEMS: Working Mode */
  mr_regm |= LSM303DLHC_CONTINUOUS_CONVERSION;

  /* Write value to Mag MEMS CRA_REG register */
  (void) i2c_write(MAG_I2C_ADDRESS, LSM303DLHC_CRA_REG_M, &cra_regm, 1);

  /* Write value to Mag MEMS CRB_REG register */
  (void) i2c_write(MAG_I2C_ADDRESS, LSM303DLHC_CRB_REG_M, &crb_regm, 1);

  /* Write value to Mag MEMS MR_REG register */
  (void) i2c_write(MAG_I2C_ADDRESS, LSM303DLHC_MR_REG_M, &mr_regm, 1);

}
