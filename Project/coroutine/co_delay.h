/**
 * @file co_delay.h
 * @brief Software delay
 * @date Fri, Aug 31, 2018 11:16:47 AM
 * @author liqiang
 *
 * @defgroup CO_DELAY Delay
 * @ingroup COROUTINE
 * @brief Delay Module
 * @details 
 *
 * @{
 */

#ifndef __CO_DELAY_H__
#define __CO_DELAY_H__

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


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/// @cond
/**
 * @brief co_delay_init()
 *
 * @return 
 **/
void co_delay_init(void);
/// @endcond

/**
 * @brief delay millisecond
 *
 * @param[in] ms  millisecond
 *
 * @return None
 **/
void co_delay_ms(uint32_t ms);

/**
 * @brief delay microsecond
 *
 * @param[in] us  microsecond
 *
 * @return None
 **/
void co_delay_us(uint32_t us);

/**
 * @brief delay 10 microsecond
 *
 * @param[in] us10  10us
 *
 * @return None
 **/
void co_delay_10us(uint32_t us10);


#ifdef __cplusplus
}
#endif

#endif

/** @} */

