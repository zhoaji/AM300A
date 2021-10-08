/**
****************************************************************************************
*
* @file mm_tb.h
*
* @brief Header file for Mesh Model Toolbox
*
* Copyright (C) HunterSun 2019
*
* @author liuqingtao
****************************************************************************************
*/

#ifndef MM_TB_
#define MM_TB_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mm_inc.h"              // Mesh Model Include
#include "m_lay.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Head length for buffers allocated by the SIG Model
#define MM_TB_BUF_HEAD_LEN          (2)
/// Transition time system step resolution (uint ms)
#define MM_TB_SYS_TRANS_STEP_MS     (100)
/// Mesh Model Transition Step Number Unknow DTT
#define MM_TB_TRANS_UNKNOWN_DTT     (0x3F)
/// Mesh Model Transition Step Number Max DTT
#define MM_TB_TRANS_MAX_DTT         (0x3E)
/// Mesh Model max light temperature value
#define MM_TB_MAX_LIGHT_TEMP        (0x4E20)
/// Mesh Model min light temperature value
#define MM_TB_MIN_LIGHT_TEMP        (0x0320)
/// Clear transition timer
#define MM_TB_CLR_TIMER(timer)      do{                             \
                                        mesh_tb_timer_clear(timer); \
                                        (timer)->cb = NULL;         \
                                    }while(0);


/*
 * STRUCTURES
 ****************************************************************************************
 */
/// Mesh Models public structure
typedef struct mm_tb_public_env
{
    /// List of buffers containing meaasge to process
    co_list_t process_queue;
    /// Delayed job structure
    mal_djob_t djob;
    /// An instance of element
    uint8_t elmt_lid;
    /// Allocated Model LID
    uint8_t model_lid;
    /// Fast period divisor
    uint8_t fast_period_divisor;

    /// Function cb of process next message
    void (*cb_pro_next)(m_lid_t mdl_lid);

    /// Function cb of alloc buffer
    uint16_t (*cb_alloc_buf)(m_lid_t mdl_lid,mesh_tb_buf_t **pp_buf,uint16_t data_len);

    /// Function cb of publish message
    void (*cb_publish)(m_lid_t mdl_lid,mesh_tb_buf_t *p_buf,uint16_t opcode);

    /// Function cb of send message
    void (*cb_send)(m_lid_t mdl_lid,mesh_tb_buf_t *p_buf,uint32_t opcode);
}tb_gm;

/** Structure of monitor TID for the models:
    To retransmit the message, a Client shall use the same value for the TID filed
    as in the previously sent message, within 6 seconds from sending that message. */
typedef struct
{
    /// Source address
    uint16_t src;
    /// Destination address
    uint16_t dst;
    /// Previously received Opcode
    uint32_t opcode;
    /// Previously received TID
    uint8_t old_tid;
    /// New transaction flag
    bool new_transaction;
    /// Expiration timer
    mesh_tb_timer_t tid_timer;
} tid_monitor_t;

/// Mesh Model Non-volatile States Structure
typedef struct mm_states_nvds
{
    /// Generic OnPowerUp state (0:off; 1:default on; 2:restore)
    uint8_t on_powerup;
    //  Reserve for future use
    uint8_t rfu;
    /// Generic Power Level state
    uint16_t plevel_last;
    uint16_t plevel_default;
    uint16_t plevel_min;
    uint16_t plevel_max;
    uint16_t duv_last;
    uint16_t duv_default;
    /// Target Scene state
    uint16_t scene_target;
} mm_states_nvds_t;

/// Mesh Model Generic States Structure of an element instance
typedef struct mm_generic_states
{
    mm_states_nvds_t nvds;
    /// Generic OnOff state
    bool onoff_present;
    bool onoff_target;
    /// Generic Level state
    int16_t level_present;
    int16_t level_target;
    int16_t level_initial_present;
    /// Generic Power Level state
    uint16_t plevel_actual; //binging with level,onoff,onpowerup,range
} mm_generic_states_t;

/// Mesh Model Light Lightness State Structure of an element instance
typedef struct mm_lightness_state
{
    mm_generic_states_t generic;
    uint16_t lightness_linear_present; //binding with actual
    uint16_t lightness_linear_target;
    uint16_t lightness_actual_present; //binding with linear,level,onoff,onpowerup,range
    uint16_t lightness_actual_target;
    uint16_t lightness_initial_present;
    uint16_t lightness_last;
    uint16_t lightness_default;
    uint16_t lightness_min;
    uint16_t lightness_max;
} mm_lightness_state_t;

/// Mesh Model Light CTL State Structure of an element instance
typedef struct mm_lctl_state
{
    mm_generic_states_t generic;
    /* Lightness info borrow mm_lightness_state of main element */
    uint16_t cct_tmp_present; //binding with level,onpowerup,range
    uint16_t cct_tmp_target;  //binding with level,onpowerup,range
    int16_t  cct_duv_present; //binging with onpowerup
    int16_t  cct_duv_target;  //binging with onpowerup

    uint16_t cct_tmp_initial;
    int16_t  cct_duv_initial;

    uint16_t cct_tmp_default;
    int16_t  cct_duv_default;

    uint16_t cct_tmp_last;
    int16_t  cct_duv_last;

    uint16_t cct_tmp_min;
    uint16_t cct_tmp_max;
} mm_lctl_state_t;

/// Mesh Model Scene State Structure of an element instance
typedef struct mm_scene_state
{
    mm_generic_states_t generic;
    uint16_t scene_present;
    uint16_t scene_target;
} mm_scene_state_t;

/// Mesh Model Generic PowerOn Server Structure
typedef struct mm_gens_poo_env
{
    /// General structural data
    tb_gm common;
    /// Generic States
    mm_generic_states_t *p_states;
} mm_gens_poo_env_t;

typedef mm_gens_poo_env_t mm_gens_poos_env_t;

/// Mesh Model Generic Default Transition Time Server Structure
typedef struct mm_gens_dtt_env
{
    /// Generic structural data
    tb_gm common;
    /// Current value of DTT
    uint8_t cur_dtt;
} mm_gens_dtt_env_t;

/// Mesh Model Generic Onoff Server Structure
typedef struct mm_gens_onoff_env
{
    /// General structural data
    tb_gm common;
    /// Generic States
    mm_generic_states_t *p_states;
    /// TID monitor
    tid_monitor_t tid_monitor;
    /// Transition Time
    uint8_t trans_time;
    /// Remaining Time
    uint8_t remain_time;
    /// Delay
    uint8_t delay_time;
    /// Generic onoff delay timer
    mesh_tb_timer_t delay_timer;
    /// Generic onoff transition timer
    mesh_tb_timer_t trans_timer;
} mm_gens_onoff_env_t;

/// Mesh Model Generic Level Server Structure
typedef struct mm_gens_level_env
{
    /// General structural data
    tb_gm common;
    /// Generic States
    mm_generic_states_t *p_states;
    /// TID monitor
    tid_monitor_t tid_monitor;

    /// Transition Time
    uint8_t trans_time;
    /// Remain Time(!0 indicate that one timer is process)
    uint8_t remain_time;
    /// Delay Time
    uint8_t delay_time;
    /// Transition opcode source
    uint16_t trans_opc;

    /// Generic level timer
    mesh_tb_timer_t delay_timer;
    /// Generic level Trans timer
    mesh_tb_timer_t trans_timer;
    /// elapsed time
    uint32_t elapsed_time;

    /// Delta change of Generic Level State
    int32_t level_delta;
    /// Move Speed for Generic Level State
    int16_t level_move;
} mm_gens_level_env_t;

/// Mesh Model Light Lightness Server Structure
typedef struct mm_lights_ln_env
{
    /// General structural data
    tb_gm common;
    /// Lightness State
    mm_lightness_state_t *p_state;
    /// TID monitor
    tid_monitor_t tid_monitor;

    /// Transition time
    uint8_t trans_time;
    /// Remain Time(!0 indicate that one timer is process)
    uint8_t remain_time;
    /// Delay Time
    uint8_t delay_time;
    /// Transition opcode source
    uint16_t trans_opc;

    /// Generic level timer
    mesh_tb_timer_t delay_timer;
    /// Generic level Trans timer
    mesh_tb_timer_t trans_timer;
    /// elapsed time
    uint32_t elapsed_time;
} mm_lights_ln_env_t;

/// Mesh Model Light Lightness Setup Server Structure
typedef struct mm_lights_lns_env
{
    /// General structural data
    tb_gm common;
    /// Lightness State
    mm_lightness_state_t *p_state;
} mm_lights_lns_env_t;

/// Mesh Model Light CTL Server Structure
typedef struct mm_lights_ctl_env
{
    /// General structural data
    tb_gm common;
    /// Light CTL State
    mm_lctl_state_t *p_state;
    /// TID monitor
    tid_monitor_t tid_monitor;

    /// Transition time
    uint8_t trans_time;
    /// Remain Time(!0 indicate that one timer is process)
    uint8_t remain_time;
    /// Delay Time
    uint8_t delay_time;
    /// Transition opcode source
    uint16_t trans_opc;

    /// Generic level timer
    mesh_tb_timer_t delay_timer;
    /// Generic level Trans timer
    mesh_tb_timer_t trans_timer;
    /// elapsed time
    uint32_t elapsed_time;

} mm_lights_ctl_env_t;

typedef mm_lights_ctl_env_t  mm_lights_ctlt_env_t;
/// Mesh Model Light CTL Setup Server Structure
typedef struct mm_lights_ctls_env
{
    /// General structural data
    tb_gm common;
    /// Light CTL State
    mm_lctl_state_t *p_state;

} mm_lights_ctls_env_t;

typedef struct mm_tscns_scene_env
{
    /// General structural data
    tb_gm common;
    /// Scene server state
    mm_scene_state_t *p_state;

    /// Transition time
    uint8_t trans_time;
    /// Remain Time(!0 indicate that one timer is process)
    uint8_t remain_time;
    /// Delay Time
    uint8_t delay_time;
    /// Transition opcode source
    uint16_t trans_opc;

    /// Generic level timer
    mesh_tb_timer_t delay_timer;
    /// Generic level Trans timer
    mesh_tb_timer_t trans_timer;
    /// elapsed time
    uint32_t elapsed_time;

} mm_tscns_scene_env_t;
/// Mesh Models Callback Function Structure
extern const m_api_model_cb_t mm_cb;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
uint32_t mm_tb_get_trans_ms(uint8_t trans_val);
uint32_t mm_tb_get_delay_ms(uint8_t delay_val);
uint8_t  mm_tb_ms_to_tt_fmt(uint8_t fmt, uint32_t time_ms);

bool mm_tb_tid_validate(tid_monitor_t * p_tid_monitor, mesh_tb_buf_t *p_buf_rx, uint32_t opcode, uint8_t tid);
bool mm_tb_transaction_is_new(tid_monitor_t * p_tid_monitor);

void mm_tb_init_gm_cb(tb_gm* p_env);

#endif /* MM_TB_ */
