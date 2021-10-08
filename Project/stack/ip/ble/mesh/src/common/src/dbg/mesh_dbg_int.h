/**
 ****************************************************************************************
 *
 * @file mesh_dbg_int.h
 *
 * @brief Header file for Mesh Stack Debug Module Internal Definitions
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef MESH_DBG_INT_
#define MESH_DBG_INT_

/**
 ****************************************************************************************
 * @defgroup MESH_DBG_INT Mesh Stack Debug Module Internal Definitions
 * @ingroup MESH_DBG_INT
 * @brief Mesh Stack Debug Module Internal Definitions
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mesh_dbg.h"       // Mesh Stack Configuration

#if (BLE_MESH_DBG)

#include "mesh_api_msg.h"      // Mesh Stack Message Application Programming Interface
#include "mal.h"               // Mesh Abstraction Layer Definitions
#include "mesh_tb_timer.h"     // Mesh Stack Toolboxes Definitions
#include "mesh_tb_sec.h"       // Mesh Stack Toolboxes Definitions
#include "mesh_tb_buf.h"       // Mesh Stack Toolboxes Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// Allowed number of debug timers
#define MESH_DBG_TIMER_NB            (3)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Debug command codes for Mesh Stack
enum mesh_dbg_cmd_code
{
    /* ************************************************************************* */
    /* *                               SECURITY                                * */
    /* ************************************************************************* */
    /// Request execution of an AES function execution
    MESH_DBG_SEC_AES               = 0,
    /// Request execution of an AES-CMAC execution to authenticate data
    MESH_DBG_SEC_AES_CMAC,
    /// Request execution of an AES-S1 execution to authenticate data
    MESH_DBG_SEC_AES_S1,
    /// Request execution of an AES-K1 execution to authenticate data
    MESH_DBG_SEC_AES_K1,
    /// Request execution of an AES-K2 execution to authenticate data
    MESH_DBG_SEC_AES_K2,
    /// Request execution of an AES-K3 execution to authenticate data
    MESH_DBG_SEC_AES_K3,
    /// Request execution of an AES-K4 execution to authenticate data
    MESH_DBG_SEC_AES_K4,
    /// Request execution of an AES-CCM execution to encrypt data
    MESH_DBG_SEC_AES_CCM_ENC,
    /// Request execution of an AES-CCM execution to decrypt data
    MESH_DBG_SEC_AES_CCM_DEC,
    /// Request to read the P256 public key
    MESH_DBG_SEC_READ_P256_PUB_KEY,
    /// Request to generate DH-KEY using P256 algorithm
    MESH_DBG_SEC_GEN_DHKEY,


    /* ************************************************************************* */
    /* *                                 TIMER                                 * */
    /* ************************************************************************* */
    /// Request to set a timer
    MESH_DBG_TIMER_SET             = 20,
    /// Request to clear a timer
    MESH_DBG_TIMER_CLEAR,

    /* ************************************************************************* */
    /* *                                BUFFERS                                * */
    /* ************************************************************************* */
    /// Request to allocate a buffer
    MESH_DBG_BUF_ALLOC             = 30,
    /// Request to acquire a buffer
    MESH_DBG_BUF_ACQUIRE,
    /// Request to release a buffer
    MESH_DBG_BUF_RELEASE,
    /// Request to dump content of a buffer
    MESH_DBG_BUF_DUMP,
    /// Request to reserve/release head/tail part in order to control data part length
    MESH_DBG_BUF_UPDATE_DATA_LEN,
    /// Request to set update content of data part
    MESH_DBG_BUF_SET_DATA,
    /// Request to copy content of a buffer to another buffer
    MESH_DBG_BUF_COPY,
};

/// Debug indication codes for Mesh Stack
enum mesh_dbg_ind_code
{
    /* ************************************************************************* */
    /* *                                 TIMER                                 * */
    /* ************************************************************************* */
    /// Request to set a timer
    MESH_DBG_IND_TIMER             = 0,
};

/*
 * MESSAGE DEFINITIONS
 ****************************************************************************************
 */

/// Debug command required structure (without parameters)
typedef struct mesh_dbg_cmd
{
    /// Debug Command code (@see enum mesh_dbg_cmd_code)
    uint32_t cmd_code;
} mesh_dbg_cmd_t;

/// Debug command complete event required structure (without parameters)
typedef struct mesh_dbg_cmp_evt
{
    /// Debug Command code (@see enum mesh_dbg_cmd_code)
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
} mesh_dbg_cmp_evt_t;

/// Debug indication required structure (without parameters)
typedef struct mesh_dbg_ind
{
    /// Indication Code (@see enum mesh_dbg_ind_code)
    uint32_t ind_code;
} mesh_dbg_ind_t;

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Environment structure for Mesh Stack Debug Module
typedef struct mesh_dbg_env
{
    /// Pointer to the active command message
    const mesh_dbg_cmd_t     *p_cmd_msg;
    /// Pointer to the command complete event message
    const mesh_dbg_cmp_evt_t *p_cmp_evt_msg;
    /// Timers used for debug purpose
    mesh_tb_timer_t          timer_dbg[MESH_DBG_TIMER_NB];
    /// List of buffers
    co_list_t                list_buffers;
} mesh_dbg_env_t;

/*
 * VARIABLES DEFINITIONS
 ****************************************************************************************
 */

/// Mesh Stack Debug Module Environment
extern mesh_dbg_env_t *p_mesh_dbg_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Send a basic debug command complete message to the application
 *
 * @param[in] cmd_code  Debug Command Operation code (@see enum mesh_dbg_cmd_code)
 * @param[in] src_id    Source identifier of task that request command execution
 * @param[in] status    Status error code of the command execution (@see enum mesh_error)
 ****************************************************************************************
 */
void mesh_dbg_basic_cmp_evt_send(uint32_t cmd_code, uint16_t src_id, uint16_t status);

/**
 ****************************************************************************************
 * @brief Handle the debug commands for Security Manager
 *
 * @param[in] src_id    Command source id
 * @param[in] p_cmd     Command parameters of the message received
 *
 * @return Execution status (@see enum mesh_error)
 ****************************************************************************************
 */
uint16_t mesh_dbg_sec_cmd_handler(uint16_t src_id, const mesh_dbg_cmd_t *p_cmd);

/**
 ****************************************************************************************
 * @brief Handle the debug commands for Buffer Manager
 *
 * @param[in] src_id    Command source id
 * @param[in] p_cmd     Command parameters of the message received
 *
 * @return Execution status (@see enum mesh_error)
 ****************************************************************************************
 */
uint16_t mesh_dbg_buf_cmd_handler(uint16_t src_id, const mesh_dbg_cmd_t *p_cmd);

/**
 ****************************************************************************************
 * @brief Handle the debug commands for Timer Manager
 *
 * @param[in] src_id    Command source id
 * @param[in] p_cmd     Command parameters of the message received
 *
 * @return Execution status (@see enum mesh_error)
 ****************************************************************************************
 */
uint16_t mesh_dbg_timer_cmd_handler(uint16_t src_id, const mesh_dbg_cmd_t *p_cmd);

#endif //(BLE_MESH_DBG)

/// @} MESH_DBG_INT

#endif /* MESH_DBG_INT_ */
