/**
 ****************************************************************************************
 *
 * @file mesh_log.h
 *
 * @brief Header file for Mesh Stack logging system.
 *
 * Copyright (C) OnMicro 2019-2020
 *
 ****************************************************************************************
 */

#ifndef MESH_LOG_
#define MESH_LOG_

/**
 ****************************************************************************************
 * @defgroup MESH_LOG Mesh Stack Log
 * @ingroup MESH
 * @brief  Mesh Stack Log
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdarg.h>
#include <stddef.h>
#include "compiler.h"
#include "co.h"
#include "co_utils.h"
//#include "reg_ipcore.h"

/*
 * COMPILATION FLAGS
 ****************************************************************************************
 */
/** Enable logging module. */
#define MESH_LOG_ENABLE 1

/*
 * DEFINES
 ****************************************************************************************
 */

/**
 * Defines various sources for logging messages.
 */
#define LOG_SRC_API             (1 <<  0) /**< Receive logs from the abstraction layer. */
#define LOG_SRC_BEARER          (1 <<  1) /**< Receive logs from the bearer layer. */
#define LOG_SRC_NETWORK         (1 <<  2) /**< Receive logs from the network layer. */
#define LOG_SRC_LTRANSPORT      (1 <<  3) /**< Receive logs from the lower transport layer. */
#define LOG_SRC_UTRANSPORT      (1 <<  4) /**< Receive logs from the upper transport layer. */
#define LOG_SRC_ACCESS          (1 <<  5) /**< Receive logs from the access layer. */
#define LOG_SRC_APP             (1 <<  6) /**< Receive logs from the app layer. */
#define LOG_SRC_PROV            (1 <<  7) /**< Receive logs from the provisioning module. */
#define LOG_SRC_FRIEND          (1 <<  8) /**< Receive logs from the friend layer. */
#define LOG_SRC_MODEL           (1 <<  9) /**< Receive logs from the model layer. */
#define LOG_SRC_PROXY           (1 <<  10) /**< Receive logs from the proxy layer. */
#define LOG_SRC_DBG             (1 <<  11) /**< Receive logs from the debug use. */

/**
 * Defines possible criticality levels for logged messages.
 */
#define LOG_LEVEL_ASSERT ( 0) /**< Log level for assertions */
#define LOG_LEVEL_ERROR  ( 1) /**< Log level for error messages. */
#define LOG_LEVEL_WARN   ( 2) /**< Log level for warning messages. */
#define LOG_LEVEL_REPORT ( 3) /**< Log level for report messages. */
#define LOG_LEVEL_INFO   ( 4) /**< Log level for information messages. */
#define LOG_LEVEL_PTS    ( 5) /**< Log level for PTS messages. */
#define LOG_LEVEL_DBG1   ( 6) /**< Log level for debug messages (debug level 1). */
#define LOG_LEVEL_DBG2   ( 7) /**< Log level for debug messages (debug level 2). */
#define LOG_LEVEL_DBG3   ( 8) /**< Log level for debug messages (debug level 3). */

/**
 * Initializes the logging module.
 *
 * @param[in] mask     Mask specifying which modules to log information from.
 * @param[in] level    Maximum log level to print messages from.
 */
void log_init(uint32_t mask, uint32_t level);

__STATIC_INLINE uint32_t log_timestamp_get(void)
{
    uint32_t time = 0;
#if defined(HS6601) || defined(HS6620)
    time = HS_BTBB_LC->NATIVE_BT_CLK;
#else
    //ip_slotclk_samp_setf(1);
    //while (ip_slotclk_samp_getf());
    //time = ip_slotclk_sclk_getf();
    time = co_time();
#endif
    //return time * 312 + time / 2;
    return time * 30 + (time >> 1);
}

void __attribute((format(printf, 3, 4))) log_printf(
    uint32_t source, uint32_t timestamp, const char * format, ...);
void log_vprintf(uint32_t source, uint32_t timestamp, const char * format, va_list arguments);

#define __LOG_INIT(msk, level) log_init(msk, level)
#define mesh_log_init(msk,level) log_init(msk,level)

/**
 * Prints a log message.
 * @param[in] source Log source
 * @param[in] level  Log level
 * @param[in] ...    Arguments passed on to the callback (similar to @c printf)
 */
#if (MESH_LOG_ENABLE > 0)
/**
 * Initializes the logging framework.
 * @param[in] msk      Log mask
 * @param[in] level    Log level
 */
#define __LOG(source, level, ...)                               \
    if ((source & g_log_dbg_msk) && level <= g_log_dbg_lvl)     \
    {                                                           \
        log_printf(source, log_timestamp_get(), __VA_ARGS__);   \
    }

/**
 * Prints an array with a message.
 * @param[in] source Log source
 * @param[in] level  Log level
 * @param[in] msg    Message string
 * @param[in] array  Pointer to array
 * @param[in] len    Length of array (in bytes)
 */
#define __LOG_XB_MAX_SIZE (80)
#define __LOG_XB(source, level, msg, array, array_len)                      \
    {                                                                       \
        ASSERT_ERR(array_len < __LOG_XB_MAX_SIZE);                          \
        if ((source & g_log_dbg_msk) && (level <= g_log_dbg_lvl))           \
        {                                                                   \
            unsigned _array_len = (array_len);                              \
            char array_text[__LOG_XB_MAX_SIZE * 2 + 1];                     \
            for(unsigned _i = 0; _i < _array_len; ++_i)                     \
            {                                                               \
                extern const char * g_log_hex_digits;                       \
                const uint8_t array_elem = (array)[_i];                     \
                array_text[_i * 2] = g_log_hex_digits[(array_elem >> 4) & 0xf]; \
                array_text[_i * 2 + 1] = g_log_hex_digits[array_elem & 0xf]; \
            }                                                               \
            array_text[_array_len * 2] = 0;                                 \
            log_printf(source, log_timestamp_get(), "%s: %s\n", msg, array_text); \
        }                                                                         \
    }
#else
#define __LOG(...)
#define __LOG_XB(...)
#endif

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */

/** Global debug mask. The value of this variable is used to filter the log messages being printed. */
extern uint32_t g_log_dbg_msk;
/** Global log level. The value of this variable is used to filter the log messages being printed. */
extern int32_t g_log_dbg_lvl;


/// @} MESH_LOG

#endif /* MESH_LOG_ */
