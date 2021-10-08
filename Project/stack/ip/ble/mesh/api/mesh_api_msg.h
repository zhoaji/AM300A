/**
 ****************************************************************************************
 *
 * @file mesh_api_msg.h
 *
 * @brief Header file for Mesh Stack Message Application Programming Interface
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef MESH_API_MSG_
#define MESH_API_MSG_

/**
 ****************************************************************************************
 * @defgroup MESH_API_MSG_ Mesh Stack Message Application Programming Interface
 * @ingroup MESH
 * @brief  Mesh Stack Message Application Programming Interface
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mesh_defines.h"
#include "mesh_config.h"
#include "mesh_api.h"

/*
 * MACROS
 ****************************************************************************************
 */

/// Retrieve message value from its index
#define MESH_API_MSG_VAL(id)        (TASK_FIRST_MSG(TASK_ID_MESH) + (id))

/*
 * ENUMERATIONS FOR MESH STACK
 ****************************************************************************************
 */

/// Mesh Stack Message Indexes
enum mesh_api_msg_id
{
    /* ******************************** */
    /* *         MESH PROFILE         * */
    /* ******************************** */

    /// Mesh Command
    MESH_API_FIRST                           = MESH_API_MSG_VAL(0),
    MESH_API_CMD                             = MESH_API_FIRST,
    /// Command Complete event
    MESH_API_CMP_EVT                         = MESH_API_MSG_VAL(1),

    /// Request model if a specific operation code is supported (shall be confirmed with
    /// MESH_MODEL_OPCODE_CFM message)
    MESH_API_MODEL_OPCODE_REQ_IND            = MESH_API_MSG_VAL(10),
    /// Used to answer if a model support or not a specific operation code.
    MESH_API_MODEL_OPCODE_CFM                = MESH_API_MSG_VAL(11),
    /// Inform reception of a specific mesh message
    MESH_API_MODEL_MSG_IND                   = MESH_API_MSG_VAL(12),
    /// Inform transmission status of message issued by local model
    MESH_API_MODEL_MSG_SENT_IND              = MESH_API_MSG_VAL(13),
    /// Inform about new publish period for a model
    MESH_API_MODEL_PUBLISH_PERIOD_IND        = MESH_API_MSG_VAL(14),

    /// Inform about updated attention state
    MESH_API_ATTENTION_UPDATE_IND            = MESH_API_MSG_VAL(20),
    /// Inform about requested node reset
    MESH_API_NODE_RESET_IND                  = MESH_API_MSG_VAL(21),

    /// Request a page of the composition data
    MESH_API_COMPO_DATA_REQ_IND              = MESH_API_MSG_VAL(30),
    /// Response containing the required composition data page
    MESH_API_COMPO_DATA_CFM                  = MESH_API_MSG_VAL(31),
    /// Configuration update indication
    MESH_API_UPDATE_IND                      = MESH_API_MSG_VAL(32),

    /// Request to get the current Registered Fault state identified by Company ID for primary element
    MESH_API_FAULT_GET_REQ_IND               = MESH_API_MSG_VAL(40),
    /// Request to start a test procedure of primary element
    MESH_API_FAULT_TEST_REQ_IND              = MESH_API_MSG_VAL(41),
    /// Response containing current Registered Fault state for primary element
    MESH_API_FAULT_CFM                       = MESH_API_MSG_VAL(42),
    /// Inform that clear of Registered Fault state identified by Company ID has been received
    /// for primary element
    MESH_API_FAULT_CLEAR_IND                 = MESH_API_MSG_VAL(43),
    /// Inform about new publication period for Current Health state of primary element
    MESH_API_FAULT_PERIOD_IND                = MESH_API_MSG_VAL(44),

    /// Mesh Provisioning state change indication
    MESH_API_PROV_STATE_IND                  = MESH_API_MSG_VAL(50),
    /// Mesh Provisioning parameters request
    MESH_API_PROV_PARAM_REQ_IND              = MESH_API_MSG_VAL(51),
    /// Mesh Provisioning parameters response
    MESH_API_PROV_PARAM_CFM                  = MESH_API_MSG_VAL(52),
    /// Mesh Provisioning Authentication Data Request
    MESH_API_PROV_AUTH_DATA_REQ_IND          = MESH_API_MSG_VAL(53),
    /// Mesh Provisioning Authentication Data response
    MESH_API_PROV_AUTH_DATA_CFM              = MESH_API_MSG_VAL(54),

    /// Mesh Friend Offer reception indication
    MESH_API_LPN_OFFER_IND                   = MESH_API_MSG_VAL(60),
    /// Mesh Friendship status indication
    MESH_API_LPN_STATUS_IND                  = MESH_API_MSG_VAL(61),

    /// Mesh Proxy advertising state update indication
    MESH_API_PROXY_ADV_UPDATE_IND            = MESH_API_MSG_VAL(70),
    MESH_API_LAST                            = MESH_API_PROXY_ADV_UPDATE_IND,

    /* ******************************** */
    /* *       MESH PROFILE DEBUG     * */
    /* ******************************** */

    /// Mesh Profile Debug Command
    MESH_DBG_FIRST                           = MESH_API_MSG_VAL(90),
    MESH_DBG_CMD                             = MESH_DBG_FIRST,
    /// Mesh Profile Debug Command Complete Event
    MESH_DBG_CMP_EVT                         = MESH_API_MSG_VAL(91),
    /// Mesh Debug Data Received Indication
    MESH_DBG_DATA_RX_IND                     = MESH_API_MSG_VAL(92),
    /// Mesh Debug Data Send Indication
    MESH_DBG_DATA_SEND_IND                   = MESH_API_MSG_VAL(93),
    /// Mesh Debug Data Sent Indication
    MESH_DBG_DATA_SENT_IND                   = MESH_API_MSG_VAL(94),
    /// Mesh Debug Bearer Opened Indication
    MESH_DBG_BEARER_OPENED_IND               = MESH_API_MSG_VAL(95),
    /// Mesh Debug Bearer Close Indication
    MESH_DBG_BEARER_CLOSED_IND               = MESH_API_MSG_VAL(96),
    /// Mesh Debug Friendship Indication
    MESH_DBG_FRIEND_LPN_IND                  = MESH_API_MSG_VAL(97),
    MESH_DBG_LAST                            = MESH_DBG_FRIEND_LPN_IND,

    /* ******************************** */
    /* *         MESH STACK           * */
    /* ******************************** */

    /// Mesh Stack Command
    MESH_COMMON_API_FIRST                    = MESH_API_MSG_VAL(110),
    MESH_COMMON_API_CMD                      = MESH_COMMON_API_FIRST,
    /// Mesh Stack Command Complete Event
    MESH_COMMON_API_CMP_EVT                  = MESH_API_MSG_VAL(111),
    /// Mesh Stack Indication
    MESH_COMMON_API_IND                      = MESH_API_MSG_VAL(112),
    /// Mesh Stack Request Indication
    MESH_COMMON_API_REQ_IND                  = MESH_API_MSG_VAL(113),
    /// Mesh Stack Confirmation
    MESH_COMMON_API_CFM                      = MESH_API_MSG_VAL(114),
    MESH_COMMON_API_LAST                     = MESH_COMMON_API_CFM,

    /* ******************************** */
    /* *       MESH STACK DEBUG       * */
    /* ******************************** */

    /// Mesh Stack Debug Command
    MESH_COMMON_DBG_FIRST                    = MESH_API_MSG_VAL(120),
    MESH_COMMON_DBG_CMD                      = MESH_COMMON_DBG_FIRST,
    /// Mesh Stack Debug Command Complete Event
    MESH_COMMON_DBG_CMP_EVT                  = MESH_API_MSG_VAL(121),
    /// Mesh Stack Debug Indication
    MESH_COMMON_DBG_IND                      = MESH_API_MSG_VAL(122),
    MESH_COMMON_DBG_LAST                     = MESH_COMMON_DBG_IND,

    /* ******************************** */
    /* *           MESH MODEL         * */
    /* ******************************** */

    /// Mesh Model Command
    MESH_MDL_API_FIRST                       = MESH_API_MSG_VAL(130),
    MESH_MDL_API_CMD                         = MESH_MDL_API_FIRST,
    /// Mesh Model Command Complete Event
    MESH_MDL_API_CMP_EVT                     = MESH_API_MSG_VAL(131),
    /// Mesh Model Indication
    MESH_MDL_API_IND                         = MESH_API_MSG_VAL(132),
    /// Mesh Model Request Indication
    MESH_MDL_API_REQ_IND                     = MESH_API_MSG_VAL(133),
    /// Mesh Model Confirm
    MESH_MDL_API_CFM                         = MESH_API_MSG_VAL(134),
    MESH_MDL_API_LAST                        = MESH_MDL_API_CFM,

    /* ******************************** */
    /* *       MESH MODEL DEBUG       * */
    /* ******************************** */

    /// Mesh Stack Debug Command
    MESH_MDL_DBG_FIRST                       = MESH_API_MSG_VAL(140),
    MESH_MDL_DBG_CMD                         = MESH_MDL_DBG_FIRST,
    /// Mesh Stack Debug Command Complete Event
    MESH_MDL_DBG_CMP_EVT                     = MESH_API_MSG_VAL(141),
    /// Mesh Stack Debug Indication
    MESH_MDL_DBG_IND                         = MESH_API_MSG_VAL(142),
    MESH_MDL_DBG_LAST                        = MESH_MDL_DBG_IND,

    /* ******************************** */
    /* *            INTERNAL          * */
    /* ******************************** */

    /// Message ID for first internal message, all following IDs are reserved
    MESH_INT_MSG                             = MESH_API_MSG_VAL(200),
};

/// Command codes for MESH_COMMON_API_CMD and MESH_COMMON_API_CMP_EVT messages
enum mesh_api_cmd_code
{
    /// Get elapsed time since first device up time
    MESH_API_GET_RUN_TIME               = 0,
    /// Set elapsed time since first device up time
    MESH_API_SET_RUN_TIME               = 1,
    /// Get Mesh Stack version
    MESH_API_GET_VERSION                = 2,

    /// Allocate a new block of buffers
    MESH_API_BUF_ALLOC_BLOCK            = 10,
    /// Free a block of buffers
    MESH_API_BUF_FREE_BLOCK             = 11,
};

/*
 * MESSAGE DEFINITIONS FOR MESH STACK
 ****************************************************************************************
 */

/// Command required structure (without parameters)
typedef struct mesh_api_cmd
{
    /// Command code (@see enum mesh_api_cmd_code)
    uint32_t cmd_code;
} mesh_api_cmd_t;

/// Command complete event required structure (without parameters)
typedef struct mesh_api_cmp_evt
{
    /// Command code (@see enum mesh_cmd_code)
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
} mesh_api_cmp_evt_t;

/// Set run time command structure
typedef struct mesh_api_set_run_time_cmd
{
    /// Command code (@see enum mesh_cmd_code)
    /// - MESH_API_SET_RUN_TIME: Set run time
    uint32_t cmd_code;
    /// Current clock value in milliseconds
    uint32_t clock_ms;
    /// Number of time the clock has wrapped since up time
    uint16_t nb_wrap;
} mesh_api_set_run_time_cmd_t;

/// Get run time command complete event structure
typedef struct mesh_api_get_run_time_cmp_evt
{
    /// Command code (@see  enum mesh_api_cmd_code)
    /// - MESH_API_GET_RUN_TIME: Get run time
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;

    /// Number of time the clock has wrapped since up time
    uint16_t nb_wrap;
    /// Current clock value in milliseconds
    uint32_t clock_ms;
} mesh_api_get_run_time_cmp_evt_t;

/// Get version command complete event structure
typedef struct mesh_api_get_version_cmp_evt
{
    /// Command code (@see  enum mesh_api_cmd_code)
    /// - MESH_API_GET_VERSION: Get version
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;

    /// Mesh Versions
    mesh_version_t version;
} mesh_api_get_version_cmp_evt_t;

/// Allocate block of buffers command structure
typedef struct mesh_api_buf_alloc_block_cmd
{
    /// Command code (@see enum mesh_cmd_code)
    /// - MESH_API_BUF_ALLOC_BLOCK: Allocate a new block of buffers
    uint32_t cmd_code;
    /// Number of buffers
    uint8_t nb_bufs;
    /// Small (1) or long buffers (0)
    uint8_t small;
} mesh_api_buf_alloc_block_cmd_t;

/// Allocate new block of buffers command complete event structure
typedef struct mesh_api_buf_alloc_block_cmp_evt
{
    /// Command code (@see enum mesh_cmd_code)
    /// - MESH_API_BUF_ALLOC_BLOCK: Allocate a new block of buffers
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;

    /// Block ID
    uint8_t block_id;
} mesh_api_buf_alloc_block_cmp_evt_t;

/// Free block of buffers command structure
typedef struct mesh_api_buf_free_block_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - MESH_API_BUF_FREE_BLOCK: Free a block of buffers
    uint32_t cmd_code;
    /// Block ID
    uint8_t block_id;
} mesh_api_buf_free_block_cmd_t;

/// Free block of buffers command complete event structure
typedef struct mesh_api_buf_free_block_cmp_evt
{
    /// Command code (@see enum mesh_cmd_code)
    /// - MESH_API_BUF_FREE_BLOCK: Free a block of buffers
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;

    /// Block ID
    uint8_t block_id;
} mesh_api_buf_free_block_cmp_evt_t;

/*
 * ENUMERATIONS FOR MESH PROFILE
 ****************************************************************************************
 */

/// Command codes for MESH_API_CMD and MESH_API_CMP_EVT messages
enum m_api_cmd_code
{
    /// Enable Mesh profile
    M_API_ENABLE                        = 0,
    /// Disable Mesh profile
    M_API_DISABLE                       = 1,
    /// Register a model instance
    M_API_REGISTER_MODEL                = 4,

    /// Model request to publish a new message
    M_API_MODEL_PUBLISH                 = 20,
    /// Model send a response message
    M_API_MODEL_RSP_SEND                = 21,

    /// Publish health current status of primary element
    M_API_HEALTH_STATUS_SEND            = 30,

    /// Read Device public key
    M_API_PROV_PUB_KEY_READ             = 40,

    /// IV Update test mode
    M_API_IV_UPD_TEST_MODE              = 50,

    /// Require to start use of Low Power Node feature
    M_API_LPN_START                     = 60,
    /// Require to stop use of Low Power Node feature
    M_API_LPN_STOP                      = 61,
    /// Require to establish a friendship with a given node
    M_API_LPN_SELECT_FRIEND             = 62,

    /// Control the Proxy Connectable mode
    M_API_PROXY_CTL                     = 70,

    /// Load stored information
    M_API_STORAGE_LOAD                  = 80,
    /// Request to force storage of information
    M_API_STORAGE_SAVE                  = 81,
    /// Configure storage module behavior
    M_API_STORAGE_CONFIG                = 82,
};

/*
 * MESSAGE DEFINITIONS FOR MESH PROFILE
 ****************************************************************************************
 */

/// Command required structure (without parameters)
typedef struct m_api_cmd
{
    /// Command code (@see enum m_cmd_code)
    uint32_t cmd_code;
} m_api_cmd_t;

/// Command complete event required structure (without parameters)
typedef struct m_api_cmp_evt
{
    /// Command code (@see enum m_cmd_code)
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
} m_api_cmp_evt_t;

/// Register a model instance command structure
typedef struct m_api_register_model_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_REGISTER_MODEL: Register a model instance
    uint32_t cmd_code;

    /// Model ID
    uint32_t model_id;
    /// Offset from the primary element unicast address
    uint8_t  addr_offset;
    /// Vendor or SIG Model ID
    uint8_t  vendor;
} m_api_register_model_cmd_t;

/// Register a model instance command complete event structure
typedef struct m_api_register_model_cmp_evt
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_REGISTER_MODEL: Register a model instance
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;

    /// Model LID
    m_lid_t model_lid;
} m_api_register_model_cmp_evt_t;

/// Load stored information command structure
typedef struct m_api_storage_load_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_STORAGE_LOAD: Load stored information
    uint32_t cmd_code;

    /// Length
    uint32_t length;
    /// Stored information
    uint8_t data[__ARRAY_EMPTY];
} m_api_storage_load_cmd_t;

/// Configure storage module command structure
typedef struct m_api_storage_config_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_STORAGE_CONFIG: Configure storage module
    uint32_t cmd_code;

    /// Configuration (meaning depends on compiled storage module)
    uint32_t config;
} m_api_storage_config_cmd_t;

/// Model request to publish a new message
typedef struct m_api_model_publish_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_MODEL_PUBLISH: Model request to publish a new message
    uint32_t cmd_code;
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Handle value used by model to retrieve which message has been sent
    uint8_t  tx_hdl;
    /// 1 = Segmented PDU force transport MIC to 64 bits ; 0 = 32 bits transport MIC
    uint8_t  trans_mic_64;
    /// Operation code of the message
    uint32_t opcode;
    /// Message length
    uint16_t len;
    /// Message content
    uint8_t  msg[__ARRAY_EMPTY];
} m_api_model_publish_cmd_t;

/// Model send a response message
typedef struct m_api_model_rsp_send_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_MODEL_RSP_SEND: Model send a response message
    uint32_t cmd_code;
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Key information.
    /// If key_lid & 0x80 != 0, key_lid & 0x7F = network key local index
    /// else key_lid & 0x7F = application key local index.
    m_lid_t  key_lid;
    /// Handle value used by model to retrieve which message has been sent
    uint8_t  tx_hdl;
    /// 1 = Segmented PDU force transport MIC to 64 bits ; 0 = 32 bits transport MIC
    uint8_t  trans_mic_64;
    /// Operation code of the message
    uint32_t opcode;
    /// Unicast destination address of the message (source address parameter of received request message)
    uint16_t dst;
    /// 1, send message to an immediate peer; 0, accept message to be relayed
    uint8_t  not_relay;
    /// Message length
    uint16_t len;
    /// Message content
    uint8_t  msg[__ARRAY_EMPTY];

} m_api_model_rsp_send_cmd_t;

/// Model send a response message
typedef struct m_api_health_status_send_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_SEND_HEALTH_STATUS: Model send a response message
    uint32_t cmd_code;
    /// Company ID
    uint16_t comp_id;
    /// Test ID
    uint8_t  test_id;
    /// Length of fault array
    uint8_t  length;
    /// Fault array
    uint8_t  fault_array[__ARRAY_EMPTY];
} m_api_health_status_send_cmd_t;

/// Request model if a specific operation code is supported (shall be confirmed with MESH_MODEL_OPCODE_CFM message)
typedef struct m_api_model_opcode_req_ind
{
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Mesh message operation code (can be 1, 2 or 3 octet operation code)
    uint32_t opcode;
} m_api_model_opcode_req_ind_t;

/// Used to answer if a model support or not a specific operation code.
typedef struct m_api_model_opcode_cfm
{
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// MESH_ERR_OR_NO_ERROR if supported, else rejected
    uint16_t status;
    /// Mesh message operation code (can be 1, 2 or 3 octet operation code)
    uint32_t opcode;
} m_api_model_opcode_cfm_t;

/// Inform reception of a specific mesh message
typedef struct m_api_model_msg_ind
{
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Application Key Local identifier (Required for a response)
    m_lid_t  app_key_lid;
    /// Measured RSSI level for the received PDU.
    int8_t   rssi;
    /// 1 = if message have been received by an immediate peer; 0 = it can have been relayed
    uint8_t  not_relayed;
    /// Mesh message operation code (can be 1, 2 or 3 octet operation code)
    uint32_t opcode;
    /// Source address of the message (Required for a response)
    uint16_t src;
    /// Message length
    uint16_t msg_len;
    /// Message content
    uint8_t  msg[__ARRAY_EMPTY];
} m_api_model_msg_ind_t;

/// Inform transmission status of message issued by local model
typedef struct m_api_model_msg_sent_ind
{
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Handle value configured by model when message has been requested to be sent
    uint8_t  tx_hdl;
    /// Transmission status
    uint16_t status;
} m_api_model_msg_sent_ind_t;

/// Inform about new publish period for a model
typedef struct m_api_model_publish_period_ind
{
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Publish period in milliseconds
    uint32_t period_ms;
} m_api_model_publish_period_ind_t;

/// Inform about attention state update
typedef struct m_api_attention_update_ind
{
    /// Attention state
    uint8_t attention_state;
} m_api_attention_update_ind_t;

/// Request a page of the composition data
typedef struct m_api_compo_data_req_ind
{
    /// Page
    uint8_t page;
} m_api_compo_data_req_ind_t;

/// Response containing the required composition data page
typedef struct m_api_compo_data_cfm
{
    /// Page
    uint8_t page;
    /// Length
    uint8_t length;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} m_api_compo_data_cfm_t;

/// Configuration update indication message structure
typedef struct m_api_update_ind
{
    /// Update type
    uint8_t upd_type;
    /// Entry length
    uint8_t length;
    /// Entry value
    uint8_t data[__ARRAY_EMPTY];
} m_api_update_ind_t;

/// Request to get the current Registered Fault state identified by Company ID for primary element
typedef struct m_api_fault_get_req_ind
{
    /// Company ID
    uint16_t comp_id;
} m_api_fault_get_req_ind_t;

/// Request to start a test procedure of primary element
typedef struct m_api_fault_test_req_ind
{
    /// Company ID
    uint16_t comp_id;
    /// Test ID
    uint8_t  test_id;
    /// Indicate if MESH_FAULT_CFM message is expected
    bool     cfm_needed;
} m_api_fault_test_req_ind_t;

/// Response containing current Registered Fault state for primary element
typedef struct m_api_fault_cfm
{
    /// Accept (true) or reject (false) the request
    bool     accept;
    /// Company ID
    uint16_t comp_id;
    /// Test ID
    uint8_t  test_id;
    /// Length of fault array
    uint8_t  length;
    /// Fault array
    uint8_t  fault_array[__ARRAY_EMPTY];
} m_api_fault_cfm_t;

/// Inform that clear of Registered Fault state identified by Company ID has been received for primary element
typedef struct m_api_fault_clear_ind
{
    /// Company ID
    uint16_t comp_id;
} m_api_fault_clear_ind_t;

/// Inform about new publication period for Current Health state of primary element
typedef struct m_api_fault_period_ind
{
    /// Publication period in milliseconds when no fault is known
    uint32_t period_ms;
    /// Publication period in milliseconds when one or several fault are known
    uint32_t period_fault_ms;
} m_api_fault_period_ind_t;

/// Mesh Provisioning state change indication
typedef struct m_api_prov_state_ind
{
    /// Provisioning procedure state (@see enum m_prov_state)
    uint8_t state;
    /// Relevant only for provisioning failed (failed reason)
    uint16_t status;
} m_api_prov_state_ind_t;

/// Mesh Provisioning Parameters Response
typedef struct m_api_prov_param_cfm
{
    /// Device UUID
    uint8_t  dev_uuid[MESH_DEV_UUID_LEN];
    /// URI hash
    uint32_t uri_hash;
    /// OOB information
    uint16_t oob_info;
    /// Public key OOB information available
    uint8_t  pub_key_oob;
    /// Static OOB information available
    uint8_t  static_oob;
    /// Maximum size of Output OOB supported
    uint8_t  out_oob_size;
    /// Maximum size in octets of Input OOB supported
    uint8_t  in_oob_size;
    /// Supported Output OOB Actions (@see enum m_prov_out_oob)
    uint16_t out_oob_action;
    /// Supported Input OOB Actions (@see enum m_prov_in_oob)
    uint16_t in_oob_action;
    /// Number of elements
    uint8_t  nb_elt;
    /// Bit field providing additional information (@see enum m_prov_info)
    uint8_t  info;
} m_api_prov_param_cfm_t;

/// Mesh Provisioning Authentication Data Request
typedef struct m_api_prov_auth_data_req_ind
{
    /// Authentication method (@see enum m_prov_auth_method)
    uint8_t  auth_method;
    /// expected authentication maximum data size
    uint8_t  auth_size;
    /// Authentication Action:
    ///   - M_PROV_AUTH_NO_OOB     = Prohibited
    ///   - M_PROV_AUTH_STATIC_OOB = 16 bytes LSB static out of band data required
    ///   - M_PROV_AUTH_OUTPUT_OOB = @see enum m_prov_out_oob, 1 bit set.
    ///   - M_PROV_AUTH_INPUT_OOB  = @see enum m_prov_in_oob, 1 bit set.
    uint16_t auth_action;
} m_api_prov_auth_data_req_ind_t;

/// Mesh Provisioning Authentication Data Response
typedef struct m_api_prov_auth_data_cfm
{
    /// 1, Accept pairing request, 0 reject
    uint8_t  accept;
    /// Authentication data size (<= requested size else pairing automatically rejected)
    uint8_t  auth_size;
    /// Authentication data (LSB for a number or array of bytes)
    uint8_t  auth_data[__ARRAY_EMPTY];
} m_api_prov_auth_data_cfm_t;

/// Read Device public key command
typedef struct m_api_prov_pub_key_read_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_PROV_PUB_KEY_READ: Read Device public key
    uint32_t cmd_code;
} m_api_prov_pub_key_read_cmd_t;

/// Read Device public key Command complete event
typedef struct m_api_prov_pub_key_read_cmp_evt
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_PROV_PUB_KEY_READ: Read Device public key
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
    /// X Coordinate of public Key (32 bytes LSB)
    uint8_t  pub_key_x[MESH_PUB_KEY_X_LEN];
    /// Y Coordinate of public Key (32 bytes LSB)
    uint8_t  pub_key_y[MESH_PUB_KEY_Y_LEN];
} m_api_prov_pub_key_read_cmp_evt_t;

/// IV Update test mode command
typedef struct m_api_iv_upd_test_mode_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_IV_UPD_TEST_MODE: Configure IV Update test mode
    uint32_t cmd_code;
    /// Transit to IV Update in Progress operation (true) or to Normal operation (false)
    bool     update;
} m_api_iv_upd_test_mode_cmd_t;

/// Start Low Power Node feature command structure
typedef struct m_api_lpn_start_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_LPN_START: Start Low Power Node feature
    uint32_t cmd_code;

    /// Initial value of PollTimeout timer (in multiple of 100ms)
    uint32_t poll_timeout;
    /// Poll interval (in milliseconds)
    uint32_t poll_intv_ms;
    /// Unicast address of the primary element of the previous friend
    uint16_t prev_addr;
    /// Receive delay
    uint8_t  rx_delay;
    /// RSSI factor
    uint8_t  rssi_factor;
    /// Receive window factor
    uint8_t  rx_window_factor;
    /// Minimum queue size (log value)
    uint8_t  min_queue_size_log;
} m_api_lpn_start_cmd_t;

/// Select Friend Node command structure
typedef struct m_api_lpn_select_friend_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_LPN_SELECT_FRIEND: Select Friend node
    uint32_t cmd_code;

    /// Friend node address
    uint16_t friend_addr;
} m_api_lpn_select_friend_cmd_t;

/// Low Power Node offer reception indication structure
typedef struct m_api_lpn_offer_ind
{
    /// Address of Friend node that sent the Friend Offer message
    uint16_t addr;
    /// Receive window value supported by the Friend node
    uint8_t  rx_window;
    /// Queue size available on the Friend node
    uint8_t  queue_size;
    /// Size of the subscription list that can be supported by the Friend Node
    uint8_t  subs_list_size;
    /// RSSI measured by the Friend node
    int8_t   rssi;
} m_api_lpn_offer_ind_t;

/// Low Power Node status indication structure
typedef struct m_api_lpn_status_ind
{
    /// Status
    uint16_t status;
    /// Friend address
    uint16_t friend_addr;
} m_api_lpn_status_ind_t;

/// Control the Proxy Connectable mode
typedef struct m_api_proxy_ctl_cmd
{
    /// Command code (@see enum m_cmd_code)
    /// - M_API_PROXY_CTL: Control proxy
    uint32_t cmd_code;

    /// Required update (@see enum mesh_proxy_adv_ctl)
    uint8_t enable;
} m_api_proxy_ctl_cmd_t;

/// Indicate that proxy advertising has been started or stopped and the reason
typedef struct m_api_proxy_adv_update_ind
{
    /// State (@see enum mesh_proxy_adv_upd)
    uint8_t state;
    /// Reason (@see enum mesh_proxy_adv_upd_reason)
    uint8_t reason;
} m_api_proxy_adv_update_ind_t;

/*
 * MESSAGE DEFINITIONS FOR MESH MODELS
 ****************************************************************************************
 */

/// Minimal content for MESH_MDL_API_CMD message
typedef struct mm_api_cmd
{
    /// Model API Identifier (@see enum mm_mdl_apid)
    uint16_t mdl_apid;
    /// Command Code (depends on indicated API)
    uint16_t cmd_code;
} mm_api_cmd_t;

/// Minimal content for MESH_MDL_API_CMP_EVT message
typedef struct mm_api_cmp_evt
{
    /// Model API Identifier (@see enum mm_mdl_apid)
    uint16_t mdl_apid;
    /// Command Code (depends on indicated API)
    uint16_t cmd_code;
    /// Status of command execution
    uint16_t status;
} mm_api_cmp_evt_t;

/// Minimal content for MESH_MDL_API_IND message
typedef struct mm_api_ind
{
    /// Model API Identifier (@see enum mm_mdl_apid)
    uint16_t mdl_apid;
    /// Indication code (depends on indicated API)
    uint16_t ind_code;
} mm_api_ind_t;

/// Minimal content for MESH_MDL_API_REQ_IND message
typedef struct mm_api_req_ind
{
    /// Model API Identifier (@see enum mm_mdl_apid)
    uint16_t mdl_apid;
    /// Request indication code (depends on indicated API)
    uint16_t req_ind_code;
} mm_api_req_ind_t;

/// Minimal content for MESH_MDL_API_CFM message
typedef struct mm_api_cfm
{
    /// Model API Identifier (@see enum mm_mdl_apid)
    uint16_t mdl_apid;
    /// Request indication code (depends on indicated API)
    uint16_t req_ind_code;
    /// Status of command execution by application
    uint16_t status;
} mm_api_cfm_t;

/// Minimal content of command used to register use of a local model
typedef struct mm_api_register
{
    /// Model API Identifier
    uint16_t mdl_apid;
    /// Index of element the model belongs to (zero based)
    uint8_t elmt_id;
} mm_api_register_t;

/// @} MESH_API_MSG_

#endif /* MESH_API_MSG_ */
