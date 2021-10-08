/**
 ****************************************************************************************
 *
 * @file mal_int.h
 *
 * @brief Header file for Mesh Abstraction Layer internals
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef MAL_INT_
#define MAL_INT_

/**
 ****************************************************************************************
 * @defgroup MAL Mesh Abstraction Layer
 * @ingroup MESH
 * @brief  Mesh Abstraction Layer
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mal_lib.h"
#include "mal.h"            // Mesh Abstraction Layer
#include "mesh_defines.h"
#include "gap.h"
#include "gapm_task.h"
#include "gapc_task.h"       // GAP Client Task Definitions
#include "gattc.h"           // GATT Client Definitions
#include "gattc_task.h"      // GATT Client Task Definitions
#include "ke_task.h"
#include "ke_timer.h"
#include "ke_mem.h"
#include "prf.h"
#include "sch_alarm.h"      // Scheduling Alarm Definitions

/*
 * DEFINES
 ****************************************************************************************
 */


/// Number of instance of abstraction layer task
#define MAL_IDX_MAX             (1)
/// Advertising channel map - 37, 38, 39
#define MAL_ADV_CHMAP           (0x07)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for internal mesh messages
enum mesh_int_msg_id
{
    MESH_FIRST_INT_MSG         = TASK_FIRST_MSG(TASK_ID_MESH) + 200,
    /// Kernel message sent when PDU reassembly expires
    MESH_CON_TIMER_FIRST_IND   = MESH_FIRST_INT_MSG,
    MESH_CON_TIMER_LAST_IND    = (MESH_CON_TIMER_FIRST_IND + BLE_CONNECTION_MAX),
};

/// Activity type
enum mal_act_type
{
    /// No activity
    MAL_ACT_NONE,
    /// Advertising activity
    MAL_ACT_ADV,
    /// Scanning activity
    MAL_ACT_SCAN,
    #if (BLE_MESH_GATT_BEARER)
    /// Connectable Advertising activity
    MAL_ACT_CON_ADV,
    #endif // (BLE_MESH_GATT_BEARER)
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Structure for Timer Abstraction Layer environment
typedef struct mal_timer_env
{
    /// Alarm element
    struct sch_alarm_tag alarm;
    /// Delayed job structure
    mal_djob_t timer_djob;
    /// Last read local time value (half slots)
    uint32_t last_read_hs;
    /// Remaining delay to program (in milliseconds)
    uint32_t rem_delay_ms;
    /// Clock (half slots part)
    uint32_t clock_hs;

    /// Clock (milliseconds part)
    uint32_t clock_ms;
    /// Clock (wrap part)
    uint16_t clock_wrap;
    /// Clock (half microseconds part)
    uint16_t clock_hus;
} mal_timer_env_t;

/// Advertising environment definition
typedef struct mal_adv_env
{
    /// Pointer to the data to transmit
    const uint8_t*           p_data;
    /// Length of data to transmit
    uint16_t                 data_len;
    /// Packet transmission interval (in slots)
    uint16_t                 interval;
    /// Number of packet transmission to perform
    uint8_t                  nb_tx;
    /// Status of the Advertising
    uint8_t                  state;
    /// Advertising activity identifier
    uint8_t                  act_id;
    /// Identifier of bearer that handle packet transmission
    m_lid_t                  bearer_lid;
} mal_adv_env_t;


/// Scanning environment definition
typedef struct mal_scan_env
{
    ///Scan window interval (in slots)
    uint16_t                 interval;
    /// Status of the scanning
    uint8_t                  state;
    /// Scanning activity identifier
    uint8_t                  act_id;
    /// Identifier of bearer that handle packet reception
    m_lid_t                  bearer_lid;
} mal_scan_env_t;

/// Structure Delayed job execution
typedef struct mal_djob_env
{
    /// Job queue to execute
    co_list_t job_queue;
} mal_djob_env_t;


/// Structure for security environment
typedef struct mal_sec_env
{
    /// Call-back to read Public key
    mal_sec_pub_key_res_cb     cb_pub_key;
    /// Call-back for DH-Key generation
    mal_sec_ecdh_secret_res_cb cb_dh_key;
} mal_sec_env_t;

/// Structure that handle connection
typedef struct mal_con
{
    /// Pointer of data to transmit
    const uint8_t* p_tx_data;
    /// Queue of received message
    co_list_t      rx_queue;
    /// Reception message type
    uint8_t        rx_type;
    /// Length of received PDU
    uint8_t        rx_length;
    /// Transmission message type
    uint8_t        tx_type;
    /// Transmission data offset
    uint8_t        tx_offset;
    /// Transmission length
    uint8_t        tx_length;
    /// Notification configuration
    uint8_t        ntf_cfg;
    /// Identifier of bearer that handle connection
    m_lid_t        bearer_lid;
} mal_con_t;

#if (BLE_MESH_GATT_BEARER)
/// Structure for Connection module
typedef struct mal_con_env
{
    /// Connection specific parameters
    mal_con_t*          p_con[BLE_CONNECTION_MAX];
    /// Callback structure
    const mal_con_cb_t* p_cb;
    /// Pointer to the advertising data
    const uint8_t*       p_adv_data;
    /// Length of advertising data
    uint16_t             adv_data_len;
    /// Provisioning service start handle
    uint16_t             prov_start_hdl;
    /// Proxy service start handle
    uint16_t             proxy_start_hdl;
    /// Connectable advertising interval (unit 1 slot)
    uint16_t             interval;
    /// Supported connection type
    uint8_t              con_type;
    /// Advertising state machine
    uint8_t              adv_state;
    /// Advertising activity identifier
    uint8_t              adv_act_id;
    /// Connection index of created connection
    uint8_t              conidx;
    /// Number of Mesh GATT Active connections
    uint8_t              nb_con;
} mal_con_env_t;
#endif // (BLE_MESH_GATT_BEARER)


/// Activity manager environment
typedef struct mal_activity_env
{
    /// List of command to process
    co_list_t cmd_queue;
    /// Source module that request command under execution (@see enum mal_act_type)
    uint8_t   cmd_src;
} mal_activity_env_t;

/// Mesh Abstraction layer Environment Variable
typedef struct mal_env
{
    /// Profile environment
    prf_env_t            prf_env;
    /// Callback for advertising bearer
    const mal_adv_cb_t* p_adv_cb;
    /// Security module environment
    mal_sec_env_t       sec;
    /// Timer Environment
    mal_timer_env_t     timer;
    /// Activity manager environment
    mal_activity_env_t  activity;
    /// Advertising environment
    mal_adv_env_t       adv;
    /// Scanning environment
    mal_scan_env_t      scan;
    #if (BLE_MESH_GATT_BEARER)
    /// Connection oriented environment
    mal_con_env_t       con;
    #endif // (BLE_MESH_GATT_BEARER)
    /// Delayed Job environment
    mal_djob_env_t      djob;

    /// Mesh task state
    ke_state_t           state[MAL_IDX_MAX];
} mal_env_t;


/*
 * GLOBAL VARIABLE
 ****************************************************************************************
 */

/// Mesh Abstraction Layer Environment
extern mal_env_t *p_mal_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize encryption block for Mesh abstraction layer
 *
 * @param[in] reset     Initialization due to a reset
 ****************************************************************************************
 */
void mal_sec_init(bool reset);

/**
 ****************************************************************************************
 * @brief Request HL to execute an AES function
 *
 * @param[in] key       128 bits key
 * @param[in] val       128 bits value to encrypt with AES
 ****************************************************************************************
 */
void mal_sec_aes_req(const uint8_t* key, const uint8_t* val);

/**
 ****************************************************************************************
 * @brief Initialize timer block for Mesh abstraction layer
 *
 * @param[in] reset Initialization due to a reset
 ****************************************************************************************
 */
void mal_timer_init(bool reset);

/**
 ****************************************************************************************
 * @brief Initialize activity block for Mesh abstraction layer
 *
 * @param[in] reset Initialization due to a reset
 ****************************************************************************************
 */
void mal_activity_init(bool reset);


/**
 ****************************************************************************************
 * @brief Request to send an activity command message to the HL stack
 *
 * @param[in] src      Module source (@see enum mal_act_type)
 * @param[in] p_param  Command message parameters
 ****************************************************************************************
 */
void mal_activity_cmd_send(uint8_t src, void* p_param);


/**
 ****************************************************************************************
 * @brief Initialize Advertising block for Mesh abstraction layer
 *
 * @param[in] reset Initialization due to a reset
 ****************************************************************************************
 */
void mal_adv_init(bool reset);

/**
 ****************************************************************************************
 * @brief Initialize scanning block for Mesh abstraction layer
 *
 * @param[in] reset Initialization due to a reset
 ****************************************************************************************
 */
void mal_scan_init(bool reset);

/**
 ****************************************************************************************
 * @brief Start Scanning operation, Scanning is performed with a duty cycle of 100%
 *
 * @param[in] bearer_lid    Bearer LID
 * @param[in] interval      Scan window interval (in slots)
 * @param[in] p_cb_evts     Set of callback executed when an event occurs on advertising bearer
 *
 * @return Error status
 ****************************************************************************************
 */
uint16_t mal_scan_start(m_lid_t bearer_lid, uint16_t interval, const mal_adv_cb_t* p_cb_evts);

/**
 ****************************************************************************************
 * @brief Stop Scanning operation
 *
 * @param[in] bearer_lid    Bearer LID
 *
 * @return Error status
 ****************************************************************************************
 */
uint16_t mal_scan_stop(m_lid_t bearer_lid);

/**
 ****************************************************************************************
 * @brief Initialize Mesh Abstraction Layer Connection Module
 *
 * @param[in] reset             Initialization due to a reset or not
 * @param[in|out] p_start_hdl   Start handle of the GATT database
 * @param[in] mesh_task         Task number of Mesh Stack
 * @param[in] sec_lvl           Database security level requirements
 * @param[in] features          Database supported features (@see enum m_api_feat)
 *
 * @return HL error status
 ****************************************************************************************
 */
uint8_t mal_con_init(bool reset, uint16_t* p_start_hdl, uint16_t mesh_task, uint8_t sec_lvl,
                      uint32_t features);

/**
 ****************************************************************************************
 * @brief Inform Mesh Abstraction Layer Connection Module about link establishment
 *
 * @param[in] conidx        Connection index
 ****************************************************************************************
 */
void mal_con_create(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Inform Mesh Abstraction Layer Connection Module about link disconnection
 *
 * @param[in] conidx        Connection index
 * @param[in] reason        Disconnection reason
 ****************************************************************************************
 */
void mal_con_cleanup(uint8_t conidx, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Initialize Mesh Abstraction Layer Delayed Job Module
 *
 * @param[in] reset         Initialization due to a reset or not
 ****************************************************************************************
 */
void mal_djob_init(bool reset);

/**
 ****************************************************************************************
 * @brief Initialize Mesh Abstraction Layer Task Module
 *
 * @param[in] reset         Initialization due to a reset or not
 * @param[out] p_env        Allocated environment data
 * @param[in] app_task      Application task number
 * @param[in] sec_lvl       Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 ****************************************************************************************
 */
void mal_task_init(bool reset, struct prf_task_env *p_env, uint16_t app_task,
                    uint8_t sec_lvl);

/// @} MAL

#endif /* MAL_INT_ */
