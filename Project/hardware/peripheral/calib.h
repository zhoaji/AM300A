/**
 * @file calib.h
 * @brief 
 * @date 2016/12/26 20:15:13
 * @author liqiang
 *
 * @defgroup 
 * @ingroup 
 * @brief
 * @details 
 *
 * @{
 */

#ifndef __CALIB_H__
#define __CALIB_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "calib_repair.h"

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

/**
 * @brief  calib rc32k accuracy check
 *
 * @param[in] win_32k_num  win 32k num
 *
 * @return ppm
 **/
int calib_rc32k_accuracy_check(uint32_t win_32k_num);

/**
 * @brief calib rc32k
 *
 * @return 
 **/
void calib_rc32k(void);

/**
 * @brief calib sys rc32m
 **/
void calib_sys_rc32m(void);

/**
 * @brief calib sys rc
 **/
void calib_sys_rc(void);

/**
 * @brief calib rf
 **/
void calib_rf(void);

/**
 * @brief  calib rf pa save
 **/
void calib_rf_pa_save(void);

/**
 * @brief calib rf restore
 **/
void calib_rf_restore(void);

/**
 * @brief  calib sys restore
 **/
void calib_sys_restore(void);

/**
 * @brief  calib flash save
 **/
void calib_flash_save(void);

/**
 * @brief  calib flash restore
 *
 * @return
 **/
bool calib_flash_restore(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

