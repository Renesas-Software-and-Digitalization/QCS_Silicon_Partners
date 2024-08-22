/*
    Global logging definitions

  Logging levels
  LOG_DISABLED - no logging
  LOG_ERROR - only error messages are logged
  LOG_WARNING - warning and error messages are logged
  LOG_INFO - information, warning, and error messages are logged
  LOG_DEBUG - debug, information, warning, and error messages are logged
*/

#define LOG_DISABLED    0
#define LOG_ERROR       1
#define LOG_WARNING     2
#define LOG_INFO        3
#define LOG_DEBUG       4

// In order to use Segger RTT for logging messages, uncomment the define below
// If left commented, logs use stderr and fprintf 
// Note: configure Segger RTT to use an existing connection with RTT control block set to
// search range 0x20000000 0x8000
#define LOG_OUTPUT_SEGGER_RTT

// Uncomment the following define and set a global logging level which will
// override the individual configurations of all modules
#define GLOBAL_LOGGING_LEVEL    LOG_DEBUG
