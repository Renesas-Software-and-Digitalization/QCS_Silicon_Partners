/*
 * i2c.h
 */
#ifndef I2C_EP_H_
#define I2C_EP_H_


/* Function declaration */
/*******************************************************************************************************************//**
 * @brief       Initialize  I2C.
 * @param[in]   None
 * @retval      FSP_SUCCESS         Upon successful open and start of timer
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful open
 ***********************************************************************************************************************/
fsp_err_t i2c_initialize(void);
/*******************************************************************************************************************//**
 *  @brief       Deinitialize I2C module
 *  @param[in]   None
 *  @retval      None
 **********************************************************************************************************************/
void i2c_deinitialize(void);

#endif
