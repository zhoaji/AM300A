/**
 * @file codec_wm8753.h
 * @brief 
 * @date Mon 05 Jun 2017 01:11:04 PM CST
 * @author liqiang
 *
 * @defgroup 
 * @ingroup 
 * @brief
 * @details 
 *
 * @{
 */

#ifndef __CODEC_WM8753_H__
#define __CODEC_WM8753_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
    CODEC_WM8753_SAMPLE_RATE_8KHZ   = 8000,
    CODEC_WM8753_SAMPLE_RATE_16KHZ  = 16000,
    CODEC_WM8753_SAMPLE_RATE_24KHZ  = 24000,
    CODEC_WM8753_SAMPLE_RATE_48KHZ  = 48000,
    CODEC_WM8753_SAMPLE_RATE_96KHZ  = 96000,
}codec_wm8753_sample_rate_t;

typedef enum
{
    CODEC_WM8753_MCLK_12MHZ,
    CODEC_WM8753_MCLK_24MHZ,
}codec_wm8753_mclk_t;

typedef enum
{
    CODEC_WM8753_SLAVE  = 0,
    CODEC_WM8753_MASTER = 1,
}codec_wm8753_mode_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief codec_wm8753_init()
 *
 * @param[in] mode  
 * @param[in] sample_rate  
 * @param[in] mclk  
 *
 * @return 
 **/
bool codec_wm8753_init(codec_wm8753_mode_t mode,
                       codec_wm8753_sample_rate_t sample_rate,
                       codec_wm8753_mclk_t mclk);


#ifdef __cplusplus
}
#endif

#endif

/** @} */

