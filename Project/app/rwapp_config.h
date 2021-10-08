/**
 * @file rwapp_config.h
 * @brief
 * @date Wed, Sep  5, 2018  5:44:25 PM
 * @author liqiang
 *
 * @addtogroup APP_SIMPLE_SERVER_CONFIG rwapp_config.h
 * @ingroup APP_SIMPLE_SERVER
 * @brief 使能profile，安全(sec)，白名单(whitelist)，隐私(privacy)
 * @details
 *
 * @{
 */

#ifndef __RWAPP_CONFIG_H__
#define __RWAPP_CONFIG_H__

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

/// enable/disable security connect
#define BLE_APP_SEC           1

/// enable/disable simple server profile
#define BLE_APP_SIMPLE_SERVER 1

/// enable/disable white list
#define BLE_APP_WHITE_LIST   0
/// enable/disable privacy
#define BLE_APP_PRIVACY      0

/// enable/disable dis profile
#define BLE_APP_DIS 0

#if (BLE_APP_DIS)
///Device Information Service Server Role
#define BLE_DIS_SERVER       1

/// Manufacturer Name Value
#define APP_DIS_MANUFACTURER_NAME       ("OnMicro")
/// Manufacturer Name Value size 
#define APP_DIS_MANUFACTURER_NAME_LEN   (sizeof(APP_DIS_MANUFACTURER_NAME))

/// Model Number String Value
#define APP_DIS_MODEL_NB_STR            ("Model Nbr 0.2")
/// Model Number String Value len
#define APP_DIS_MODEL_NB_STR_LEN        (sizeof(APP_DIS_MODEL_NB_STR))

/// Serial Number
#define APP_DIS_SERIAL_NB_STR           ("SerialNum0.2")
/// Serial Number len
#define APP_DIS_SERIAL_NB_STR_LEN       (sizeof(APP_DIS_SERIAL_NB_STR))

/// Firmware Revision
#define APP_DIS_FIRM_REV_STR            ("6621A3")
/// Firmware Revision len
#define APP_DIS_FIRM_REV_STR_LEN        (5)

/// System ID Value - LSB -> MSB
#define APP_DIS_SYSTEM_ID               ("\x00\x00\x01\x02\x00\x03\x04\x05")
/// System ID Value len
#define APP_DIS_SYSTEM_ID_LEN           (8)

/// Hardware Revision String
#define APP_DIS_HARD_REV_STR           ("1.3")
/// Hardware Revision String len
#define APP_DIS_HARD_REV_STR_LEN       (sizeof(APP_DIS_HARD_REV_STR))

/// Software Revision String
#define APP_DIS_SW_REV_STR              ("1.0.1.2")
/// Software Revision String len
#define APP_DIS_SW_REV_STR_LEN          (sizeof(APP_DIS_SW_REV_STR))

/// IEEE
#define APP_DIS_IEEE                    ("\x52\x54\x4B\x42\x65\x65\x49\x45\x45\x45\x44\x61\x74\x61\x6C\x69\x73\x74\x00")
/// IEEE len
#define APP_DIS_IEEE_LEN                (19)

/**
 * PNP ID Value - LSB -> MSB
 *      Vendor ID Source : 0x02 (USB Implementer’s Forum assigned Vendor ID value)
 *      Vendor ID : 0x045E      (Microsoft Corp)
 *      Product ID : 0x0040
 *      Product Version : 0x0300
 */
#define APP_DIS_PNP_ID               ("\x01\xBF\x01\xB8\x32\x79\x49")
/// dis pnp id len
#define APP_DIS_PNP_ID_LEN           (7)
#endif
/// the dis features
#define APP_DIS_FEATURES             (DIS_ALL_FEAT_SUP)

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */


#ifdef __cplusplus
}
#endif

#endif

/** @} */

