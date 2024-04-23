#include "common_utils.h"
#include "icm42688_sensor.h"

rm_icm426xx_instance_ctrl_t g_icm426xx_sensor0_ctrl;
const rm_icm426xx_cfg_t g_icm426xx_sensor0_cfg =
{
 .p_instance = &g_comms_i2c_icm42688,
 .p_callback = NULL,
 .p_context  = NULL,
};

const rm_icm426xx_instance_t g_icm426xx_sensor0 =
{ .p_ctrl = &g_icm426xx_sensor0_ctrl, .p_cfg = &g_icm426xx_sensor0_cfg, .p_api = &g_icm426xx_on_icm426xx, };

volatile i2c_master_event_t g_master_event = (i2c_master_event_t)0x00;
/* Flag set from device irq handler */
volatile int irq_from_device = 0;

static AccDataPacket accData;
static GyroDataPacket gyroData;
static chip_temperature imu_chip_temperature;

/*******************************************************************************************************************//**
 *  @brief        User defined irq callback function
 *  @param[IN]    p_args
 *  @retval       None
 **********************************************************************************************************************/
void icm42688_irq_callback(external_irq_callback_args_t *p_args)
{
    if (NULL != p_args)
    {
        irq_from_device = 1;
    }
}

/* TODO: Enable if you want to open I2C Communications Device */
#define G_COMMS_I2C_DEVICE0_NON_BLOCKING (1)
#define RESET_VALUE 0x00

#if G_COMMS_I2C_DEVICE0_NON_BLOCKING
volatile bool g_i2c_completed = false;
#endif

/* TODO: Enable if you want to use a callback */
#define G_COMMS_I2C_DEVICE0_CALLBACK_ENABLE (1)
#if G_COMMS_I2C_DEVICE0_CALLBACK_ENABLE
void icm42688_callback(rm_comms_callback_args_t * p_args)
{
#if G_COMMS_I2C_DEVICE0_NON_BLOCKING
    if (RM_COMMS_EVENT_OPERATION_COMPLETE == p_args->event)
    {
        g_i2c_completed = true;
    }
#else
    FSP_PARAMETER_NOT_USED(p_args);
#endif
}
#endif

void g_icm42688_quick_setup(void)
{
    fsp_err_t err;
    bool sensor_init = false;

    /* Open icm42688 sensor instance, this must be done before calling any icm42688 API */
    err = g_icm426xx_sensor0.p_api->open(g_icm426xx_sensor0.p_ctrl, g_icm426xx_sensor0.p_cfg);
    assert(FSP_SUCCESS == err);
    /* register i2c event, this must be done after api before calling any icm42688 API =*/
    err = g_icm426xx_sensor0.p_api->register_i2c_evetnt(g_icm426xx_sensor0.p_ctrl,&g_i2c_completed);
    assert(FSP_SUCCESS == err);

    err = g_icm426xx_sensor0.p_api->sensorInit(g_icm426xx_sensor0.p_ctrl,&sensor_init);
    assert(FSP_SUCCESS == err);

    printf("sensor_init: %x \n", sensor_init);
    err = R_ICU_ExternalIrqOpen(&g_external_irq_icm42688_ctrl, &g_external_irq_icm42688_cfg);
    assert(FSP_SUCCESS == err);
}

/* Quick getting icm42688 data*/
void g_icm42688_sensor0_read(float *p_temp, float acc_data[3], float gyro_data[3])
{
    float acc_odr = 1.0f;
    uint16_t acc_watermark = 2;
    float gyro_odr = 1.0f;
    uint16_t gyro_watermark = 2;
    fsp_err_t err;

    err = g_icm426xx_sensor0.p_api->acc_Setodr(g_icm426xx_sensor0.p_ctrl,acc_odr,acc_watermark);
    assert(FSP_SUCCESS == err);

    err = g_icm426xx_sensor0.p_api->gyro_Setodr(g_icm426xx_sensor0.p_ctrl,gyro_odr,gyro_watermark);
    assert(FSP_SUCCESS == err);

    // enable gyro sensor
    err = g_icm426xx_sensor0.p_api->gyroEnable(g_icm426xx_sensor0.p_ctrl,true);
    assert(FSP_SUCCESS == err);

    // enable acc sensor
    err = g_icm426xx_sensor0.p_api->accEnable(g_icm426xx_sensor0.p_ctrl,true);
    assert(FSP_SUCCESS == err);

    err = R_ICU_ExternalIrqEnable(&g_external_irq_icm42688_ctrl);
    assert(FSP_SUCCESS == err);

    while(irq_from_device == 0){}

    err = R_ICU_ExternalIrqDisable(&g_external_irq_icm42688_ctrl);
    assert(FSP_SUCCESS == err);

    irq_from_device = 0;

    memset(&accData,0,sizeof(accData));
    memset(&gyroData,0,sizeof(gyroData));
    err = g_icm426xx_sensor0.p_api->get_sensordata(g_icm426xx_sensor0.p_ctrl,&accData,&gyroData,
                                                   &imu_chip_temperature);
    assert(FSP_SUCCESS == err);

    // enable gyro sensor
    err = g_icm426xx_sensor0.p_api->gyroEnable(g_icm426xx_sensor0.p_ctrl,false);
    assert(FSP_SUCCESS == err);

    // enable acc sensor
    err = g_icm426xx_sensor0.p_api->accEnable(g_icm426xx_sensor0.p_ctrl,false);
    assert(FSP_SUCCESS == err);

    for(int i=0;i<accData.accDataSize;i++){
        printf("receive ACC  %f %lu %f %f %f \r\n", imu_chip_temperature, (uint32_t)accData.databuff[i].timeStamp,
               accData.databuff[i].x,
               accData.databuff[i].y,
               accData.databuff[i].z);
    }

    for(int i=0;i<gyroData.gyroDataSize;i++){
        printf("receive gyro  %f %lu %f %f %f\r\n", imu_chip_temperature, (uint32_t)gyroData.databuff[i].timeStamp,
               gyroData.databuff[i].x,
               gyroData.databuff[i].y,
               gyroData.databuff[i].z);
    }

    *p_temp = (float)imu_chip_temperature;
    acc_data[0] = accData.databuff[0].x;
    acc_data[1] = accData.databuff[0].y;
    acc_data[2] = accData.databuff[0].z;

    gyro_data[0] = accData.databuff[0].x;
    gyro_data[1] = accData.databuff[0].y;
    gyro_data[2] = accData.databuff[0].z;
}
