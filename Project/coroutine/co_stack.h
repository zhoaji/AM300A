/*********************************************************************
 * @file co_stack.h
 * @brief 
 * @version 1.0
 * @date Thu 27 Nov 2014 10:30:40 AM CST
 * @author liqiang
 *
 * @note 
 */

#ifndef __CO_STACK_H__
#define __CO_STACK_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */


/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */
typedef void (* co_stack_mem_info_callback_t)(uint8_t **base, uint32_t *size);


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */
/**
 * @brief co_stack_init()
 *
 * @return 
 **/
void co_stack_init(void);

/**
 * @brief co_stack_check()
 *
 * @return 
 **/
bool co_stack_check(void);

/**
 * @brief co_stack_unused_space()
 *
 * @return 
 **/
unsigned co_stack_unused_space(void);

/**
 * @brief co_stack_register()
 *
 * @param[in] mem_info_cb  
 *
 * @return 
 **/
void co_stack_register(co_stack_mem_info_callback_t mem_info_cb);



#ifdef __cplusplus
}
#endif

#endif

