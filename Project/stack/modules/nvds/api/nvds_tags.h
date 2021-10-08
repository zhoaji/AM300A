/**
 * @file nvds_tags.h
 * @brief 
 * @date Thu, Feb 14, 2019  3:20:35 PM
 * @author liqiang
 *
 * @defgroup 
 * @ingroup 
 * @brief 
 * @details 
 *
 * @{
 */

#ifndef __NVDS_TAGS_H__
#define __NVDS_TAGS_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */


/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */
enum NVDS_TAGS
{
    /// Definition of the tag associated to each parameters
    /// Local Bd Address
    NVDS_TAG_BD_ADDRESS                 = 0x01,
    /// Device Name
    NVDS_TAG_DEVICE_NAME                = 0x02,
    /// loc irk
    NVDS_TAG_LOC_IRK                    = 0x03,
    /// Radio Drift
    NVDS_TAG_LPCLK_DRIFT                = 0x07,
    /// Radio Jitter
    NVDS_TAG_LPCLK_JITTER               = 0x08,
    /// External wake-up time
    NVDS_TAG_EXT_WAKEUP_TIME            = 0x0D,
    /// Oscillator wake-up time
    NVDS_TAG_OSC_WAKEUP_TIME            = 0x0E,
    /// Radio wake-up time
    NVDS_TAG_RM_WAKEUP_TIME             = 0x0F,
    /// UART baudrate
    NVDS_TAG_UART_BAUDRATE              = 0x10,
    /// Enable sleep mode
    NVDS_TAG_SLEEP_ENABLE               = 0x11,
    /// Enable External Wakeup
    NVDS_TAG_EXT_WAKEUP_ENABLE          = 0x12,
    /// SP Private Key 192
    NVDS_TAG_SP_PRIVATE_KEY_P192        = 0x13,
    /// SP Public Key 192
    NVDS_TAG_SP_PUBLIC_KEY_P192         = 0x14,

    /// Activity Move Configuration (enables/disables activity move for BLE connections and BT (e)SCO links)
    NVDS_TAG_ACTIVITY_MOVE_CONFIG       = 0x15,

    /// Enable/disable scanning for extended advertising PDUs
    NVDS_TAG_SCAN_EXT_ADV               = 0x16,

    /// Duration of the schedule reservation for long activities such as scan, inquiry, page, HDC advertising
    NVDS_TAG_SCHED_SCAN_DUR             = 0x17,

    /// Synchronous links configuration
    NVDS_TAG_SYNC_CONFIG                = 0x2C,
    /// PCM Settings
    NVDS_TAG_PCM_SETTINGS               = 0x2D,
    /// Sleep algorithm duration
    NVDS_TAG_SLEEP_ALGO_DUR             = 0x2E,
    /// Tracer configuration
    NVDS_TAG_TRACER_CONFIG              = 0x2F,

    /// Diagport configuration
    NVDS_TAG_DIAG_BT_HW                 = 0x30,
    NVDS_TAG_DIAG_BLE_HW                = 0x31,
    NVDS_TAG_DIAG_SW                    = 0x32,
    NVDS_TAG_DIAG_DM_HW                 = 0x33,
    NVDS_TAG_DIAG_PLF                   = 0x34,

    /// IDC selection (for audio demo)
    NVDS_TAG_IDCSEL_PLF                 = 0x37,

    /// RSSI threshold tags
    NVDS_TAG_RSSI_HIGH_THR              = 0x3A,
    NVDS_TAG_RSSI_LOW_THR               = 0x3B,
    NVDS_TAG_RSSI_INTERF_THR            = 0x3C,

    /// RF BTIPT
    NVDS_TAG_RF_BTIPT_VERSION           = 0x3E,
    NVDS_TAG_RF_BTIPT_XO_SETTING        = 0x3F,

    /// ER/IR/Static Address
    NVDS_TAG_LOCAL_ER                   = 0x64,
    NVDS_TAG_LOCAL_IR                   = 0x65,
    NVDS_TAG_OTAS_STATIC_ADDRESS        = 0x66,
    NVDS_TAG_APP_STATIC_ADDRESS         = 0x67,

    NVDS_TAG_BT_LINK_KEY_FIRST          = 0x68,
    NVDS_TAG_BT_LINK_KEY_LAST           = 0x6F,

    NVDS_TAG_BLE_LINK_KEY_FIRST         = 0x70,
    NVDS_TAG_BLE_LINK_KEY_LAST          = 0x7F,
    /// SC Private Key (Low Energy)
    NVDS_TAG_LE_PRIVATE_KEY_P256        = 0x80,
    /// SC Public Key (Low Energy)
    NVDS_TAG_LE_PUBLIC_KEY_P256         = 0x81,
    /// SC Debug: Used Fixed Private Key from NVDS (Low Energy)
    NVDS_TAG_LE_DBG_FIXED_P256_KEY      = 0x82,
    /// SP Private Key (classic BT)
    NVDS_TAG_SP_PRIVATE_KEY_P256        = 0x83,
    /// SP Public Key (classic BT)
    NVDS_TAG_SP_PUBLIC_KEY_P256         = 0x84,

    /// LE Coded PHY 500 Kbps selection
    NVDS_TAG_LE_CODED_PHY_500           = 0x85,

    /// Application specific
    NVDS_TAG_APP_SPECIFIC_FIRST         = 0x90,
    NVDS_TAG_APP_SPECIFIC_LAST          = 0xAF,
};

enum NVDS_TAGS_LEN
{
     // Definition of length associated to each parameters
     /// Local Bd Address
     NVDS_LEN_BD_ADDRESS                 = 6,
     /// Device Name
     NVDS_LEN_DEVICE_NAME                = 248,
     /// loc irk
     NVDS_LEN_LOC_IRK                    = 16,
     /// Low power clock drift
     NVDS_LEN_LPCLK_DRIFT                = 2,
     /// Low power clock jitter
     NVDS_LEN_LPCLK_JITTER               = 1,
     /// External wake-up time
     NVDS_LEN_EXT_WAKEUP_TIME            = 2,
     /// Oscillator wake-up time
     NVDS_LEN_OSC_WAKEUP_TIME            = 2,
     /// Radio wake-up time
     NVDS_LEN_RM_WAKEUP_TIME             = 2,
     /// UART baudrate
     NVDS_LEN_UART_BAUDRATE              = 4,
     /// Enable sleep mode
     NVDS_LEN_SLEEP_ENABLE               = 1,
     /// Enable External Wakeup
     NVDS_LEN_EXT_WAKEUP_ENABLE          = 1,
     /// SP Private Key 192
     NVDS_LEN_SP_PRIVATE_KEY_P192        = 24,
     /// SP Public Key 192
     NVDS_LEN_SP_PUBLIC_KEY_P192         = 48,

     /// Activity Move Configuration
     NVDS_LEN_ACTIVITY_MOVE_CONFIG       = 1,

     /// Enable/disable scanning for extended advertising PDUs
     NVDS_LEN_SCAN_EXT_ADV               = 1,

     /// Duration of the schedule reservation for long activities such as scan, inquiry, page, HDC advertising
     NVDS_LEN_SCHED_SCAN_DUR             = 2,

     /// Synchronous links configuration
     NVDS_LEN_SYNC_CONFIG                = 2,
     /// PCM Settings
     NVDS_LEN_PCM_SETTINGS               = 8,
     /// Tracer configuration
     NVDS_LEN_TRACER_CONFIG              = 4,

     /// Diagport configuration
     NVDS_LEN_DIAG_BT_HW                 = 4,
     NVDS_LEN_DIAG_BLE_HW                = 4,
     NVDS_LEN_DIAG_SW                    = 4,
     NVDS_LEN_DIAG_DM_HW                 = 4,
     NVDS_LEN_DIAG_PLF                   = 4,

     /// IDC selection (for audio demo)
     NVDS_LEN_IDCSEL_PLF                 = 4,

     /// RSSI thresholds
     NVDS_LEN_RSSI_THR                   = 1,

     /// RF BTIPT
     NVDS_LEN_RF_BTIPT_VERSION          = 1,
     NVDS_LEN_RF_BTIPT_XO_SETTING       = 1,

    /// Static Address
    NVDS_LEN_STATIC_ADDRESS             = 6,
    NVDS_LEN_OTAS_STATIC_ADDRESS        = 6,

     /// Link keys
     NVDS_LEN_BT_LINK_KEY                = 22,
     NVDS_LEN_BLE_LINK_KEY               = 48,

     /// P256
     NVDS_LEN_PRIVATE_KEY_P256           = 32,
     NVDS_LEN_PUBLIC_KEY_P256            = 64,
     NVDS_LEN_DBG_FIXED_P256_KEY         = 1,

     /// LE Coded PHY 500 Kbps selection
     NVDS_LEN_LE_CODED_PHY_500           = 1,
};

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */


#ifdef __cplusplus
}
#endif

#endif

/** @} */

