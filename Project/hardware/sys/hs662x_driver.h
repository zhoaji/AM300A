/**
 * @file hs6620_driver.h
 * @brief 
 * @date Fri 27 Nov 2015 07:01:00 PM CST
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details 
 *
 * @{
 */

#ifndef __HS6620_DRIVER_H__
#define __HS6620_DRIVER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>

/*********************************************************************
 * MACROS
 */

#define MASK_1REG(name1, value1) \
    name1##_MASK, \
    ((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))

#define MASK_2REG(name1, value1, \
                  name2, value2) \
    name1##_MASK | \
    name2##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK)))

#define MASK_3REG(name1, value1, \
                  name2, value2, \
                  name3, value3) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK)))

#define MASK_4REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK)))

#define MASK_5REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK)))

#define MASK_6REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK)))

#define MASK_7REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6, \
                  name7, value7) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK | \
    name7##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK))) | \
    (((((uint32_t)(value7))<<(name7##_POS))&(name7##_MASK)))

#define MASK_8REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6, \
                  name7, value7, \
                  name8, value8) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK | \
    name7##_MASK | \
    name8##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK))) | \
    (((((uint32_t)(value7))<<(name7##_POS))&(name7##_MASK))) | \
    (((((uint32_t)(value8))<<(name8##_POS))&(name8##_MASK)))

#define MASK_9REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6, \
                  name7, value7, \
                  name8, value8, \
                  name9, value9) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK | \
    name7##_MASK | \
    name8##_MASK | \
    name9##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK))) | \
    (((((uint32_t)(value7))<<(name7##_POS))&(name7##_MASK))) | \
    (((((uint32_t)(value8))<<(name8##_POS))&(name8##_MASK))) | \
    (((((uint32_t)(value9))<<(name9##_POS))&(name9##_MASK)))

#define MASK_10REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6, \
                  name7, value7, \
                  name8, value8, \
                  name9, value9, \
                  name10, value10) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK | \
    name7##_MASK | \
    name8##_MASK | \
    name9##_MASK | \
    name10##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK))) | \
    (((((uint32_t)(value7))<<(name7##_POS))&(name7##_MASK))) | \
    (((((uint32_t)(value8))<<(name8##_POS))&(name8##_MASK))) | \
    (((((uint32_t)(value9))<<(name9##_POS))&(name9##_MASK))) | \
    (((((uint32_t)(value10))<<(name10##_POS))&(name10##_MASK)))

#define MASK_11REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6, \
                  name7, value7, \
                  name8, value8, \
                  name9, value9, \
                  name10, value10, \
                  name11, value11) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK | \
    name7##_MASK | \
    name8##_MASK | \
    name9##_MASK | \
    name10##_MASK | \
    name11##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK))) | \
    (((((uint32_t)(value7))<<(name7##_POS))&(name7##_MASK))) | \
    (((((uint32_t)(value8))<<(name8##_POS))&(name8##_MASK))) | \
    (((((uint32_t)(value9))<<(name9##_POS))&(name9##_MASK))) | \
    (((((uint32_t)(value10))<<(name10##_POS))&(name10##_MASK))) | \
    (((((uint32_t)(value11))<<(name11##_POS))&(name11##_MASK)))

#define MASK_12REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6, \
                  name7, value7, \
                  name8, value8, \
                  name9, value9, \
                  name10, value10, \
                  name11, value11, \
                  name12, value12) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK | \
    name7##_MASK | \
    name8##_MASK | \
    name9##_MASK | \
    name10##_MASK | \
    name11##_MASK | \
    name12##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK))) | \
    (((((uint32_t)(value7))<<(name7##_POS))&(name7##_MASK))) | \
    (((((uint32_t)(value8))<<(name8##_POS))&(name8##_MASK))) | \
    (((((uint32_t)(value9))<<(name9##_POS))&(name9##_MASK))) | \
    (((((uint32_t)(value10))<<(name10##_POS))&(name10##_MASK))) | \
    (((((uint32_t)(value11))<<(name11##_POS))&(name11##_MASK))) | \
    (((((uint32_t)(value12))<<(name12##_POS))&(name12##_MASK)))

#define MASK_13REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6, \
                  name7, value7, \
                  name8, value8, \
                  name9, value9, \
                  name10, value10, \
                  name11, value11, \
                  name12, value12, \
                  name13, value13) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK | \
    name7##_MASK | \
    name8##_MASK | \
    name9##_MASK | \
    name10##_MASK | \
    name11##_MASK | \
    name12##_MASK | \
    name13##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK))) | \
    (((((uint32_t)(value7))<<(name7##_POS))&(name7##_MASK))) | \
    (((((uint32_t)(value8))<<(name8##_POS))&(name8##_MASK))) | \
    (((((uint32_t)(value9))<<(name9##_POS))&(name9##_MASK))) | \
    (((((uint32_t)(value10))<<(name10##_POS))&(name10##_MASK))) | \
    (((((uint32_t)(value11))<<(name11##_POS))&(name11##_MASK))) | \
    (((((uint32_t)(value12))<<(name12##_POS))&(name12##_MASK))) | \
    (((((uint32_t)(value13))<<(name13##_POS))&(name13##_MASK)))

#define MASK_14REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6, \
                  name7, value7, \
                  name8, value8, \
                  name9, value9, \
                  name10, value10, \
                  name11, value11, \
                  name12, value12, \
                  name13, value13, \
                  name14, value14) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK | \
    name7##_MASK | \
    name8##_MASK | \
    name9##_MASK | \
    name10##_MASK | \
    name11##_MASK | \
    name12##_MASK | \
    name13##_MASK | \
    name14##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK))) | \
    (((((uint32_t)(value7))<<(name7##_POS))&(name7##_MASK))) | \
    (((((uint32_t)(value8))<<(name8##_POS))&(name8##_MASK))) | \
    (((((uint32_t)(value9))<<(name9##_POS))&(name9##_MASK))) | \
    (((((uint32_t)(value10))<<(name10##_POS))&(name10##_MASK))) | \
    (((((uint32_t)(value11))<<(name11##_POS))&(name11##_MASK))) | \
    (((((uint32_t)(value12))<<(name12##_POS))&(name12##_MASK))) | \
    (((((uint32_t)(value13))<<(name13##_POS))&(name13##_MASK))) | \
    (((((uint32_t)(value14))<<(name14##_POS))&(name14##_MASK)))

#define MASK_15REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6, \
                  name7, value7, \
                  name8, value8, \
                  name9, value9, \
                  name10, value10, \
                  name11, value11, \
                  name12, value12, \
                  name13, value13, \
                  name14, value14, \
                  name15, value15) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK | \
    name7##_MASK | \
    name8##_MASK | \
    name9##_MASK | \
    name10##_MASK | \
    name11##_MASK | \
    name12##_MASK | \
    name13##_MASK | \
    name14##_MASK | \
    name15##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK))) | \
    (((((uint32_t)(value7))<<(name7##_POS))&(name7##_MASK))) | \
    (((((uint32_t)(value8))<<(name8##_POS))&(name8##_MASK))) | \
    (((((uint32_t)(value9))<<(name9##_POS))&(name9##_MASK))) | \
    (((((uint32_t)(value10))<<(name10##_POS))&(name10##_MASK))) | \
    (((((uint32_t)(value11))<<(name11##_POS))&(name11##_MASK))) | \
    (((((uint32_t)(value12))<<(name12##_POS))&(name12##_MASK))) | \
    (((((uint32_t)(value13))<<(name13##_POS))&(name13##_MASK))) | \
    (((((uint32_t)(value14))<<(name14##_POS))&(name14##_MASK))) | \
    (((((uint32_t)(value15))<<(name15##_POS))&(name15##_MASK)))

#define MASK_16REG(name1, value1, \
                  name2, value2, \
                  name3, value3, \
                  name4, value4, \
                  name5, value5, \
                  name6, value6, \
                  name7, value7, \
                  name8, value8, \
                  name9, value9, \
                  name10, value10, \
                  name11, value11, \
                  name12, value12, \
                  name13, value13, \
                  name14, value14, \
                  name15, value15, \
                  name16, value16) \
    name1##_MASK | \
    name2##_MASK | \
    name3##_MASK | \
    name4##_MASK | \
    name5##_MASK | \
    name6##_MASK | \
    name7##_MASK | \
    name8##_MASK | \
    name9##_MASK | \
    name10##_MASK | \
    name11##_MASK | \
    name12##_MASK | \
    name13##_MASK | \
    name14##_MASK | \
    name15##_MASK | \
    name16##_MASK, \
    (((((uint32_t)(value1))<<(name1##_POS))&(name1##_MASK))) | \
    (((((uint32_t)(value2))<<(name2##_POS))&(name2##_MASK))) | \
    (((((uint32_t)(value3))<<(name3##_POS))&(name3##_MASK))) | \
    (((((uint32_t)(value4))<<(name4##_POS))&(name4##_MASK))) | \
    (((((uint32_t)(value5))<<(name5##_POS))&(name5##_MASK))) | \
    (((((uint32_t)(value6))<<(name6##_POS))&(name6##_MASK))) | \
    (((((uint32_t)(value7))<<(name7##_POS))&(name7##_MASK))) | \
    (((((uint32_t)(value8))<<(name8##_POS))&(name8##_MASK))) | \
    (((((uint32_t)(value9))<<(name9##_POS))&(name9##_MASK))) | \
    (((((uint32_t)(value10))<<(name10##_POS))&(name10##_MASK))) | \
    (((((uint32_t)(value11))<<(name11##_POS))&(name11##_MASK))) | \
    (((((uint32_t)(value12))<<(name12##_POS))&(name12##_MASK))) | \
    (((((uint32_t)(value13))<<(name13##_POS))&(name13##_MASK))) | \
    (((((uint32_t)(value14))<<(name14##_POS))&(name14##_MASK))) | \
    (((((uint32_t)(value15))<<(name15##_POS))&(name15##_MASK))) | \
    (((((uint32_t)(value16))<<(name16##_POS))&(name16##_MASK)))


#define MASK_POS(name) \
    name##_MASK, name##_POS


// register op
#define REGW   register_set
#define REGWA  register_set_raw
#define REGW1  register_set1
#define REGW0  register_set0
#define REGR   register_get

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */
/**
 * @brief register_set()
 *
 * @param[in] reg  
 * @param[in] mask  
 * @param[in] value  
 *
 * @return 
 **/
__STATIC_FORCEINLINE void register_set(volatile uint32_t *reg, uint32_t mask, uint32_t value)
{
    register uint32_t reg_prev;

    reg_prev = *reg;
    reg_prev &= ~mask;
    reg_prev |= mask & value;
    *reg = reg_prev;
}

/**
 * @brief register_set()
 *
 * @param[in] reg  
 * @param[in] mask  
 * @param[in] value  
 *
 * @return 
 **/
__STATIC_FORCEINLINE void register_set_raw(volatile uint32_t *reg, uint32_t mask, uint32_t value)
{
    *reg = mask & value;
}

/**
 * @brief register_set1()
 *
 * @param[in] reg  
 * @param[in] mask  
 *
 * @return 
 **/
__STATIC_FORCEINLINE void register_set1(volatile uint32_t *reg, uint32_t mask)
{
    *reg |= mask;
}

/**
 * @brief register_set0()
 *
 * @param[in] reg  
 * @param[in] mask  
 *
 * @return 
 **/
__STATIC_FORCEINLINE void register_set0(volatile uint32_t *reg, uint32_t mask)
{
    *reg &= ~mask;
}

/**
 * @brief register_get()
 *
 * @param[in] reg  
 * @param[in] mask  
 * @param[in] pos  
 *
 * @return 
 **/
__STATIC_FORCEINLINE uint32_t register_get(const volatile uint32_t *reg, uint32_t mask, uint32_t pos)
{
    return (*reg & mask) >> pos;
}


#ifdef __cplusplus
}
#endif

#endif

/** @} */

