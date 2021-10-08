/**
 ****************************************************************************************
 *
 * @file app_sec.h
 * @brief about Security setting.
 * @date Mon, Jan  7, 2019  4:31:27 PM
 * @author chenzhiyuan
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_COMMON_SEC app_sec.h
 * @ingroup APP_COMMON
 * @brief about Security setting.
 * @{
 ****************************************************************************************
 */

#ifndef APP_SEC_H_
#define APP_SEC_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if 1 //(BLE_APP_SEC)

#include <stdint.h>          // Standard Integer Definition

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * STRUCTURES DEFINITIONS
 ****************************************************************************************
 */

/// connection information
struct app_sec_conn_info_tag
{
    /// id addr
    struct gap_bdaddr  id_addr;
	/// ltk
    struct gapc_ltk    ltk; 
	/// irk
    struct gapc_irk    irk;  
	/// csrk
    struct gap_sec_key csrk;    
};

/// Secure env information
struct app_sec_env_tag
{
    ///Bond status
    bool bonded;
#if (BLE_APP_SEC_CON)
    ///Secure Connections on current link
    bool sec_con_enabled[BLE_CONNECTION_MAX];
#endif
	///connect bonded state
    bool con_bonded[BLE_CONNECTION_MAX];
	///connection information
    struct app_sec_conn_info_tag info[BLE_CONNECTION_MAX]; 
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern struct app_sec_env_tag app_sec_env; /// Application Security Environment

extern const struct app_subtask_handlers app_sec_handlers; /// Table of message handlers

/*
 * GLOBAL FUNCTIONS DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize the Application Security Module
 ****************************************************************************************
 */
void app_sec_init(void);


#if (NVDS_SUPPORT)
/**
 ****************************************************************************************
 * @brief Remove all bond data stored in NVDS
 ****************************************************************************************
 */
void app_sec_remove_bond(void);
#endif //(NVDS_SUPPORT)

/**
 ****************************************************************************************
 * @brief Send a security request to the peer device. This function is used to require the
 * central to start the encryption with a LTK that would have shared during a previous
 * bond procedure.
 *
 * @param[in] conidx: Connection Index
 ****************************************************************************************
 */
void app_sec_send_security_req(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Send a encrypt request to the peer device. This function is used to require the
 * central to start the encryption with a LTK.
 *
 * @param[in] conidx: Connection Index
 * @param[in] peer_addr: the peer device addres
 ****************************************************************************************
 */
void app_sec_encrypt_req(uint8_t conidx, bd_addr_t peer_addr);

/**
 ****************************************************************************************
 * @brief Return if the device is currently bonded
 ****************************************************************************************
 */
bool app_sec_get_bond_status_by_addr(bd_addr_t addr);

/**
 ****************************************************************************************
 * @brief Return if the device is currently bonded in the flash
 * @return if bonded return true, else return false.
 ****************************************************************************************
 */
bool app_sec_get_bond_status(void);
/**
 ****************************************************************************************
 * @brief Send a bond request to the peer device. This function is used to require the
 * central to start bond procedure.
 *
 * @param[in] conidx: Connection Index
 ****************************************************************************************
 */
void app_sec_bond_req(uint8_t conidx);


/**
 ****************************************************************************************
 * @brief when connect disconnect, call this function.
 *
 * @param[in] msgid  msgid
 * @param[in] param  param
 * @param[in] dest_id  dest id
 * @param[in] src_id  src id
 *
 * @return  KE_MSG_CONSUMED
 ****************************************************************************************
 */
int app_sec_gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                    void *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);
#endif //(BLE_APP_SEC)

/// @} APP_SEC

#endif // APP_SEC_H_

