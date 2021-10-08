/**
 * @file cpft.h
 * @brief 
 * @date Fri 29 Sep 2017 05:29:29 PM CST
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details 
 *
 * @{
 */

#ifndef __CPFT_H__
#define __CPFT_H__

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
#define CPFT_ADC_CALIB_VALID_MASK   0x00000001
#define CPFT_VDDIO_3P3_VALID_MASK   0x00000002
#define CPFT_DVDD_1P0_VALID_MASK    0x00000004
#define CPFT_DCDC_1P27_VALID_MASK   0x00000008
#define CPFT_VCHARGE_1P0_VALID_MASK 0x00000010

/*********************************************************************
 * TYPEDEFS
 */
/// unpack code '4sHHI8s34I4f20x'
typedef struct
{
    uint32_t magic_code;
    uint16_t crc16;
    uint16_t length;
    uint32_t bitmap;
    uint8_t batch_number[8];

    // bitmap valid section
    adc_cal_table_t adc_calib;  // bitmap0
    float vddio_3p3;            // bitmap1
    float dvdd_1p0;             // bitmap2
    float dcdc_1p27;            // bitmap3
    float vcharge_1p0;          // bitmap4
    uint32_t reserve[5];
}cpft_data_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief  cpft data get
 *
 * @param[in] cpft  cpft
 *
 * @return ok ?
 **/
bool cpft_data_get(cpft_data_t *cpft);

/**
 * @brief  cpft data setup
 *
 * @param[in] cpft  cpft
 **/
void cpft_data_setup(cpft_data_t *cpft);

/**
 * @brief  cpft setup
 **/
void cpft_setup(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

