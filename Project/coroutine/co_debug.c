/*********************************************************************
 * @file co_debug.c
 * @brief
 * @version 1.0
 * @date 14/11/3 19:30:15
 * @author liqiang
 *
 * @note
 */

/*********************************************************************
 * INCLUDES
 */
#include "rwip_config.h" // RW SW configuration

#include "co.h"
#include "arch.h"      // architectural platform definitions
#include "rwip.h"      // RW SW initialization
#include "dbg.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

#ifdef CONFIG_FUNCTION_PROFILING

#define PROFILE_NUM 1000

typedef struct
{
    void *func;
    uint32_t count;
}cyg_profile_func_t;

typedef struct
{
    uint32_t index;
    cyg_profile_func_t profile[PROFILE_NUM];
}cyg_profile_func_env_t;

cyg_profile_func_env_t cyg_profile_func_env = {0};

/**
 * @brief  cyg profile func clear
 **/
void cyg_profile_func_clear(void)
{
    cyg_profile_func_env.index = 0;
    memset(cyg_profile_func_env.profile, 0, sizeof(cyg_profile_func_env.profile));
}

/**
 * @brief  cyg profile func sort
 **/
void cyg_profile_func_sort(void)
{
    int i, j;
    cyg_profile_func_t temp;

    for (i=0; i<cyg_profile_func_env.index; ++i)
    {
        for (j=i+1; j<cyg_profile_func_env.index; ++j)
        {
            if (cyg_profile_func_env.profile[j].count > cyg_profile_func_env.profile[i].count)
            {
                temp = cyg_profile_func_env.profile[i];
                cyg_profile_func_env.profile[i] = cyg_profile_func_env.profile[j];
                cyg_profile_func_env.profile[j] = temp;
            }
        }
    }
}

/**
 * @brief  cyg profile func enter
 *
 * @param[in] this_fn  this fn
 * @param[in] call_site  call site
 **/
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    if (cyg_profile_func_env.index < PROFILE_NUM)
    {
        int index;
        for (index=0; index<cyg_profile_func_env.index; ++index)
        {
            if (cyg_profile_func_env.profile[index].func == this_fn)
                break;
        }

        if (index == cyg_profile_func_env.index)
        {
            cyg_profile_func_env.profile[index].func = this_fn;
            cyg_profile_func_env.index++;
        }

        cyg_profile_func_env.profile[index].count++;
    }
}

/**
 * @brief  cyg profile func exit
 *
 * @param[in] this_fn  this fn
 * @param[in] call_site  call site
 **/
void __cyg_profile_func_exit(void *this_fn, void *call_site)
{

}

#endif

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

#if (defined(CONFIG_ASSERT) || defined(CFG_DBG))
/**
 * @brief __co_assert()
 *
 * @param[in] exp  
 * @param[in] file  
 * @param[in] func  
 * @param[in] line  
 *
 * @return 
 **/
int __co_assert(const char *exp, const char *file, const char *func, int line)
{
    log_error("### assert fail, %s[%d]{%s()}:\"%s\" ###\n", file, line, func, exp);
    co_fault_assert_set(exp, file, func, line);
    return 0;
}
#endif

#ifdef CFG_DBG

// for RW
void assert_err(const char *condition, const char * file, int line)
{
    TRC_REQ_SW_ASS_ERR(file, line, 0, 0);

    // Trigger assert message
    rwip_assert(file, line, 0, 0, ASSERT_TYPE_ERROR);

    __co_assert(condition, file, "assert_err", line);
}

// for RW
void assert_param(int param0, int param1, const char * file, int line)
{
    TRC_REQ_SW_ASS_ERR(file, line, param0, param1);

    // Trigger assert message
    rwip_assert(file, line, param0, param1, ASSERT_TYPE_ERROR);

    __co_assert("assert_param", file, "assert_param", line);
}

// for RW
void assert_warn(int param0, int param1, const char * file, int line)
{
    TRC_REQ_SW_ASS_WARN(file, line, param0, param1);

    // Trigger assert message
    rwip_assert(file, line, param0, param1, ASSERT_TYPE_WARNING);

    log_warn("assert_warn: 0x%08x, 0x%08x (%s[%d])\n", param0, param1, file, line);
}

// for RW
void dump_data(uint8_t* data, uint16_t length)
{
}

#endif //PLF_DEBUG

