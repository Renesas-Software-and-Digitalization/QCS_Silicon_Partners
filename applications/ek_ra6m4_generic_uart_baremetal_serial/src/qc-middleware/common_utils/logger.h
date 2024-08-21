/*
  Logging levels
  LOG_DISABLED - no logging
  LOG_ERROR - only error messages are logged
  LOG_WARNING - warning and error messages are logged
  LOG_INFO - information, warning, and error messages are logged
  LOG_DEBUG - debug, information, warning, and error messages are logged

  In order to select a debug level, include one of the following headers:
  #include "log_disabled.h"         // All logs are disabled
  #include "log_error.h"            // Enable error logging
  #include "log_warning.h"          // Enable error and warning logs
  #include "log_info.h"             // Enable error, warning and info logs
  #include "log_debug.h"            // Enable all logs

  Usage:
  log_error("This is an error");    // Will log on any logging level except for DISABLED
  log_error("The error is %d",1);   // Will log on any logging level except for DISABLED
  log_warning("This is a warning"); // Will log on any logging level except for DISABLED and LOG_ERROR
  log_info("This is an info");      // Will log on any logging level except for DISABLED, LOG_ERROR and LOG_WARNING
  log_debug("This is a debug");     // Will log only when LOG_DEBUG is selected
*/

#ifndef __LOGGER_H_
#define __LOGGER_H_

#include "global_logging_definitions.h"

#ifdef GLOBAL_LOGGING_LEVEL
  #undef LOGGING_LEVEL
  #define LOGGING_LEVEL GLOBAL_LOGGING_LEVEL
#endif
#ifndef LOGGING_LEVEL
  #define LOGGING_LEVEL LOG_DISABLED
#endif

#ifdef LOG_OUTPUT_SEGGER_RTT
  /* SEGGER RTT and error related headers */
  #include "SEGGER_RTT.h"
  #define SEGGER_INDEX      (0)
  #define _log_printf(...)       SEGGER_RTT_printf (SEGGER_INDEX, ##__VA_ARGS__)
#else
  #include <stdio.h>
  #define _log_printf(...)  fprintf(stderr, ##__VA_ARGS__)
#endif
#define _log_output(level, ...) _log_printf(level);_log_printf(__FILE__ " " __VA_ARGS__);_log_printf("\r\n");

#define _log_error(...)    _log_output("ERROR:",__VA_ARGS__);
#define _log_warning(...)  _log_output("WARNING:",__VA_ARGS__);
#define _log_info(...)     _log_output("INFO:",__VA_ARGS__);
#define _log_debug(...)    _log_output("DEBUG:",__VA_ARGS__);

#if LOGGING_LEVEL == LOG_ERROR
  #define log_error(...)     _log_error( __VA_ARGS__ )
  #define log_warning(...)
  #define log_info(...)
  #define log_debug(...)
#elif LOGGING_LEVEL == LOG_WARNING
  #define log_error(...)     _log_error( __VA_ARGS__ )
  #define log_warning(...)   _log_warning( __VA_ARGS__ )
  #define log_info(...)
  #define log_debug(...)
#elif LOGGING_LEVEL == LOG_INFO
  #define log_error(...)     _log_error( __VA_ARGS__ )
  #define log_warning(...)   _log_warning( __VA_ARGS__ )
  #define log_info(...)      _log_info( __VA_ARGS__ )
  #define log_debug(...)
#elif LOGGING_LEVEL == LOG_DEBUG
  #define log_error(...)    _log_error( __VA_ARGS__ )
  #define log_warning(...)  _log_warning( __VA_ARGS__ )
  #define log_info(...)     _log_info( __VA_ARGS__ )
  #define log_debug(...)    _log_debug( __VA_ARGS__ )
#else
  #define log_error(...)
  #define log_warning(...)
  #define log_info(...)
  #define log_debug(...)
#endif

#endif  // __LOGGER_H_