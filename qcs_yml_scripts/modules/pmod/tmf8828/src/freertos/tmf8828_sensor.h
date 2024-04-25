#ifndef TMF8828_SENSOR_H_
#define TMF8828_SENSOR_H_

#include "common_utils.h"
#include "sensor_thread.h"

#define TMF8828_CFG_DEVICE_TYPE (1)

void g_tmf8828_quick_setup(void);
bool g_tmf8828_sensor0_read(int obj0conf[][8], int obj0dist[][8], int obj1conf[][8], int obj1dist[][8]);

#endif /* TMF8828_SENSOR_H_ */
