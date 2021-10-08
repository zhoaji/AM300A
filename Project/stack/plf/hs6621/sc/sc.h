/**
 * @file sc.h
 * @brief 
 * @date Tue, Apr 23, 2019  7:38:15 PM
 * @author liqiang
 *
 * @defgroup 
 * @ingroup 
 * @brief 
 * @details 
 *
 * @{
 */

#ifndef __SC_H__
#define __SC_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <stdbool.h>

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */

/// EM config
typedef struct
{
#ifndef CONFIG_HS6621
    uint8_t activity_max;
#endif
    // EM_BLE_ACLTXBUF_NB (EM_BLE_ACLTXBUF_NB_MAX)
    uint8_t acltxbuf_nb;
    // EM_BLE_ACLTXBUF_SIZE (EM_BLE_ACLTXBUF_SIZE_MAX)
    uint16_t acltxbuf_size;

    // EM_BLE_DATARXBUF_NB (EM_BLE_DATARXBUF_NB_MAX)
    uint8_t datarxbuf_nb;
    // EM_BLE_DATARXBUF_SIZE (EM_BLE_DATARXBUF_SIZE_MAX)
    uint16_t datarxbuf_size;

    // EM_BLE_ADVDATATXBUF_NB (EM_BLE_ADVDATATXBUF_NB_MAX)
    uint8_t advdatatxbuf_nb;
    // EM_BLE_ADVDATATXBUF_SIZE BLE_ADV_FRAG_NB_TX_MAX
    uint8_t advdatatxbuf_frag_nb;
    // EM_BLE_ADVDATATXBUF_SIZE BLE_ADV_FRAG_SIZE_TX_MAX
    uint16_t advdatatxbuf_frag_size;
}em_confg_t;

/// stack config
typedef struct
{
    uint16_t    wakeup_time;
    uint16_t    lpclk_drift;
    uint8_t     lld_prog_delay;
    uint8_t     arb_event_start_delay;
    uint8_t     min_sleep_space;
    bool        hz32000;
    bool        dbg_fixed_p256_key;
    bool        coded_phy_500k;
    bool        rw_sleep_enable;
    em_confg_t  em;
}stack_config_t;

/*********************************************************************
 * EXTERN VARIABLES
 */
extern stack_config_t sc;

/*********************************************************************
 * EXTERN FUNCTIONS
 */


#ifdef __cplusplus
}
#endif

#endif

/** @} */

