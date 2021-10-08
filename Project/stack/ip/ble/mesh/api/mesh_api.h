/**
 ****************************************************************************************
 *
 * @file mesh_api.h
 *
 * @brief Header file for Mesh Stack Application Programming Interface
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef MESH_API_
#define MESH_API_

/**
 ****************************************************************************************
 * @defgroup MESH_API_MSG_ Mesh Stack Application Programming Interface
 * @ingroup MESH
 * @brief  Mesh Stack Application Programming Interface
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mesh_defines.h"
#include "mesh_config.h"

/*
 * ENUMERATIONS FOR MESH PROFILE
 ****************************************************************************************
 */

/// Mesh Supported Features
enum m_api_feat
{
    /// Relay Node
    M_FEAT_RELAY_NODE_SUP       = (1 << 0),
    /// Proxy Node
    M_FEAT_PROXY_NODE_SUP       = (1 << 1),
    /// Friend Node
    M_FEAT_FRIEND_NODE_SUP      = (1 << 2),
    /// Low Power Node
    M_FEAT_LOW_POWER_NODE_SUP   = (1 << 3),

    /// Message API supported
    M_FEAT_MSG_API_SUP          = (1 << 16),
    /// Provisioning over GATT
    M_FEAT_PB_GATT_SUP          = (1 << 17),
    /// Dynamic beacon interval supported
    M_FEAT_DYN_BCN_INTV_SUP     = (1 << 18),
};

/*
 * TYPES DEFINITION FOR MESH PROFILE
 ****************************************************************************************
 */

/// Mesh Profile Configuration Structure
typedef struct m_cfg
{
    /// Mask of supported features (@see enum m_api_feat)
    uint32_t features;
    /// 16-bit company identifier assigned by the Bluetooth SIG
    uint16_t cid;
    /// 16-bit vendor-assigned product identifier
    uint16_t pid;
    /// 16-bit vendor-assigned product version identifier
    uint16_t vid;
    /// Localization descriptor
    uint16_t loc;
    /// Number of pages in the Composition Data
    uint8_t  nb_cdata_page;

    /// Receive window in milliseconds when Friend feature is supported
    uint8_t frd_rx_window_ms;
    /// Queue size when Friend feature is supported
    uint8_t frd_queue_size;
    /// Subscription list size when Friend feature is supported
    uint8_t frd_subs_list_size;
} m_cfg_t ;

/**
 ****************************************************************************************
 * @brief Mesh Buffer, the value of the pointer must not be changed.
 *
 * It must be:
 * - Allocated through m_api_buf_alloc()
 * - Released with m_api_buf_release()
 ****************************************************************************************
 */
typedef void m_api_buf_t;

/*
 * CALLBACKS DEFINITION FOR MESH PROFILE
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Definition of callback function to call upon reception of a PDU for specific model identifier
 *
 * @param[in] model_lid    Model Local Identifier
 * @param[in] opcode       Operation code
 * @param[in] p_buf        Pointer to the buffer containing the message PDU. - No need to release buffer.
 * @param[in] app_key_lid  Application Key Local identifier (Required for a response)
 * @param[in] src          Source address of the message (Required for a response)
 * @param[in] rssi         Measured RSSI level for the received PDU.
 * @param[in] not_relayed  True if message have been received by an immediate peer; False, it can have been relayed
 ****************************************************************************************
 */
typedef void (*m_api_model_rx_cb)(m_lid_t model_lid, uint32_t opcode, m_api_buf_t* p_buf, m_lid_t app_key_lid,
                                  uint16_t src, int8_t rssi, bool not_relayed);

/**
 ****************************************************************************************
 * @brief Definition of callback function to call upon reception of a PDU to check that model can handle it
 *
 * @note m_api_model_opcode_status function must be used to provide information about opcode support
 *
 * @param[in] model_lid Model Local Identifier
 * @param[in] opcode    Operation code to check
 ****************************************************************************************
 */
typedef void (*m_api_model_opcode_check_cb)(m_lid_t model_lid, uint32_t opcode);

/**
 ****************************************************************************************
 * @brief Definition of callback function to call upon reception of a new publish period value.
 *
 * @param[in] model_lid    Model Local Identifier
 * @param[in] period_ms    Publish period in milliseconds
 ****************************************************************************************
 */
typedef void (*m_api_model_publish_period_cb)(m_lid_t model_lid, uint32_t period_ms);

/**
 ****************************************************************************************
 * @brief Definition of callback function to call once PDU has been sent.
 *
 * @param[in] model_lid Model Local Identifier
 * @param[in] tx_hdl    Handle value configured by model when message has been requested to be sent
 * @param[in] p_buf     Pointer to the buffer containing the transmitted PDU. - Buffer must be released by model.
 * @param[in] status    Transmission status.
 ****************************************************************************************
 */
typedef void (*m_api_model_sent_cb)(m_lid_t model_lid, uint8_t tx_hdl, m_api_buf_t* p_buf, uint16_t status);

/**
 ****************************************************************************************
 * @brief Callback executed when simple execution performed
 *
 * @param[in] status    Execution status.
 ****************************************************************************************
 */
typedef void (*m_api_end_cb)(uint16_t status);

/**
 ****************************************************************************************
 * Callback used to inform about a modification of the provisioning module state
 *
 * @param state    State of the provisioner   (@see enum m_prov_state)
 * @param status   Relevant only for provisioning failed (failed reason)
 ****************************************************************************************
 */
typedef void (*m_api_prov_state_cb)(uint8_t state, uint16_t status);

/**
 ****************************************************************************************
 * Callback used to inform that provisioning parameters are required
 ****************************************************************************************
 */
typedef void (*m_api_prov_param_req_cb)(void);

/**
 ****************************************************************************************
 * Callback used to inform that Out Of Band Authentication Data is required for provisioning
 *
 * @note Authentication data must be provided using @see m_api_prov_oob_auth_rsp() function
 *
 *
 * @param auth_method  Authentication method (@see enum m_prov_auth_method)
 * @param auth_action  Authentication Action:
 *                     - M_PROV_AUTH_NO_OOB     = Prohibited
 *                     - M_PROV_AUTH_STATIC_OOB = 16 bytes LSB static out of band data required
 *                     - M_PROV_AUTH_OUTPUT_OOB = @see enum m_prov_out_oob, 1 bit set.
 *                     - M_PROV_AUTH_INPUT_OOB  = @see enum m_prov_in_oob, 1 bit set.
 * @param auth_size    expected authentication maximum data size
 ****************************************************************************************
 */
typedef void (*m_api_prov_oob_auth_req_cb)(uint8_t auth_method, uint16_t auth_action, uint8_t auth_size);

/**
 ****************************************************************************************
 * Callback used to inform that stored information have been loaded
 ****************************************************************************************
 */
typedef void (*m_api_storage_loaded_cb)(uint16_t status);

/**
 ****************************************************************************************
 * Callback used to inform application about start/end of attention timer
 ****************************************************************************************
 */
typedef void (*m_api_attention_cb)(uint8_t attention_state);

/**
 ****************************************************************************************
 * Callback used to request a page of composition data to the application
 ****************************************************************************************
 */
typedef void (*m_api_compo_data_cb)(uint8_t page);

/**
 ****************************************************************************************
 * Callback used to inform application about received node reset request
 ****************************************************************************************
 */
typedef void (*m_api_node_reset_cb)(void);

/**
 ****************************************************************************************
 * Callback used to inform application of friendship update as low power node
 ****************************************************************************************
 */
typedef void (*m_api_lpn_status_cb)(uint16_t status, uint16_t friend_addr);

/**
 ****************************************************************************************
 * Callback used to inform application about reception of a Friend Offer message
 ****************************************************************************************
 */
typedef void (*m_api_lpn_offer_cb)(uint16_t friend_addr, uint8_t rx_window,
                                   uint8_t queue_size, uint8_t subs_list_size, int8_t rssi);

/**
 ****************************************************************************************
 * Callback used to inform application about reception of a Friend Offer message
 ****************************************************************************************
 */
typedef void (*m_api_proxy_adv_update_cb)(uint8_t state, uint8_t reason);

/**
 ****************************************************************************************
 * Callback used to request list of fault for primary element
 ****************************************************************************************
 */
typedef void (*m_api_fault_get_cb)(uint16_t comp_id);

/**
 ****************************************************************************************
 * Callback used to request test of faults for primary element
 ****************************************************************************************
 */
typedef void (*m_api_fault_test_cb)(uint16_t comp_id, uint8_t test_id, bool cfm_needed);

/**
 ****************************************************************************************
 * Callback used to inform application that fault status for primary element must be cleared
 ****************************************************************************************
 */
typedef void (*m_api_fault_clear_cb)(uint16_t comp_id);

/**
 ****************************************************************************************
 * Callback used to inform application that fault period for primary element has been updated
 ****************************************************************************************
 */
typedef void (*m_api_fault_period_cb)(uint32_t period_ms, uint32_t period_fault_ms);

/*
 * CALLBACK STRUCTURES FOR MESH PROFILE
 ****************************************************************************************
 */

/// Mesh Profile Callback Structure
typedef struct m_api_cb
{
    /// Callback executed at end of mesh enable request
    m_api_end_cb                cb_enabled;
    /// Callback executed at end of mesh disable request
    m_api_end_cb                cb_disabled;
    /// Callback used to inform application about provisioning state
    m_api_prov_state_cb         cb_prov_state;
    /// Callback used to request provisioning parameters to the application
    m_api_prov_param_req_cb     cb_prov_param_req;
    /// Callback used to request out of band authentication data
    m_api_prov_oob_auth_req_cb  cb_prov_auth_req;
    /// Callback used to inform that stored information have been loaded
    m_api_storage_loaded_cb     cb_loaded;
    /// Callback used to inform application about start/end of attention timer
    m_api_attention_cb          cb_attention;
    /// Callback used to request a page of composition data to the application
    m_api_compo_data_cb         cb_compo_data;
    /// Callback used to inform application about received node reset request
    m_api_node_reset_cb         cb_node_reset;
    #if (BLE_MESH_LPN)
    /// Callback used to inform application of friendship update as low power node
    m_api_lpn_status_cb         cb_lpn_status;
    /// Callback used to inform application about reception of a Friend Offer message
    m_api_lpn_offer_cb          cb_lpn_offer;
    #endif //(BLE_MESH_LPN)
    #if (BLE_MESH_GATT_PROXY)
    /// Callback used to inform application about state update of proxy advertising
    m_api_proxy_adv_update_cb   cb_proxy_adv_update;
    #endif //(BLE_MESH_GATT_PROXY)
} m_api_cb_t;

/// Callback Structure for Health Model for primary element
typedef struct m_api_fault_cb
{
    /// Callback used to request list of fault for primary element
    m_api_fault_get_cb          cb_fault_get;
    /// Callback used to request test of faults for primary element
    m_api_fault_test_cb         cb_fault_test;
    /// Callback used to inform application that fault status for primary element must be cleared
    m_api_fault_clear_cb        cb_fault_clear;
    /// Callback used to inform application that fault period for primary element has been updated
    m_api_fault_period_cb       cb_fault_period;
} m_api_fault_cb_t;

/// Callback Structure for registered models
typedef struct m_api_model_cb
{
    /// Reception of a buffer for model
    m_api_model_rx_cb             cb_rx;
    /// Callback executed when a PDU is properly sent
    m_api_model_sent_cb           cb_sent;
    /// Check if model can handle operation code
    m_api_model_opcode_check_cb   cb_opcode_check;
    /// Callback function called when a new publish period is received
    m_api_model_publish_period_cb cb_publish_period;
} m_api_model_cb_t;

/*
 * FUNCTIONS DEFINITION FOR MESH PROFILE
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Define the set of callback to communicate with mesh native API
 *
 * @param[in] p_cb_api Native application callback set use to communicate with a native API
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t m_api_set(const m_api_cb_t *p_cb_api, const m_api_fault_cb_t *p_fault_cb_api);

/**
 ****************************************************************************************
 * @brief Enable Mesh profile
 *
 * @note cb_enabled() of m_api_cb_t called at end of enable execution
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t m_api_enable(void);

/**
 ****************************************************************************************
 * @brief Disable Mesh profile
 *
 * @note cb_disabled() of m_api_cb_t called at end of disable execution
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t m_api_disable(void);

/**
 ****************************************************************************************
 * @brief Allocate buffers command
 *
 * @param[out] pp_buf    Pointer to the buffer structure allocated
 * @param[in]  size      size of data required for the buffer
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t m_api_buf_alloc(m_api_buf_t** pp_buf, uint16_t size);

/**
 ****************************************************************************************
 * @brief Release allocate buffers. The buffer is free as soon as all reference to buffer are released.
 *
 * @param[in] p_buf    Pointer to the buffer structure
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t m_api_buf_release(m_api_buf_t* p_buf);

/**
 ****************************************************************************************
 * @brief Register a model
 *
 * @param[in] model_id          Model ID.
 * @param[in] addr_offset       Offset from primary element address.
 * @param[in] vendor            Vendor Model ID or SIG Model ID.
 * @param[in] p_cb              Pointer to callback functions defined by the model
 * @param[out] p_model_lid      Pointer to the variable that will contain the allocated Model LID.
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_register_model(uint32_t model_id, uint8_t addr_offset, bool vendor,
                              const m_api_model_cb_t *p_cb, m_lid_t *p_model_lid);

/**
 ****************************************************************************************
 * @brief Load stored information for storage manager
 *
 * @param[in] length    Length of stored information report
 * @param[in] p_data    Pointer to stored information report
 ****************************************************************************************
 */
uint16_t m_api_storage_load(uint16_t length, uint8_t *p_data);

/**
 ****************************************************************************************
 * @brief Store information about mesh network
 ****************************************************************************************
 */
uint16_t m_api_storage_save(void);

/**
 ****************************************************************************************
 * @brief Configure storage module
 ****************************************************************************************
 */
uint16_t m_api_storage_config(uint32_t config);

/**
 ****************************************************************************************
 * @brief Set IV update mode and ignore 96-hour limit
 *
 * @param[in] update    True if transition to IV Update in Progress state is required, False if
 * require to transit to Normal state
 ****************************************************************************************
 */
void m_api_iv_upd_test_mode(bool update);

/**
 ****************************************************************************************
 * @brief Let the model publish a message over mesh network
 *
 * @note Message status will be reported with model callback (@see m_api_model_sent_cb)
 *
 * @param[in] model_id     Model ID.
 * @param[in] opcode       Operation code of the message
 * @param[in] tx_hdl       Handle value used by model to retrieve which message has been sent
 * @param[in] p_buf        Pointer to the buffer structure that contains message to publish
 * @param[in] trans_mic_64 1 = Segmented PDU force transport MIC to 64 bits ; 0 = 32 bits transport MIC
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_model_publish(m_lid_t model_lid, uint32_t opcode, uint8_t tx_hdl, m_api_buf_t *p_buf, bool trans_mic_64);

/**
 ****************************************************************************************
 * @brief Let the model publish a message over mesh network
 *
 * @note Message status will be reported with model callback (@see m_api_model_sent_cb)
 *
 * @param[in] model_id     Model ID.
 * @param[in] opcode       Operation code of the message
 * @param[in] tx_hdl       Handle value used by model to retrieve which message has been sent
 * @param[in] p_buf        Pointer to the buffer structure that contains message to publish
 * @param[in] key_lid      Key information.
 *                         If key_lid & 0x80 != 0, key_lid & 0x7F = network key local index
 *                         else key_lid & 0x7F = application key local index.
 * @param[in] dst          Unicast destination address of the message (source address parameter of received request message)
 * @param[in] trans_mic_64 For a segmented PDU force transport mic to 64 bits
 * @param[in] not_relay    True, send message to an immediate peer; False, accept message to be relayed
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_model_rsp_send(m_lid_t model_lid, uint32_t opcode, uint8_t tx_hdl, m_api_buf_t *p_buf,
                              m_lid_t key_lid, uint16_t dst, bool trans_mic_64, bool not_relay);

/**
 ****************************************************************************************
 * @brief Reply to the Model operation code support (@see m_api_model_opcode_check_cb)
 *
 * @param[in] model_id  Model ID.
 * @param[in] opcode    Operation code checked
 * @param[in] status    MESH_ERR_NO_ERROR if operation supported by model, other error code to reject
 *
 * @return Execution status code
 ****************************************************************************************
 */
void m_api_model_opcode_status(m_lid_t model_lid, uint32_t opcode, uint16_t status);

/**
 ****************************************************************************************
 * @brief Provide composition data
 *
 * @param[in] page      Page of composition data
 * @param[in] length    Page length
 * @param[in] p_data    Pointer to page content
 ****************************************************************************************
 */
void m_api_compo_data_cfm(uint8_t page, uint8_t length, uint8_t *p_data);

/**
 ****************************************************************************************
 * @brief Provide fault status for primary element
 *
 * @param[in] comp_id           Company ID
 * @param[in] test_id           Test ID
 * @param[in] length            Length of fault array
 * @param[in] p_fault_array     Pointer to the fault array
 ****************************************************************************************
 */
void m_api_health_status_send(uint16_t comp_id, uint8_t test_id, uint8_t length,
                              uint8_t *p_fault_array);

/**
 ****************************************************************************************
 * @brief Provide fault status for primary element
 *
 * @param[in] comp_id           Company ID
 * @param[in] test_id           Test ID
 * @param[in] length            Length of fault array
 * @param[in] p_fault_array     Pointer to the fault array
 ****************************************************************************************
 */
void m_api_health_cfm(bool accept, uint16_t comp_id, uint8_t test_id, uint8_t length,
                      uint8_t *p_fault_array);

/**
 ****************************************************************************************
 * @brief Provide provisioning parameters to the provisioning module
 *
 * @param[in] p_param      Provisioning parameters
 ****************************************************************************************
 */
void m_api_prov_param_rsp(void *p_param);

/**
 ****************************************************************************************
 * @brief Provide authentication data to the provisioning module
 *
 * @param[in] accept      True, Accept pairing request, False reject
 * @param[in] auth_size   Authentication data size (<= requested size else pairing automatically rejected)
 * @param[in] p_auth_data Authentication data (LSB for a number or array of bytes)
 ****************************************************************************************
 */
void m_api_prov_oob_auth_rsp(bool accept, uint8_t auth_size, const uint8_t* p_auth_data);

/**
 ****************************************************************************************
 * @brief Get the local public key for out of band transmission of local public key
 *
 * @param[out] p_pub_key_x   X Coordinate of public Key (32 bytes LSB)
 * @param[out] p_pub_key_y   Y Coordinate of public Key (32 bytes LSB)
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_prov_pub_key_read(uint8_t* p_pub_key_x, uint8_t* p_pub_key_y);

#if (BLE_MESH_LPN)
/**
 ****************************************************************************************
 * @brief Enable Low Power Node feature and start looking for an available Friend node in
 * the neighborhood.
 *
 * @param[in] poll_timeout          Initial value of PollTimeout timer
 * @param[in] poll_intv_ms          Poll interval in milliseconds
 * @param[in] rx_delay              Requested receive delay
 * @param[in] rssi_factor           RSSI factor
 * @param[in] rx_window_factor      Receive window factor
 * @param[in] min_queue_size_log    Requested minimum number of messages that the Friend node
 * can store in its Friend Queue.
 ****************************************************************************************
 */
uint16_t m_api_lpn_start(uint32_t poll_timeout, uint32_t poll_intv_ms, uint16_t prev_addr, uint8_t rx_delay,
                         uint8_t rssi_factor, uint8_t rx_window_factor, uint8_t min_queue_size_log);

/**
 ****************************************************************************************
 * @brief Disable Low Power Node feature.
 ****************************************************************************************
 */
uint16_t m_api_lpn_stop(void);

/**
 ****************************************************************************************
 * @brief Select a friend after reception of one or several Friend Offer messages.
 *
 * @param[in] friend_addr       Address of the selected Friend node.
 ****************************************************************************************
 */
uint16_t m_api_lpn_select_friend(uint16_t friend_addr);
#endif //(BLE_MESH_LPN)

#if (BLE_MESH_GATT_PROXY)
/**
 ****************************************************************************************
 * @brief Control if Proxy service should start / stop advertising it's capabilities
 *
 * @param[in] enable  True to enable advertising for 60s, False to stop advertising
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_proxy_ctrl(uint8_t enable);
#endif // (BLE_MESH_GATT_PROXY)

/*
 * ENUMERATIONS FOR MESH MODELS
 ****************************************************************************************
 */

/// Model API Identifiers
enum mm_mdl_apid
{
    /// Common API
    MM_APID_COMMON = 0,

    /// Generic Server Models API
    MM_APID_GENS,
    /// Generic Client Models API
    MM_APID_GENC,
    /// Lighting Server Models API
    MM_APID_LIGHTS,
    /// Lighting Client Models API
    MM_APID_LIGHTC,
    /// Time and Scene Server Models API
    MM_APID_TSCNS,
    /// Time and Scene Client Models API
    MM_APID_TSCNC,
    /// Sensor Server Models API
    MM_APID_SENSS,
    /// Sensor Client Models API
    MM_APID_SENSC,

    /// Number of known identifiers
    MM_APID_NB,
};

/// Command Codes for Common API
enum mm_mdl_ccode_common
{
    /// Register use of a local model
    MM_CCODE_COMMON_REGISTER = 0,

    /// Number of known command codes
    MM_CCODE_COMMON_NB,
};

/*
 * TYPE DEFINITIONS FOR MESH MODELS
 ****************************************************************************************
 */

/// Mesh Model Configuration Structure
typedef struct mm_cfg
{
    /// Number of models that will be handled by the Mesh Model Module
    uint8_t nb_models;
} mm_cfg_t;

/*
 * FUNCTION DECLARATIONS FOR MESH MODELS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Register use of a model
 *
 * @param[in] mdl_id       Model API Identifier
 * @param[in] elmt_id      Element Identifier
 *
 * @return Handling status
 ****************************************************************************************
 */
uint16_t mm_api_register(uint16_t mdl_apid, uint8_t elmt_id);

/*
 * TYPE DEFINITIONS FOR MESH STACK
 ****************************************************************************************
 */

/// Mesh Stack Configuration Structure
typedef struct mesh_cfg
{
    /// Mesh Profile Configuration
    m_cfg_t prf_cfg;
    /// Mesh Model Configuration
    mm_cfg_t model_cfg;
} mesh_cfg_t;

/// Mesh Stack Version Structure
typedef struct mesh_version
{
    /// Mesh Specification version (X.Y.Z)
    uint8_t mesh_version_x;
    uint8_t mesh_version_y;
    uint8_t mesh_version_z;

    /// Mesh Software version (X.Y)
    uint8_t mesh_sw_version_x;
    uint8_t mesh_sw_version_y;
} mesh_version_t;

/*
 * CALLBACK DEFINITIONS FOR MESH STACK
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Callback executed when a buffer block has been properly released.
 *
 * @param[in] block_id  Buffer Block identifier
 ****************************************************************************************
 */
typedef void (*mesh_api_buf_block_released_cb)(uint8_t block_id);

/*
 * CALLBACK STRUCTURES FOR MESH STACK
 ****************************************************************************************
 */

/// Callbacks structure for Mesh Stack
typedef struct mesh_api_cb
{
    /// Callback used to inform that a buffer block has been properly released
    mesh_api_buf_block_released_cb cb_buf_block_freed;
} mesh_api_cb_t;

/*
 * FUNCTION DECLARATIONS FOR MESH STACK
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief TODO [LT]
 ****************************************************************************************
 */
uint8_t mesh_handler(uint16_t msg_id, uint16_t src_id, const void *p_param);

/**
 ****************************************************************************************
 * @brief Define the set of callback to communicate with mesh native API
 *
 * @param[in] p_cb_api Native application callback set use to communicate with a native API
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t mesh_api_set(const mesh_api_cb_t *p_cb_api);

/**
 ****************************************************************************************
 * @brief Get device run time
 *
 * @param[out] p_clock_ms       Pointer to variable that will contain the current clock in milliseconds.
 * @param[out] p_nb_wrap        Pointer to variable that will contain the number of wrap.
 ****************************************************************************************
 */
void mesh_api_get_run_time(uint32_t *p_clock_ms, uint16_t *p_nb_wrap);

/**
 ****************************************************************************************
 * @brief Set device run time
 *
 * @param[out] clock_ms       Current clock in milliseconds.
 * @param[out] nb_wrap        Number of wraps.
 ****************************************************************************************
 */
uint16_t mesh_api_set_run_time(uint32_t clock_ms, uint16_t nb_wrap);

/**
 ****************************************************************************************
 * @brief Get Mesh Stack version
 *
 * @param[out] p_version       Pointer to structure in which version information will be provided
 ****************************************************************************************
 */
void mesh_api_get_version(mesh_version_t *p_version);

/**
 ****************************************************************************************
 * @brief Allocate block of buffers
 *
 * @param[out] p_block_id  Pointer to the variable that will contain the index of the allocated block.
 * @param[in]  nb_bufs     Number of buffers contained in the block.
 * @param[in]  small       Indicate if block contains small or long buffers.
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t mesh_api_buf_alloc_block(uint8_t *p_block_id, uint8_t nb_bufs, bool small);

/**
 ****************************************************************************************
 * @brief Free block of buffers
 *
 * @note cb_release() of m_api_cb_t called at end of disable execution
 *
 * @param[in] block_id   Index of the allocated block.
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t mesh_api_buf_free_block(uint8_t block_id);

/// @} MESH_API_

#endif /* MESH_API_ */
