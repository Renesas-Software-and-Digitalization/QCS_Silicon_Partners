/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2017 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively "Software") is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "inv_icm426xx.h"


/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Definitions of Open flag */
#define RM_ICM426XX_OPEN                               (0x433E4432UL) // Open state


/* Definitions of Timeout */
#define RM_ICM426XX_EVENT_WAITTIME                     (10U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global function prototypes
 **********************************************************************************************************************/
//void rm_icm426xx_callback(rm_comms_callback_args_t * p_args);

int idd_io_hal_read_reg(void * context, uint8_t reg, uint8_t *rbuffer, uint32_t rlen);
int idd_io_hal_write_reg(void * context, uint8_t reg, const uint8_t *wbuffer, uint32_t wlen);

void sm_delay_ms(uint16_t ms);
void sm_delay_us(uint16_t us);

void inv_data_handler(rm_icm426xx_instance_ctrl_t * p_ctrl,
                      AccDataPacket *accDatabuff,GyroDataPacket *gyroDatabuff,
                      chip_temperature*chip_temper);

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
rm_icm426xx_instance_ctrl_t *g_p_ctrl_p;

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
rm_icm426xx_api_t const g_icm426xx_on_icm426xx =
{
    .open                 = RM_ICM426XX_Open,
    .close                = RM_ICM426XX_Close,
    .sensorIdGet          = RM_ICM426XX_SensorIdGet,
    .sensorInit           = RM_ICM426XX_Sensor_Init,
    .accEnable            = RM_ICM426XX_Acc_Enable,
    .gyroEnable           = RM_ICM426XX_Gyro_Enable,
    .acc_Setodr           = RM_ICM426XX_Acc_SetRate,
    .gyro_Setodr          = RM_ICM426XX_Gyro_SetRate,
    .get_sensordata       = RM_ICM426XX_Sensor_GetData,
    .register_i2c_evetnt  = RM_ICM426XX_Register_I2C_event,
  
};

/*******************************************************************************************************************//**
 * @addtogroup RM_ICM426XX
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 *  @brief      Perform Master Write operation
 *  @param[IN]  None
 *  @retval     FSP_SUCCESS               Master Writes and Slave receives data successfully.
 *  @retval     FSP_ERR_TRANSFER_ABORTED  callback event failure
 *  @retval     FSP_ERR_ABORTED           data mismatched occurred.
 *  @retval     FSP_ERR_TIMEOUT           In case of no callback event occurrence
 *  @retval     read_err                  API returned error if any
 **********************************************************************************************************************/
static fsp_err_t sci_i2c_master_write_register(rm_icm426xx_instance_ctrl_t * p_ctrl,uint8_t reg,const uint8_t *write_data,uint32_t length)
{
    fsp_err_t write_err           = FSP_SUCCESS;
    uint8_t *write_buffer ;
    uint16_t write_time_out       = 50000;
    volatile bool *g_master_event = p_ctrl->p_g_master_event;
    //volatile i2c_master_event_t *g_master_event = p_ctrl->p_g_master_event;

    /* resetting callback event */
    //*g_master_event = (i2c_master_event_t)RESET_VALUE;
    *g_master_event = false;

    write_buffer = malloc(length + 1);
    memset(write_buffer, 0, (size_t) sizeof(write_buffer));

    write_buffer[0] = reg;
    memcpy(&write_buffer[1], write_data, (size_t) length);

    /* resetting callback event */
    *g_master_event = (i2c_master_event_t)RESET_VALUE;

    /* Start master Write operation */
    write_err = p_ctrl->p_comms_i2c_instance->p_api->write(p_ctrl->p_comms_i2c_instance->p_ctrl, write_buffer, length+1);
 
    /* Start master read. Master has to initiate the transfer. */
    if (FSP_SUCCESS != write_err)
    {
       printf("\r\n ** R_SCI_I2C_Write API FAILED ** \r\n");
        goto transfer_error;
    }

    /* Wait until slave write and master write process gets completed */
    while (false == *g_master_event)
    {  
        /*start checking for time out to avoid infinite loop*/
        R_BSP_SoftwareDelay(RM_ICM426XX_EVENT_WAITTIME, BSP_DELAY_UNITS_MICROSECONDS);
        --write_time_out;

        /*check for time elapse*/
        if (0x00 == write_time_out)
        {
        /*we have reached to a scenario where i2c event not occurred*/
            printf (" ** No event wrtien during Master read and Slave write operation **\r\r");

            /*no event received*/
            goto transfer_error;
        }
        
    }

    free(write_buffer);
    return write_err;

transfer_error:
    free(write_buffer);
    return FSP_ERR_WRITE_FAILED;
}

/*******************************************************************************************************************//**
 *  @brief       Perform Master Read operation
 *  @param[IN]   rm_icm426xx_instance_ctrl_t * p_ctrl,uint8_t reg,uint8_t *read_data,uint8_t length
 *  @retval      FSP_SUCCESS               Master successfully read all data written by slave device.
 *  @retval      FSP_ERR_TRANSFER_ABORTED  callback event failure
 *  @retval      FSP_ERR_ABORTED           data mismatched occurred.
 *  @retval      FSP_ERR_TIMEOUT           In case of no callback event occurrence
 *  @retval      err                       API returned error if any
 **********************************************************************************************************************/
static fsp_err_t sci_i2c_master_read_register(rm_icm426xx_instance_ctrl_t * p_ctrl,uint8_t reg,uint8_t *read_data,uint32_t length)
{
    fsp_err_t read_err           = FSP_SUCCESS;

    uint16_t read_time_out       = 50000;  // Us unit
    uint8_t *read_buffer  ;
    uint8_t write_buffer ;
    volatile bool *g_master_event = p_ctrl->p_g_master_event;
    /*uint16_t write_time_out      = 50000;
     fsp_err_t write_err          = FSP_SUCCESS;
     volatile i2c_master_event_t *g_master_event = p_ctrl->p_g_master_event;*/

     rm_comms_write_read_params_t write_read_params;

    /* resetting callback event */
    *g_master_event = false;
    //*g_master_event = (i2c_master_event_t)RESET_VALUE;

    read_buffer = read_data;
    write_buffer = reg;

    write_read_params.p_dest = read_buffer;
    write_read_params.dest_bytes = (uint8_t)length;
    write_read_params.p_src  = &write_buffer;
    write_read_params.src_bytes = 1;

    read_err =  p_ctrl->p_comms_i2c_instance->p_api->writeRead(p_ctrl->p_comms_i2c_instance->p_ctrl, write_read_params);
    #if 0
    /* Start master Write operation */
    write_err = p_ctrl->p_comms_i2c_instance->p_api->write(p_ctrl->p_comms_i2c_instance->p_ctrl, &write_buffer, length+1);
    /* Start master read. Master has to initiate the transfer. */
    if (FSP_SUCCESS != write_err)
    {
        printf("\r\n ** R_SCI_I2C_Write API FAILED ** \r\n");
        return read_err;
    }

     /* Wait until slave write and master read process gets completed */
    while (false == *g_master_event)
    {

        /*start checking for time out to avoid infinite loop*/
        R_BSP_SoftwareDelay(RM_ICM426XX_EVENT_WAITTIME, BSP_DELAY_UNITS_MICROSECONDS);
        --write_time_out;

        /*check for time elapse*/
        if (0x00 == write_time_out)
        {
            /*we have reached to a scenario where i2c event not occurred*/
            printf (" ** No event wrtien during Master read and Slave write operation **\r\r");

            /*no event received*/
            return FSP_ERR_TIMEOUT;
        }
    }

    *g_master_event = false;

    read_err = p_ctrl->p_comms_i2c_instance->p_api->read(p_ctrl->p_comms_i2c_instance->p_ctrl, read_buffer, length);
    
    #endif
    
    /* Start master read. Master has to initiate the transfer. */
    if (FSP_SUCCESS != read_err)
    {
        printf("\r\n ** R_SCI_I2C_Read API FAILED ** \r\n");
        return read_err;
    }

    /* Wait until slave write and master read process gets completed */
    while (false == *g_master_event)
    {

        /*start checking for time out to avoid infinite loop*/
        R_BSP_SoftwareDelay(RM_ICM426XX_EVENT_WAITTIME, BSP_DELAY_UNITS_MICROSECONDS);
        --read_time_out;

        /*check for time elapse*/
        if (0x00 == read_time_out)
        {
            /*we have reached to a scenario where i2c event not occurred*/
            printf (" ** No event received during Master read and Slave write operation **\r\r");

            /*no event received*/
            return FSP_ERR_TIMEOUT;
        }
    }
 
    return read_err;
}


/*******************************************************************************************************************//**
 * @brief  communication call back register function for icm426xx_hal_driver
   @param[IN]   None
 *  @retval      FSP_SUCCESS               Master successfully read all data written by slave device.
 *  @retval      FSP_ERR_TRANSFER_ABORTED  callback event failure
 *  @retval      FSP_ERR_ABORTED           data mismatched occurred.
 *  @retval      FSP_ERR_TIMEOUT           In case of no callback event occurrence
 *  @retval      err                       API returned error if any
 *

 **********************************************************************************************************************/
int idd_io_hal_read_reg(void * context, uint8_t reg, uint8_t *rbuffer, uint32_t rlen)
{
	(void)context;
#if SPI_MODE_EN
	//return spi_master_transfer_rx(NULL, reg, rbuffer, rlen);
#else /* SERIF_TYPE_I2C */
    if(NULL != g_p_ctrl_p )
        return sci_i2c_master_read_register(g_p_ctrl_p,reg,rbuffer,rlen);
    else{
        printf (" ** Null point for g_p_ctrl_p wwhen assign bus read call back\r\n");
        return FSP_ERR_INVALID_POINTER;        
    }
#endif	
}

int idd_io_hal_write_reg(void * context, uint8_t reg, const uint8_t *wbuffer, uint32_t wlen)
{
	(void)context;
#if SPI_MODE_EN
	//return spi_master_transfer_tx(NULL, reg, wbuffer, wlen);

#else /* SERIF_TYPE_I2C */
    if(NULL != g_p_ctrl_p )   
        return sci_i2c_master_write_register(g_p_ctrl_p,reg, wbuffer,wlen);
    else{
        printf (" ** Null point for g_p_ctrl_p wwhen assign bus write call back\r\n");
        return FSP_ERR_INVALID_POINTER;        
    }

#endif	
}

/*******************************************************************************************************************//**
 * @brief  delay call back register function for icm426xx_hal_driver
   @param[IN]   None
 *  @retval     None

 **********************************************************************************************************************/
void sm_delay_ms(uint16_t ms)
{
    R_BSP_SoftwareDelay(ms, BSP_DELAY_UNITS_MILLISECONDS);
}

void sm_delay_us(uint16_t us)
{
    R_BSP_SoftwareDelay(us, BSP_DELAY_UNITS_MICROSECONDS);
}

#if POLLING_MODE_SUPPORT
void polling_read_data(){

	memset(&datapacket, 0, sizeof(struct accGyroDataPacket));

		inv_icm4x6xx_polling_rawdata(&datapacket);

	    INV_LOG(SENSOR_LOG_LEVEL, "ACC accOutSize %d gyroOutSize %d ",
						  datapacket.accOutSize,datapacket.gyroOutSize);

		if (datapacket.outBuf[0].sensType == ACC){
		    INV_LOG(SENSOR_LOG_LEVEL, "ACC %d  %d  %d",datapacket.outBuf[0].x, datapacket.outBuf[0].y, datapacket.outBuf[0].z);
				//datapacket.outBuf[0].timeStamp, datapacket.outBuf[0].x, datapacket.outBuf[0].y, datapacket.outBuf[0].z);
		}
		else if (datapacket.outBuf[1].sensType == ACC){
			INV_LOG(SENSOR_LOG_LEVEL, "acc+1 %d  %d  %d",datapacket.outBuf[1].x, datapacket.outBuf[1].y, datapacket.outBuf[1].z);
				//datapacket.outBuf[1].timeStamp, datapacket.outBuf[1].x, datapacket.outBuf[1].y, datapacket.outBuf[1].z);
		}

		if (datapacket.outBuf[0].sensType == GYR){
		    INV_LOG(SENSOR_LOG_LEVEL, "GYR BUFF  %d  %d  %d",datapacket.outBuf[0].x, datapacket.outBuf[0].y, datapacket.outBuf[0].z);
		        //datapacket.outBuf[0].timeStamp, datapacket.outBuf[0].x, datapacket.outBuf[0].y, datapacket.outBuf[0].z);
		}
		else if (datapacket.outBuf[1].sensType == GYR){
		    INV_LOG(SENSOR_LOG_LEVEL, "GYR BUFF +1 %d  %d  %d",datapacket.outBuf[1].x, datapacket.outBuf[1].y, datapacket.outBuf[1].z);
		        //datapacket.outBuf[1].timeStamp, datapacket.outBuf[1].x, datapacket.outBuf[1].y, datapacket.outBuf[1].z);
		}
		//sm_delay_ms(10);	
}
#endif // POLLING_MODE_SUPPORT
  
static struct accGyroDataPacket datapacket;

void inv_data_handler(rm_icm426xx_instance_ctrl_t * p_ctrl,
                      AccDataPacket *accDatabuff,GyroDataPacket *gyroDatabuff,
                      chip_temperature*chip_temper)
{
  
	int32_t data[3];
    int acc_index = 0;
    int gyro_index = 0;

    AccDataPacket *AccDatabuff = accDatabuff;
    GyroDataPacket *GyroDatabuff = gyroDatabuff;

    if(NULL == AccDatabuff || NULL == GyroDatabuff){
        printf (" ** Null point for databuff when handle irq data\r\n"); 
		return;
    }

    /* Raw A+G */
    if (true == p_ctrl->sensor[ACC] || true == p_ctrl->sensor[GYR])
    {
        memset(&datapacket, 0, sizeof(struct accGyroDataPacket));
        inv_icm4x6xx_get_rawdata_interrupt(&datapacket);

        INV_LOG(SENSOR_LOG_LEVEL, "Get Sensor Data ACC %d GYR %d ",
            datapacket.accOutSize, datapacket.gyroOutSize);

        AccDatabuff->accDataSize = datapacket.accOutSize;
        GyroDatabuff->gyroDataSize = datapacket.gyroOutSize;

#if (!SUPPORT_PEDOMETER)
        for (int i = 0; i < datapacket.accOutSize + datapacket.gyroOutSize; i++)
        {
	        if(IS_HIGH_RES_MODE)
	        {
				data[0] = ((int32_t)datapacket.outBuf[i].x <<4) | datapacket.outBuf[i].high_res[0];
				data[1] = ((int32_t)datapacket.outBuf[i].y <<4) | datapacket.outBuf[i].high_res[1];
				data[2] = ((int32_t)datapacket.outBuf[i].z <<4) | datapacket.outBuf[i].high_res[2];
			}
			else
			{
				data[0] = (int32_t)datapacket.outBuf[i].x;
				data[1] = (int32_t)datapacket.outBuf[i].y;
				data[2] = (int32_t)datapacket.outBuf[i].z;
			}

	#if DATA_FORMAT_DPS_G  // Output data in dps/g/degree format.
			inv_apply_mounting_matrix(data);

			/* convert to physical unit: Celsius degree */
			if (FIFO_WM_MODE_EN && (!IS_HIGH_RES_MODE))  // Normal FIFO mode
				*chip_temper = ((float)datapacket.outBuf[i].temperature * TEMP_SENSITIVITY_1_BYTE) + ROOM_TEMP_OFFSET;
			else  // FIFO High Resolution mode and DRI mode.
				*chip_temper = ((float)datapacket.outBuf[i].temperature * TEMP_SENSITIVITY_2_BYTE) + ROOM_TEMP_OFFSET;

			if( ACC==datapacket.outBuf[i].sensType )
			{
				AccDatabuff->databuff[acc_index].x = (float)data[0] * KSCALE_ACC_FSR;
                AccDatabuff->databuff[acc_index].y = (float)data[1] * KSCALE_ACC_FSR;
                AccDatabuff->databuff[acc_index].z = (float)data[2] * KSCALE_ACC_FSR;
                AccDatabuff->databuff[acc_index].timeStamp = datapacket.outBuf[i].timeStamp;

                #if 0
                INV_LOG(SENSOR_LOG_LEVEL, "ACC  %f %f %f %f", chip_temper, 
                                       AccDatabuff->databuff[acc_index].x, 
                                       AccDatabuff->databuff[acc_index].y, 
                                       AccDatabuff->databuff[acc_index].z);
				//INV_LOG(SENSOR_LOG_LEVEL, "ACC  %lld %f %f %f %f", datapacket.outBuf[i].timeStamp, chip_temper, fData[0], fData[1], fData[2] );
                #endif
                acc_index ++;
			}
			else if(GYR==datapacket.outBuf[i].sensType)
			{
				
				GyroDatabuff->databuff[gyro_index].x = (float)data[0] * KSCALE_GYRO_FSR;
                GyroDatabuff->databuff[gyro_index].y = (float)data[1] * KSCALE_GYRO_FSR;
                GyroDatabuff->databuff[gyro_index].z = (float)data[2] * KSCALE_GYRO_FSR;
                GyroDatabuff->databuff[gyro_index].timeStamp = datapacket.outBuf[i].timeStamp;

                #if 0
                INV_LOG(SENSOR_LOG_LEVEL, "GYR  %f %f %f %f", chip_temper, 
                                     GyroDatabuff->databuff[gyro_index].x, 
                                     GyroDatabuff->databuff[gyro_index].y,
                                     GyroDatabuff->databuff[gyro_index].z);
				//INV_LOG(SENSOR_LOG_LEVEL, "GYR  %lld %f %f %f %f", datapacket.outBuf[i].timeStamp, chip_temper, fData[0], fData[1], fData[2] );
                #endif
                gyro_index ++;
			}

	#else  // Output data in LSB format.
            if ( ACC==datapacket.outBuf[i].sensType ){
                AccDatabuff->databuff[acc_index].x = data[0] ;
                AccDatabuff->databuff[acc_index].y = data[1] ;
                AccDatabuff->databuff[acc_index].z = data[2] ;
                AccDatabuff->databuff[acc_index].timeStamp = datapacket.outBuf[i].timeStamp;

                *chip_temper  = datapacket.outBuf[i].temperature;
                acc_index ++;
                INV_LOG(SENSOR_LOG_LEVEL, "ACC %lld %ld  %ld  %ld %ld", datapacket.outBuf[i].timeStamp, datapacket.outBuf[i].temperature, data[0], data[1], data[2]);
            }else if ( GYR==datapacket.outBuf[i].sensType ){
                INV_LOG(SENSOR_LOG_LEVEL, "GYR %lld %ld  %ld  %ld %ld", datapacket.outBuf[i].timeStamp, datapacket.outBuf[i].temperature, data[0], data[1], data[2]);
                GyroDatabuff->databuff[gyro_index].x = data[0];
                GyroDatabuff->databuff[gyro_index].y = data[1];
                GyroDatabuff->databuff[gyro_index].z = data[2];
                GyroDatabuff->databuff[gyro_index].timeStamp = datapacket.outBuf[i].timeStamp;
                
                *chip_temper = datapacket.outBuf[i].temperature;
                gyro_index ++;
            }
    #endif
        }
#endif
    }

#if SUPPORT_PEDOMETER
    if (true == p_ctrl->sensor[PEDO]) {
        uint64_t step_cnt = 0;
        inv_icm4x6xx_pedometer_get_stepCnt(&step_cnt);
        INV_LOG(SENSOR_LOG_LEVEL, "Step count %d", step_cnt);
    }
#endif

#if SUPPORT_WOM
    if (true == p_ctrl->sensor[WOM]) {
        bool wom_detect = false;
        inv_icm4x6xx_wom_get_event(&wom_detect);
        INV_LOG(SENSOR_LOG_LEVEL, "wom Motion %d", wom_detect);
    }
#endif
}

/*******************************************************************************************************************//**
 * @brief Opens and configures the ICM426XX Middle module. Implements @ref rm_icm426xx_api_t::open.
 *
 * Example:
 * @snippet  RM_ICM426XX_Open
 *
 * @retval FSP_SUCCESS              ICM426XX successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval FSP_ERR_ALREADY_OPEN     Module is already open.  This module can only be opened once.
 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_Open (rm_icm426xx_ctrl_t * const p_api_ctrl, rm_icm426xx_cfg_t const * const p_cfg)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM426XX_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_cfg);
    FSP_ASSERT(NULL != p_cfg->p_instance);
    FSP_ERROR_RETURN(RM_ICM426XX_OPEN != p_ctrl->open, FSP_ERR_ALREADY_OPEN);
#else
    FSP_ASSERT(NULL != p_ctrl);
#endif
    p_ctrl->p_cfg                  = p_cfg;
    p_ctrl->p_comms_i2c_instance   = p_cfg->p_instance;
    //p_ctrl->p_i2c_master_instance  = p_cfg->p_instance;
    p_ctrl->p_context              = p_cfg->p_context;
    p_ctrl->p_callback             = p_cfg->p_callback;

    //assign the main p_ctrl to global point for futher use
    g_p_ctrl_p = p_ctrl;

    /* Open Communications middleware */
    err = p_ctrl->p_comms_i2c_instance->p_api->open(p_ctrl->p_comms_i2c_instance->p_ctrl,
                                                      p_ctrl->p_comms_i2c_instance->p_cfg);
                                                      
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Initialize SPI/I2C call back function */
    inv_icm4x6xx_set_serif(idd_io_hal_read_reg, idd_io_hal_write_reg);

    /* Initialize delay call back function */
    inv_icm4x6xx_set_delay(sm_delay_ms, sm_delay_us);

    /* Set open flag */
    p_ctrl->open = RM_ICM426XX_OPEN;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Disables specified ICM426XX control block. Implements @ref rm_icm426xx_api_t::close.
 *
 * @retval FSP_SUCCESS              Successfully closed.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_Close (rm_icm426xx_ctrl_t * const p_api_ctrl)
{
    rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;

    FSP_ASSERT(NULL != p_ctrl);

    /* Close Communications Middleware */
    p_ctrl->p_comms_i2c_instance->p_api->close(p_ctrl->p_comms_i2c_instance->p_ctrl);

    /* Clear Open flag */
    p_ctrl->open = 0;

    return FSP_SUCCESS;
}
/*******************************************************************************************************************//**
 * @brief Register I2C event to instance driver 
 * @Importatnce must be called right after open api to set up i2c call back.  After then, i2c communication could be called
 * Implements @ref rm_icm426xx_api_t::register_i2c_evetnt.
 * @par i2c_master_event_t        input to assign i2c master call back event
 * @retval FSP_SUCCESS              Successfully closed.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_Register_I2C_event (rm_icm426xx_ctrl_t * const p_api_ctrl,volatile bool *p)
{
    rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;

    FSP_ASSERT(NULL != p_ctrl);

    /* assing gmaster_event address */
    p_ctrl->p_g_master_event = p;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief This function should be called to get SENSOR ID
 * Implements @ref rm_icm426xx_api_t::sensorIdGet.
 * @par p_sensor_id   output chip whoami value
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_SensorIdGet (rm_icm426xx_ctrl_t * const p_api_ctrl, uint32_t * const p_sensor_id)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;
    uint8_t who_am_i;

    FSP_ASSERT(NULL != p_ctrl);

    err = sci_i2c_master_read_register(p_ctrl,0x75,&who_am_i,1);
    printf("who_am_i: %x \r\n", who_am_i);

    (*p_sensor_id) = who_am_i;
    return err;
}

/*******************************************************************************************************************//**
 * @brief This function should be called to init sensor
 * Implements @ref rm_icm426xx_api_t::sensorInit.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_Sensor_Init (rm_icm426xx_ctrl_t * const p_api_ctrl, bool *init_flag)
{
    int ret;
    //rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;

    rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;

    FSP_ASSERT(NULL != p_ctrl);

    ret = inv_icm4x6xx_initialize();

    if (ret != 0) {
        printf("Chip Initialize Failed. Do nothing\r\n");
        return FSP_ERR_HW_LOCKED;
    }else{
        *init_flag = true;
    }
    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief This function should be called to enable Accelerate sensor
 * Implements @ref rm_icm426xx_api_t::accEnable.
 * @par enable  input sensor ACC to turn on or turn off
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_Acc_Enable(rm_icm426xx_ctrl_t * const p_api_ctrl, bool enable)
{
    int ret;
    rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;
    
    FSP_ASSERT(NULL != p_ctrl);

    if(enable){
        ret = inv_icm4x6xx_acc_enable();
        p_ctrl->sensor[ACC] = true;
    }else{
        ret = inv_icm4x6xx_acc_disable();
        p_ctrl->sensor[ACC] = false;
    }

    if (ret != 0) {
        printf("RM_ICM426XX_Acc_Enable fail. Do nothing\r\n");
        return FSP_ERR_HW_LOCKED;
    }
    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief This function should be called to enable Gyroscope sensor
 * Implements @ref rm_icm426xx_api_t::gyroEnable.
 * @par enableinput to set sensor Gyro to turn on or turn off
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_Gyro_Enable(rm_icm426xx_ctrl_t * const p_api_ctrl, bool enable)
{
    int ret;
    rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;
    
    FSP_ASSERT(NULL != p_ctrl);

    if(enable){
        ret = inv_icm4x6xx_gyro_enable();
        p_ctrl->sensor[GYR] = true;
    }else{
        ret = inv_icm4x6xx_gyro_disable();
        p_ctrl->sensor[GYR] = false;
    }

    if (ret != 0) {
        printf("RM_ICM426XX_Gyro_Enable fail. Do nothing\r\n");
        return FSP_ERR_HW_LOCKED;
    }
    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief This function should be called to set Accelerate odr
 * Implements @ref rm_icm426xx_api_t::accEnable.
 * @par odr         input to set acc odr rate
 * @par watermark   input to set acc fifo watermark (sample record value)

 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_Acc_SetRate(rm_icm426xx_ctrl_t * const p_api_ctrl, float odr_hz, uint16_t watermark)
{
    float hw_rate = 0.0;
    int ret;
    rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;
    
    FSP_ASSERT(NULL != p_ctrl);

    p_ctrl->acc_require_odr = odr_hz;

    ret = inv_icm4x6xx_acc_set_rate(odr_hz, watermark,&hw_rate);

    if (ret != 0) {
        printf("RM_ICM426XX_Acc_SetRate fail. Do nothing\r\n");
        return FSP_ERR_HW_LOCKED;
    }

    p_ctrl->acc_report_odr = hw_rate;
    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief This function should be called to set gyroscope odr
 * Implements @ref rm_icm426xx_api_t::accEnable.
 * @par odr         input to set gyro odr rate
 * @par watermark   input to set gyro fifo watermark (sample record value)
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_Gyro_SetRate(rm_icm426xx_ctrl_t * const p_api_ctrl, float odr_hz, uint16_t watermark)
{
    float hw_rate = 0.0;
    int ret;
    rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;
    
    FSP_ASSERT(NULL != p_ctrl);

    p_ctrl->gyro_require_odr = odr_hz;

    ret = inv_icm4x6xx_gyro_set_rate(odr_hz, watermark,&hw_rate);

    if (ret != 0) {
        printf("RM_ICM426XX_Gyro_SetRate fail. Do nothing\r\n");
        return FSP_ERR_HW_LOCKED;
    }

    p_ctrl->gyro_report_odr = hw_rate;
    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief This function should be called to get Accel and Gyroscope sensor data when enable
 * Implements @ref rm_icm426xx_api_t:: get_sensordata.
 * @par AccDataPacket output data for 3 axis value and timestamp
 * @par GyroDataPacket output data for 3 axis value and timestamp

 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.

 **********************************************************************************************************************/
fsp_err_t RM_ICM426XX_Sensor_GetData(rm_icm426xx_ctrl_t * const p_api_ctrl,
                   AccDataPacket *accDatabuff,GyroDataPacket *gyroDatabuff,
                                              chip_temperature*chip_temper)
{
    rm_icm426xx_instance_ctrl_t * p_ctrl = (rm_icm426xx_instance_ctrl_t *) p_api_ctrl;

    FSP_ASSERT(NULL != p_ctrl);
    
#if POLLING_MODE_SUPPORT
	polling_read_data();
#else
    inv_data_handler(p_ctrl,accDatabuff,gyroDatabuff,chip_temper);
#endif
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Internal icm426xx private function.
 **********************************************************************************************************************/
