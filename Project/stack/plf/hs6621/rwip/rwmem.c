/**
 * @file rwmem.c
 * @brief 
 * @date Thu, Jun 13, 2019 10:05:01 AM
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details 
 *
 * @{
 */

/*********************************************************************
 * INCLUDES
 */
#include "rwip_config.h"     // RW SW configuration

#include <string.h>          // for mem* functions
#include <stdio.h>
#include "arch.h"            // Platform architecture definition
#include "compiler.h"
#include "co_version.h"      // version information
#include "co_utils.h"

#include "rwip.h"            // RW definitions

#if (NVDS_SUPPORT)
#include "nvds.h"         // NVDS definitions
#endif // NVDS_SUPPORT

#if (BLE_EMB_PRESENT)
#include "rwble.h"           // rwble definitions
#endif //BLE_EMB_PRESENT

#if (BLE_HOST_PRESENT)
#include "rwble_hl.h"        // BLE HL definitions
#include "gapc.h"
#include "gapm.h"
#include "gattc.h"
#include "l2cc.h"
#endif //BLE_HOST_PRESENT

//#if (BLE_APP_PRESENT)
//#include "app.h"             // Application definitions
//#endif //BLE_APP_PRESENT

#if (BT_EMB_PRESENT || BLE_EMB_PRESENT)
#include "sch_arb.h"            // Scheduling Arbiter
#include "sch_prog.h"           // Scheduling Programmer
#include "sch_plan.h"           // Scheduling Planner
#include "sch_slice.h"          // Scheduling Slicer
#include "sch_alarm.h"          // Scheduling Alarm
#endif //(BT_EMB_PRESENT || BLE_EMB_PRESENT)

#if (BT_EMB_PRESENT || BLE_EMB_PRESENT)
#include "rf.h"              // RF definitions
#endif //BT_EMB_PRESENT || BLE_EMB_PRESENT

#if (H4TL_SUPPORT)
#include "h4tl.h"
#endif //H4TL_SUPPORT

#if (AHI_TL_SUPPORT)
#include "ahi.h"
#endif //AHI_TL_SUPPORT

#if (HCI_PRESENT)
#include "hci.h"             // HCI definition
#endif //HCI_PRESENT

#include "ke.h"              // kernel definition
#include "ke_event.h"        // kernel event
#include "ke_timer.h"        // definitions for timer
#include "ke_mem.h"          // kernel memory manager

/*********************************************************************
 * MACROS
 */
// Heap header size is 12 bytes
#define RWIP_HEAP_HEADER                    (12 / sizeof(uint32_t))
// ceil(len/sizeof(uint32_t)) + RWIP_HEAP_HEADER
#define RWIP_CALC_HEAP_LEN(len)             ((((len) + (sizeof(uint32_t) - 1)) / sizeof(uint32_t)) + RWIP_HEAP_HEADER)
// compute size of the heap block in bytes
#define RWIP_CALC_HEAP_LEN_IN_BYTES(len)    (RWIP_CALC_HEAP_LEN(len) * sizeof(uint32_t))
// co_mem ext size
#ifdef CONFIG_RWIP_CO_MEM_EXT_SIZE
#define RWIP_CO_MEM_EXT_SIZE                CONFIG_RWIP_CO_MEM_EXT_SIZE
#else
#if defined(CONFIG_BOOTLOADER) && defined(CONFIG_HS6621) && !defined(CONFIG_USER_APP)
#define RWIP_CO_MEM_EXT_SIZE                (1024*12)
#else
#define RWIP_CO_MEM_EXT_SIZE                0
#endif
#endif

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
/// Heap definitions - use uint32 to ensure that memory blocks are 32bits aligned.
/// Memory allocated for environment variables
static uint32_t rwip_heap_env[RWIP_CALC_HEAP_LEN(RWIP_HEAP_ENV_SIZE) + RWIP_CO_MEM_EXT_SIZE];
#if (BLE_HOST_PRESENT)
/// Memory allocated for Attribute database
static uint32_t rwip_heap_db[RWIP_CALC_HEAP_LEN(RWIP_HEAP_DB_SIZE)];
#endif // (BLE_HOST_PRESENT)
/// Memory allocated for kernel messages
static uint32_t rwip_heap_msg[RWIP_CALC_HEAP_LEN(RWIP_HEAP_MSG_SIZE)];
/// Non Retention memory block
static uint32_t rwip_heap_non_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_NON_RET_SIZE)];

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief rwmem_init()
 *
 * @return 
 **/
void rwmem_init(void)
{
    // Initialize memory heap used by kernel.
    // Memory allocated for environment variables
    ke_mem_init(KE_MEM_ENV,           (uint8_t*)rwip_heap_env,     RWIP_CALC_HEAP_LEN_IN_BYTES(RWIP_HEAP_ENV_SIZE) + RWIP_CO_MEM_EXT_SIZE);
    #if (BLE_HOST_PRESENT)
    // Memory allocated for Attribute database
    ke_mem_init(KE_MEM_ATT_DB,        (uint8_t*)rwip_heap_db,      RWIP_CALC_HEAP_LEN_IN_BYTES(RWIP_HEAP_DB_SIZE));
    #endif // (BLE_HOST_PRESENT)
    // Memory allocated for kernel messages
    ke_mem_init(KE_MEM_KE_MSG,        (uint8_t*)rwip_heap_msg,     RWIP_CALC_HEAP_LEN_IN_BYTES(RWIP_HEAP_MSG_SIZE));
    // Non Retention memory block
    ke_mem_init(KE_MEM_NON_RETENTION, (uint8_t*)rwip_heap_non_ret, RWIP_CALC_HEAP_LEN_IN_BYTES(RWIP_HEAP_NON_RET_SIZE));
}

/** @} */

