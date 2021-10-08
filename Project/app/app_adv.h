/**
 ****************************************************************************************
 *
 * @file app_adv.h
 * @brief Advertising data create and send.
 * @date Mon, Jan  7, 2019  4:31:27 PM
 * @author chenzhiyuan
 *
 *
 ****************************************************************************************
 */

#ifndef APP_ADV_H_
#define APP_ADV_H_

/**
 ****************************************************************************************
 * @addtogroup APP_COMMON_ADV app_adv.h
 * @ingroup APP_COMMON
 *
 * @brief Advertising data create and send.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_PRESENT)

#include <stdint.h>          // Standard Integer Definition
#include <co_bt.h>           // Common BT Definitions
#include "arch.h"            // Platform Definitions
#include "gapc.h"            // GAPC Definitions

#if (NVDS_SUPPORT)
#include "nvds.h"
#endif // (NVDS_SUPPORT)

/*
 * DEFINES
 ****************************************************************************************
 */
/// the adv data max len
#define APP_ADV_MAX_LEN                   (252-3)
/// the adv manufacturer spec data max len
#define APP_ADV_MANUF_SPEC_DATA_MAX_LEN   250 // (APP_ADV_MAX_LEN-2-3(flags))
/*
 * MACROS
 ****************************************************************************************
 */


/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/**
 * @defgroup AD_TYPE_DEFINITIONS GAP Advertising and Scan Response Data format
 * @note Found at https://www.bluetooth.org
 * @{ */
#define APP_ADV_DATATYPE_FLAGS                               0x01 /**< Flags for discoverability. */
#define APP_ADV_DATATYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE   0x02 /**< Partial list of 16 bit service UUIDs. */
#define APP_ADV_DATATYPE_16BIT_SERVICE_UUID_COMPLETE         0x03 /**< Complete list of 16 bit service UUIDs. */
#define APP_ADV_DATATYPE_32BIT_SERVICE_UUID_MORE_AVAILABLE   0x04 /**< Partial list of 32 bit service UUIDs. */
#define APP_ADV_DATATYPE_32BIT_SERVICE_UUID_COMPLETE         0x05 /**< Complete list of 32 bit service UUIDs. */
#define APP_ADV_DATATYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE  0x06 /**< Partial list of 128 bit service UUIDs. */
#define APP_ADV_DATATYPE_128BIT_SERVICE_UUID_COMPLETE        0x07 /**< Complete list of 128 bit service UUIDs. */
#define APP_ADV_DATATYPE_SHORT_LOCAL_NAME                    0x08 /**< Short local device name. */
#define APP_ADV_DATATYPE_COMPLETE_LOCAL_NAME                 0x09 /**< Complete local device name. */
#define APP_ADV_DATATYPE_TX_POWER_LEVEL                      0x0A /**< Transmit power level. */
#define APP_ADV_DATATYPE_CLASS_OF_DEVICE                     0x0D /**< Class of device. */
#define APP_ADV_DATATYPE_SIMPLE_PAIRING_HASH_C               0x0E /**< Simple Pairing Hash C. */
#define APP_ADV_DATATYPE_SIMPLE_PAIRING_RANDOMIZER_R         0x0F /**< Simple Pairing Randomizer R. */
#define APP_ADV_DATATYPE_SECURITY_MANAGER_TK_VALUE           0x10 /**< Security Manager TK Value. */
#define APP_ADV_DATATYPE_SECURITY_MANAGER_OOB_FLAGS          0x11 /**< Security Manager Out Of Band Flags. */
#define APP_ADV_DATATYPE_SLAVE_CONNECTION_INTERVAL_RANGE     0x12 /**< Slave Connection Interval Range. */
#define APP_ADV_DATATYPE_SOLICITED_SERVICE_UUIDS_16BIT       0x14 /**< List of 16-bit Service Solicitation UUIDs. */
#define APP_ADV_DATATYPE_SOLICITED_SERVICE_UUIDS_128BIT      0x15 /**< List of 128-bit Service Solicitation UUIDs. */
#define APP_ADV_DATATYPE_SERVICE_DATA                        0x16 /**< Service Data - 16-bit UUID. */
#define APP_ADV_DATATYPE_PUBLIC_TARGET_ADDRESS               0x17 /**< Public Target Address. */
#define APP_ADV_DATATYPE_RANDOM_TARGET_ADDRESS               0x18 /**< Random Target Address. */
#define APP_ADV_DATATYPE_APPEARANCE                          0x19 /**< Appearance. */
#define APP_ADV_DATATYPE_ADVERTISING_INTERVAL                0x1A /**< Advertising Interval. */ 
#define APP_ADV_DATATYPE_LE_BLUETOOTH_DEVICE_ADDRESS         0x1B /**< LE Bluetooth Device Address. */
#define APP_ADV_DATATYPE_LE_ROLE                             0x1C /**< LE Role. */
#define APP_ADV_DATATYPE_SIMPLE_PAIRING_HASH_C256            0x1D /**< Simple Pairing Hash C-256. */
#define APP_ADV_DATATYPE_SIMPLE_PAIRING_RANDOMIZER_R256      0x1E /**< Simple Pairing Randomizer R-256. */
#define APP_ADV_DATATYPE_SERVICE_DATA_32BIT_UUID             0x20 /**< Service Data - 32-bit UUID. */
#define APP_ADV_DATATYPE_SERVICE_DATA_128BIT_UUID            0x21 /**< Service Data - 128-bit UUID. */
#define APP_ADV_DATATYPE_3D_INFORMATION_DATA                 0x3D /**< 3D Information Data. */
#define APP_ADV_DATATYPE_MANUFACTURER_SPECIFIC_DATA          0xFF /**< Manufacturer Specific Data. */
/**@} */


/// Advertising state machine
enum app_adv_state
{
    /// Advertising activity does not exists
    APP_ADV_STATE_IDLE = 0,
    /// Creating advertising activity
    APP_ADV_STATE_CREATING,
    /// Setting advertising data
    APP_ADV_STATE_SETTING_ADV_DATA,
    /// Setting scan response data
    APP_ADV_STATE_SETTING_SCAN_RSP_DATA,

    /// Advertising activity created
    APP_ADV_STATE_CREATED,
    /// Starting advertising activity
    APP_ADV_STATE_STARTING,
    /// Advertising activity started
    APP_ADV_STATE_STARTED,
    /// Stopping advertising activity
    APP_ADV_STATE_STOPPING,
    /// deleting advertising activity
    APP_ADV_STATE_DELETING,

};

/// Advertising data type
enum app_adv_data_include
{
    APP_ADV_DATA_INCLUDE_APPEARANCE      = 0x01,
    APP_ADV_DATA_INCLUDE_NAME            = 0x02,
    APP_ADV_DATA_INCLUDE_TX_POWER_LEVEL  = 0x04,
    APP_ADV_DATA_INCLUDE_UUID            = 0x08,
    APP_ADV_DATA_INCLUDE_SPEC_DATA       = 0x10,
};

/**@brief Advertising modes.
*/
enum app_adv_mode_type
{
    APP_ADV_MODE_IDLE,     /**< Idle; no connectable advertising is ongoing.*/
    APP_ADV_MODE_DIRECTED, /**< Directed advertising attempts to connect to the most recently disconnected peer. */
    APP_ADV_MODE_FAST,     /**< Fast advertising will connect to any peer device, or filter with a whitelist if one exists. */
    APP_ADV_MODE_SLOW,     /**< Slow advertising is similar to fast advertising. By default, it uses a longer advertising interval and time-out than fast advertising. However, these options are defined by the user. */
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// Advertising uuid structure
struct app_adv_uuid_tag
{
	/// uuid type
    uint8_t type;
	/// uuid len
    uint8_t len;
	/// uuid value
    uint8_t uuid[16];
};

/**@struct app_adv_manuf_spec_data_tag .*/
/**@brief Advertising manuf spec data structure. */
struct app_adv_manuf_spec_data_tag
{
   /// data len
    uint8_t len; 
   /// data value
    uint8_t data[APP_ADV_MANUF_SPEC_DATA_MAX_LEN];
};

/**@struct app_adv_data_tag 
 * @brief Advertising data structure. */
struct app_adv_data_tag
{
    /// bit0:flags;bit1:appearance:bit2:name;bit3:tx power,bit4:uuid;bit5:manuf spec data
    uint8_t                             include;  
    /// Appearance Numbers.
    uint16_t                            appearance;   
    /// Type of device name.
    uint8_t                             name_type;    
    /// TX Power Level field.
    int8_t                              tx_power_level;     
    /// uuid s.
    struct app_adv_uuid_tag             uuid;        
    /// Manufacturer specific data.
    struct app_adv_manuf_spec_data_tag  manuf_specific_data;   
};

/**@struct app_adv_modes_config_tag .*/
/**@brief Advertising mode config structure. */
struct app_adv_modes_config_tag
{
    bool     whitelist_enabled; /**< Enable or disable use of the whitelist. */
    bool     directed_enabled;  /**< Enable or disable direct advertising mode. */
    uint32_t directed_timeout;  /**< Time-out (number of tries) for direct advertising. */
    bool     fast_enabled;      /**< Enable or disable fast advertising mode. */
    uint32_t fast_interval;     /**< Advertising interval for fast advertising. */
    uint32_t fast_timeout;      /**< Time-out (in seconds) for fast advertising. */
    bool     slow_enabled;      /**< Enable or disable slow advertising mode. */
    uint32_t slow_interval;     /**< Advertising interval for slow advertising. */
    uint32_t slow_timeout;      /**< Time-out (in seconds) for slow advertising. */
};
/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Init advertising
 *
 * @param[in] adv_data: the adv data info.
 * @param[in] config: the mode conifg info.
 * ****************************************************************************************
 */
void appm_adv_init(struct app_adv_data_tag* adv_data, struct app_adv_modes_config_tag *config);

/**
 ****************************************************************************************
 * @brief set adv data
 *
 * @param[in] adv_data: adv data.
 * @param[in] len: adv data len.
 * ****************************************************************************************
 */
void appm_adv_set_adv_data(uint8_t* adv_data, uint8_t len);

/**
 ****************************************************************************************
 * @brief set scan response data
 *
 * @param[in] res_data: response data.
 * @param[in] len: response data len.
 * ****************************************************************************************
 */
void appm_adv_set_res_data(uint8_t* res_data, uint8_t len);

/**
 ****************************************************************************************
 * @brief Start/stop advertising
 *
 * @param[in] start     True if advertising has to be started, else false
 ****************************************************************************************
 */
void appm_adv_update_state(bool start);

/**
 ****************************************************************************************
 * @brief delete advertising
 *
 * @return void
 ****************************************************************************************
 */

void appm_adv_delete_advertising(void);

/**
 ****************************************************************************************
 * @brief create ind handler
 *
 * @param[in] p_param: created activity param.
 * ****************************************************************************************
 */
void appm_adv_create_ind_handler(void *p_param);

/**
 ****************************************************************************************
 * @brief stop ind handler
 *
 * @param[in] p_param: stop activity param.
 * ****************************************************************************************
 */
void appm_adv_stopped_ind_handler(void *p_param);


/**
 ****************************************************************************************
 * @brief complete event handler
 *
 * @param[in] param: the complete param.
 * ****************************************************************************************
 */
void appm_adv_cmp_evt_handler(void *param);

/**
 ****************************************************************************************
 * @brief start adv, after appm_adv_init and appm_adv_set_adv_data
 *
 * @return void
 * ****************************************************************************************
 */
void appm_adv_start(void);

/**
 ****************************************************************************************
 * @brief stop adv
 *
 * @param[in] is_del  is delete activity. default 0.
 * ****************************************************************************************
 */
void appm_adv_stop(uint8_t is_del);

/**
 ****************************************************************************************
 * @brief set adv mode
 *
 * @param[in] mode @ref app_adv_mode_type
 * ****************************************************************************************
 */
void appm_adv_set_mode(uint8_t mode);

/**
 ****************************************************************************************
 * @brief get adv state
 *
 * @return the adv state @ref app_adv_state
 * ****************************************************************************************
 */
uint8_t appm_adv_get_state(void);

#endif //(BLE_APP_PRESENT)

/// @} APP

#endif // APP_ADV_H_
