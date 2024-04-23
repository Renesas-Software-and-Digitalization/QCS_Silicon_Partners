#include "common_utils.h"
#include "sensor_thread.h"
#include "tmf8828_sensor.h"
#include "tmf8828/tmf8828.h"
// Specify sensor parameters //

void g_tmf8828_quick_setup(void)
{
    TMF8828_enable();
    TMF8828_factoryCalibration(0, false); //must be before startMeasuring
    TMF8828_startMeasuring();
}

/* Quick getting tmf8828 */
bool g_tmf8828_sensor0_read(int obj0conf[][8], int obj0dist[][8], int obj1conf[][8], int obj1dist[][8])
{
    return TMF8828_update8x8(obj0conf, obj0dist, obj1conf, obj1dist);
}
