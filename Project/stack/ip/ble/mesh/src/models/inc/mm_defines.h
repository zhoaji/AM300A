/**
 ****************************************************************************************
 *
 * @file mm_defines.h
 *
 * @brief Header file for Mesh Model Definitions
 *
 * Copyright (C) RivieraWaves 2017-2018
 *
 ****************************************************************************************
 */

#ifndef MM_DEFINES_
#define MM_DEFINES_

/**
 ****************************************************************************************
 * @defgroup MM_DEFINES Mesh Model Definitions
 * @ingroup MESH_MDL
 * @brief  Mesh Model Defines
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES (MODEL IDS)
 ****************************************************************************************
 */

/// ************************** Model IDs for Generic Models *****************************

/// Generic Server - On/Off
#define MM_ID_GENS_OO           (0x1000)
/// Generic Server - Level
#define MM_ID_GENS_LVL          (0x1002)
/// Generic Server - Default Transition Time
#define MM_ID_GENS_DTT          (0x1004)
/// Generic Server - Power On/Off
#define MM_ID_GENS_POO          (0x1006)
/// Generic Server - Power On/Off Setup
#define MM_ID_GENS_POOS         (0x1007)
/// Generic Server - Power Level
#define MM_ID_GENS_PLVL         (0x1009)
/// Generic Server - Power Level Setup
#define MM_ID_GENS_PLVLS        (0x100A)
/// Generic Server - Battery
#define MM_ID_GENS_BAT          (0x100C)
/// Generic Server - Location
#define MM_ID_GENS_LOC          (0x100E)
/// Generic Server - Location Setup
#define MM_ID_GENS_LOCS         (0x100F)
/// Generic Server - User Property
#define MM_ID_GENS_UPROP        (0x1013)
/// Generic Server - Admin Property
#define MM_ID_GENS_APROP        (0x1011)
/// Generic Server - Manufacturer Property
#define MM_ID_GENS_MPROP        (0x1012)
/// Generic Server - Client Property
#define MM_ID_GENS_CPROP        (0x1014)

/// Generic Client - On/Off
#define MM_ID_GENC_OO           (0x1001)
/// Generic Client - Level
#define MM_ID_GENC_LVL          (0x1003)
/// Generic Client - Default Transition Time
#define MM_ID_GENC_DTT          (0x1005)
/// Generic Client - Power On/Off
#define MM_ID_GENC_POO          (0x1008)
/// Generic Client - Location
#define MM_ID_GENC_LOC          (0x1010)
/// Generic Client - Battery
#define MM_ID_GENC_BAT          (0x100D)
/// Generic Client - Property
#define MM_ID_GENC_PROP         (0x1015)

/// ************************** Model IDs for Sensors Models *****************************

/// Sensors Server - Sensor
#define MM_ID_SENS_SEN          (0x1100)
/// Sensors Server - Sensor Setup
#define MM_ID_SENS_SENS         (0x1101)

/// Sensors Client - Sensor
#define MM_ID_SENC_SEN          (0x1102)

/// ********************** Model IDs for Time and Scenes Models *************************

/// Time and Scene Server - Time
#define MM_ID_TSCNS_TIM         (0x1200)
/// Time and Scene Server - Time Setup
#define MM_ID_TSCNS_TIMS        (0x1201)
/// Time and Scene Server - Scene
#define MM_ID_TSCNS_SCN         (0x1203)
/// Time and Scene Server - Scene Setup
#define MM_ID_TSCNS_SCNS        (0x1204)
/// Time and Scene Server - Scheduler
#define MM_ID_TSCNS_SCH         (0x1206)
/// Time and Scene Server - Scheduler Setup
#define MM_ID_TSCNS_SCHS        (0x1207)

/// Time and Scene Client - Time
#define MM_ID_TSCNC_TIM         (0x1202)
/// Time and Scene Client - Scene
#define MM_ID_TSCNC_SCN         (0x1205)
/// Time and Scene Client - Scheduler
#define MM_ID_TSCNC_SCH         (0x1208)

/// ************************** Model IDs for Lighting Models ****************************

/// Lighting Server - Light Lightness
#define MM_ID_LIGHTS_LN         (0x1300)
/// Lighting Server - Light Lightness Setup
#define MM_ID_LIGHTS_LNS        (0x1301)
/// Lighting Server - Light CTL
#define MM_ID_LIGHTS_CTL        (0x1303)
/// Lighting Server - Light CTL Temperature
#define MM_ID_LIGHTS_CTLT       (0x1306)
/// Lighting Server - Light CTL Setup
#define MM_ID_LIGHTS_CTLS       (0x1304)
/// Lighting Server - Light HSL
#define MM_ID_LIGHTS_HSL        (0x1307)
/// Lighting Server - Light HSL Hue
#define MM_ID_LIGHTS_HSLH       (0x130A)
/// Lighting Server - Light HSL Saturation
#define MM_ID_LIGHTS_HSLSAT     (0x130B)
/// Lighting Server - Light HSL Setup
#define MM_ID_LIGHTS_HSLS       (0x1308)
/// Lighting Server - Light xyL
#define MM_ID_LIGHTS_XYL        (0x130C)
/// Lighting Server - Light xyL Setup
#define MM_ID_LIGHTS_XYLS       (0x130D)
/// Lighting Server - Light LC
#define MM_ID_LIGHTS_LC         (0x130F)
/// Lighting Server - Light LC Setup
#define MM_ID_LIGHTS_LCS        (0x1310)

/// Lighting Client - Light Lightness
#define MM_ID_LIGHTC_LN         (0x1302)
/// Lighting Client - Light CTL
#define MM_ID_LIGHTC_CTL        (0x1305)
/// Lighting Client - Light HSL
#define MM_ID_LIGHTC_HSL        (0x1309)
/// Lighting Client - Light xyL
#define MM_ID_LIGHTC_XYL        (0x130E)
/// Lighting Client - Light LC
#define MM_ID_LIGHTC_LC         (0x1311)

/*
 * DEFINES (MESSAGE IDS)
 ****************************************************************************************
 */

/// ********************* Message IDs for Generic On/Off Model **************************

/// Generic OnOff Get
#define MM_MSG_GEN_OO_GET               (0x0182)
/// Generic OnOff Set
#define MM_MSG_GEN_OO_SET               (0x0282)
/// Generic OnOff Set Unacknowledged
#define MM_MSG_GEN_OO_SET_UNACK         (0x0382)
/// Generic OnOff Status
#define MM_MSG_GEN_OO_STATUS            (0x0482)

/// ********************* Message IDs for Generic Level Model ***************************

/// Generic Level Get
#define MM_MSG_GEN_LVL_GET              (0x0582)
/// Generic Level Set
#define MM_MSG_GEN_LVL_SET              (0x0682)
/// Generic Level Set Unacknowledged
#define MM_MSG_GEN_LVL_SET_UNACK        (0x0782)
/// Generic Level Status
#define MM_MSG_GEN_LVL_STATUS           (0x0882)
/// Generic Delta Set
#define MM_MSG_GEN_DELTA_SET            (0x0982)
/// Generic Delta Set Unacknowledged
#define MM_MSG_GEN_DELTA_SET_UNACK      (0x0A82)
/// Generic Move Set
#define MM_MSG_GEN_MOVE_SET             (0x0B82)
/// Generic Move Set Unacknowledged
#define MM_MSG_GEN_MOVE_SET_UNACK       (0x0C82)

/// ************** Message IDs for Generic Default Transition Time Model ****************

/// Generic Default Transition Time Get
#define MM_MSG_GEN_DTT_GET              (0x0D82)
/// Generic Default Transition Time Set
#define MM_MSG_GEN_DTT_SET              (0x0E82)
/// Generic Default Transition Time Set Unacknowledged
#define MM_MSG_GEN_DTT_SET_UNACK        (0x0F82)
/// Generic Default Transition Time Status
#define MM_MSG_GEN_DTT_STATUS           (0x1082)

/// ******************** Message IDs for Generic Power On/Off Model *********************

/// Generic OnPowerUp Get
#define MM_MSG_GEN_ONPUP_GET            (0x1182)
/// Generic OnPowerUp Status
#define MM_MSG_GEN_ONPUP_STATUS         (0x1282)

/// ******************** Message IDs for Generic Power On/Off Model *********************

/// Generic OnPowerUp Set
#define MM_MSG_GEN_ONPUP_SET            (0x1382)
/// Generic OnPowerUp Set Unacknowledged
#define MM_MSG_GEN_ONPUP_SET_UNACK      (0x1482)

/// ******************** Message IDs for Generic Power Level Model **********************

/// Generic Power Level Get
#define MM_MSG_GEN_PLVL_GET             (0x1582)
/// Generic Power Level Set
#define MM_MSG_GEN_PLVL_SET             (0x1682)
/// Generic Power Level Set Unacknowledged
#define MM_MSG_GEN_PLVL_SET_UNACK       (0x1782)
/// Generic Power Level Status
#define MM_MSG_GEN_PLVL_STATUS          (0x1882)
/// Generic Power Last Get
#define MM_MSG_GEN_PLAST_GET            (0x1982)
/// Generic Power Last Status
#define MM_MSG_GEN_PLAST_STATUS         (0x1A82)
/// Generic Power Default Get
#define MM_MSG_GEN_PDFLT_GET            (0x1B82)
/// Generic Power Default Status
#define MM_MSG_GEN_PDFLT_STATUS         (0x1C82)
/// Generic Power Range Get
#define MM_MSG_GEN_PRANGE_GET           (0x1D82)
/// Generic Power Range Status
#define MM_MSG_GEN_PRANGE_STATUS        (0x1E82)

/// ***************** Message IDs for Generic Power Level Setup Model *******************

/// Generic Power Default Set
#define MM_MSG_GEN_PDFLT_SET            (0x1F82)
/// Generic Power Default Set Unacknowledged
#define MM_MSG_GEN_PDFLT_SET_UNACK      (0x2082)
/// Generic Power Range Set
#define MM_MSG_GEN_PRANGE_SET           (0x2182)
/// Generic Power Range Set Unacknowledged
#define MM_MSG_GEN_PRANGE_SET_UNACK     (0x2282)

/// ***************** Message IDs for Generic Battery Model *******************

/// Generic Battery Get
#define MM_MSG_GEN_BAT_GET              (0x2382)
/// Generic Battery Status
#define MM_MSG_GEN_BAT_STATUS           (0x2482)

/// ***************** Message IDs for Generic Location Model *******************

/// Generic Location Global Get
#define MM_MSG_GEN_LOCG_GET             (0x2582)
/// Generic Location Global Status
#define MM_MSG_GEN_LOCG_STATUS          (0x40)
/// Generic Location Local Get
#define MM_MSG_GEN_LOCL_GET             (0x2682)
/// Generic Location Local Status
#define MM_MSG_GEN_LOCL_STATUS          (0x2782)

/// ***************** Message IDs for Generic Location Setup Model *******************

/// Generic Location Global Set
#define MM_MSG_GEN_LOCG_SET             (0x41)
/// Generic Location Global Set Unacknowledged
#define MM_MSG_GEN_LOCG_SET_UNACK       (0x42)
/// Generic Location Local Set
#define MM_MSG_GEN_LOCL_SET             (0x2882)
/// Generic Location Local Set Unacknowledged
#define MM_MSG_GEN_LOCL_SET_UNACK       (0x2982)

/// ***************** Message IDs for Generic Manufacturer Property Model *******************

/// Generic Manufacturer Properties Get
#define MM_MSG_GEN_MPROPS_GET           (0x2A82)
/// Generic Manufacturer Properties Status
#define MM_MSG_GEN_MPROPS_STATUS        (0x43)
/// Generic Manufacturer Property Get
#define MM_MSG_GEN_MPROP_GET            (0x2B82)
/// Generic Manufacturer Property Set
#define MM_MSG_GEN_MPROP_SET            (0x44)
/// Generic Manufacturer Property Set Unacknowledged
#define MM_MSG_GEN_MPROP_SET_UNACK      (0x45)
/// Generic Manufacturer Property Status
#define MM_MSG_GEN_MPROP_STATUS         (0x46)

/// ***************** Message IDs for Generic Admin Property Model *******************

/// Generic Admin Properties Get
#define MM_MSG_GEN_APROPS_GET           (0x2C82)
/// Generic Admin Properties Status
#define MM_MSG_GEN_APROPS_STATUS        (0x47)
/// Generic Admin Property Get
#define MM_MSG_GEN_APROP_GET            (0x2D82)
/// Generic Admin Property Set
#define MM_MSG_GEN_APROP_SET            (0x48)
/// Generic Admin Property Set Unacknowledged
#define MM_MSG_GEN_APROP_SET_UNACK      (0x49)
/// Generic Admin Property Status
#define MM_MSG_GEN_APROP_STATUS         (0x4A)

/// ***************** Message IDs for Generic User Property Model *******************

/// Generic User Properties Get
#define MM_MSG_GEN_UPROPS_GET           (0x2E82)
/// Generic User Properties Status
#define MM_MSG_GEN_UPROPS_STATUS        (0x4B)
/// Generic User Property Get
#define MM_MSG_GEN_UPROP_GET            (0x2F82)
/// Generic User Property Set
#define MM_MSG_GEN_UPROP_SET            (0x4C)
/// Generic User Property Set Unacknowledged
#define MM_MSG_GEN_UPROP_SET_UNACK      (0x4D)
/// Generic User Property Status
#define MM_MSG_GEN_UPROP_STATUS         (0x4E)

/// ***************** Message IDs for Generic Client Property Model *******************

/// Generic Client Properties Get
#define MM_MSG_GEN_CPROPS_GET           (0x4F)
/// Generic Client Properties Status
#define MM_MSG_GEN_CPROPS_STATUS        (0x50)

/// ***************** Message IDs for Sensor Server Model *******************

/// Sensor Server Descriptor Get
#define MM_MSG_SEN_DESC_GET             (0x3082)
/// Sensor Server Descriptor Status
#define MM_MSG_SEN_DESC_STATUS          (0x51)
/// Sensor Server Get
#define MM_MSG_SEN_GET                  (0x3182)
/// Sensor Server Status
#define MM_MSG_SEN_STATUS               (0x52)
/// Sensor Server Column Get
#define MM_MSG_SEN_COL_GET              (0x3282)
/// Sensor Server Column
#define MM_MSG_SEN_COL_STATUS           (0x53)
/// Sensor Series Get
#define MM_MSG_SEN_SER_GET              (0x3382)
/// Sensor Series Status
#define MM_MSG_SEN_SER_STATUS           (0x54)

/// ***************** Message IDs for Sensor Server Setup Model *******************

/// Sensor Server Setup Cadence Get
#define MM_MSG_SENS_CAD_GET             (0x3482)
/// Sensor Server Setup Cadence Set
#define MM_MSG_SENS_CAD_SET             (0x55)
/// Sensor Server Setup Cadence Set Unacknowledged
#define MM_MSG_SENS_CAD_SET_UNACK       (0x56)
/// Sensor Server Setup Cadence Status
#define MM_MSG_SENS_CAD_STATUS          (0X57)
/// Sensor Server Settings Get
#define MM_MSG_SENS_SETS_GET            (0x3582)
/// Sensor Server Settings Status
#define MM_MSG_SENS_SETS_STATUS         (0x58)
/// Sensor Server Setting Get
#define MM_MSG_SENS_SET_GET             (0x3682)
/// Sensor Server Setting Set
#define MM_MSG_SENS_SET_SET             (0x59)
/// Sensor Server Setting Set Unacknowledged
#define MM_MSG_SENS_SET_SET_UNACK       (0x5A)
/// Sensor Server Setting Status
#define MM_MSG_SENS_SET_STATUS          (0x5B)

/// ***************** Message IDs for Sensor Server Time Model *******************

/// Time Server Get
#define MM_MSG_TIME_GET                 (0x3782)
/// Time Server Set
#define MM_MSG_TIME_SET                 (0x5C)
/// Time Server Status
#define MM_MSG_TIME_STATUS              (0x5D)
/// Time Server Role Get
#define MM_MSG_TIME_ROLE_GET            (0x3882)
/// Time Server Role Set
#define MM_MSG_TIME_ROLE_SET            (0x3982)
/// Time Server Role Status
#define MM_MSG_TIME_ROLE_STATUS         (0x3A82)
/// Time Server Zone Get
#define MM_MSG_TIME_ZONE_GET            (0x3B82)
/// Time Server Zone Set
#define MM_MSG_TIME_ZONE_SET            (0x3C82)
/// Time Server Zone Status
#define MM_MSG_TIME_ZONE_STATUS         (0x3D82)
/// Time Server TAI-UTC Delta Get
#define MM_MSG_TIME_DELTA_GET           (0x3E82)
/// Time Server TAI-UTC Delta Set
#define MM_MSG_TIME_DELTA_SET           (0x3F82)
/// Time Server TAI-UTC Delta Status
#define MM_MSG_TIME_DELTA_STATUS        (0x4082)

/// ***************** Message IDs for Scene Server Model *******************

/// Scene Server Get
#define MM_MSG_TSCNS_SCE_GET            (0x4182)
/// Scene Server Recall
#define MM_MSG_TSCNS_SCE_RECALL         (0x4282)
/// Scene Server Recall Unacknowledged
#define MM_MSG_TSCNS_SCE_RECALL_UNACK   (0x4382)
/// Scene Server Status
#define MM_MSG_TSCNS_SCE_STATUS         (0x5E)
/// Scene Server Register Get
#define MM_MSG_TSCNS_SCE_REG_GET        (0x4482)
/// Scene Server Register Status
#define MM_MSG_TSCNS_SCE_REG_STATUS     (0x4582)

/// ***************** Message IDs for Scene Server Setup Model *******************

/// Scene Server Setup Store
#define MM_MSG_TSCNS_SCES_STORE         (0x4682)
/// Scene Server Setup Store Unacknowledged
#define MM_MSG_TSCNS_SCES_STORE_UNACK   (0x4782)
/// Scene Server Setup Delete
#define MM_MSG_TSCNS_SCES_DEL           (0x9E82)
/// Scene Server Setup Delete Unacknowledged
#define MM_MSG_TSCNS_SCES_DEL_UNACK     (0x9F82)

/// ***************** Message IDs for Scheduler Server Model *******************

/// Scheduler Server Action Get
#define MM_MSG_SCH_ACT_GET              (0x4882)
/// Scheduler Server Action Status
#define MM_MSG_SCH_ACT_STATUS           (0x5F)
/// Scheduler Server Get
#define MM_MSG_SCH_GET                  (0x4982)
/// Scheduler Server Status
#define MM_MSG_SCH_STATUS               (0x4A82)

/// ***************** Message IDs for Scheduler Setup Server Model *******************

/// Scheduler Server Setup Action Set
#define MM_MSG_SCH_ACT_SET              (0x60)
/// Scheduler Server Setup Action Set Unacknowledged
#define MM_MSG_SCH_ACT_SET_UNACK        (0x61)

/// ***************** Message IDs for Light Lightness Model *******************

/// Light Lightness Get
#define MM_MSG_LIGHT_LN_GET             (0x4B82)
/// Light Lightness Set
#define MM_MSG_LIGHT_LN_SET             (0x4C82)
/// Light Lightness Set Unacknowledged
#define MM_MSG_LIGHT_LN_SET_UNACK       (0x4D82)
/// Light Lightness Status
#define MM_MSG_LIGHT_LN_STATUS          (0x4E82)
/// Light Lightness Linear Get
#define MM_MSG_LIGHT_LN_LIR_GET         (0x4F82)
/// Light Lightness Linear Set
#define MM_MSG_LIGHT_LN_LIR_SET         (0x5082)
/// Light Lightness Linear Set Unacknowledged
#define MM_MSG_LIGHT_LN_LIR_SET_UNACK   (0x5182)
/// Light Lightness Linear Status
#define MM_MSG_LIGHT_LN_LIR_STATUS      (0x5282)
/// Light Lightness Last Get
#define MM_MSG_LIGHT_LN_LST_GET         (0x5382)
/// Light Lightness Last Status
#define MM_MSG_LIGHT_LN_LST_STATUS      (0x5482)
/// Light Lightness Default Get
#define MM_MSG_LIGHT_LN_DEF_GET         (0x5582)
/// Light Lightness Default Status
#define MM_MSG_LIGHT_LN_DEF_STATUS      (0x5682)
/// Light Lightness Range Get
#define MM_MSG_LIGHT_LN_RAN_GET         (0x5782)
/// Light Lightness Range Status
#define MM_MSG_LIGHT_LN_RAN_STATUS      (0x5882)

/// ***************** Message IDs for Light Lightness Setup Model *******************

/// Light Lightness Default Set
#define MM_MSG_LIGHT_LN_DEF_SET         (0x5982)
/// Light Lightness Default Set Unacknowledged
#define MM_MSG_LIGHT_LN_DEF_SET_UNACK   (0x5A82)
/// Light Lightness Range Set
#define MM_MSG_LIGHT_LN_RAN_SET         (0x5B82)
/// Light Lightness Range Set Unacknowledged
#define MM_MSG_LIGHT_LN_RAN_SET_UNACK   (0x5C82)

/// ***************** Message IDs for Light CTL Model *******************

/// Light CTL Get
#define MM_MSG_LIGHT_CTL_GET            (0x5D82)
/// Light CTL Set
#define MM_MSG_LIGHT_CTL_SET            (0x5E82)
/// Light CTL Set Unacknowledged
#define MM_MSG_LIGHT_CTL_SET_UNACK      (0x5F82)
/// Light CTL Status
#define MM_MSG_LIGHT_CTL_STATUS         (0x6082)
/// Light CTL Temperature Get
#define MM_MSG_LIGHT_CTL_TEMP_GET       (0x6182)
/// Light CTL Temperature Range Get
#define MM_MSG_LIGHT_CTL_TEMP_RAN_GET   (0x6282)
/// Light CTL Temperature Range Status
#define MM_MSG_LIGHT_CTL_TEMP_RAN_STATUS (0x6382)
/// Light CTL Temperature Set
#define MM_MSG_LIGHT_CTL_TEMP_SET       (0x6482)
/// Light CTL Temperature Set Unacknowledged
#define MM_MSG_LIGHT_CTL_TEMP_SET_UNACK (0x6582)
/// Light CTL Temperature Status
#define MM_MSG_LIGHT_CTL_TEMP_STATUS    (0x6682)
/// Light CTL Default Get
#define MM_MSG_LIGHT_CTL_DEF_GET        (0x6782)
/// Light CTL Default Status
#define MM_MSG_LIGHT_CTL_DEF_STATUS     (0x6882)

/// ***************** Message IDs for Light CTL Setup Model *******************

/// Light CTL Default Set
#define MM_MSG_LIGHT_CTL_DEF_SET        (0x6982)
/// Light CTL Default Set Unacknowledged
#define MM_MSG_LIGHT_CTL_DEF_SET_UNACK  (0x6A82)
/// Light CTL Temperature Range Set
#define MM_MSG_LIGHT_CTL_TEMP_RAN_SET   (0x6B82)
/// Light CTL Temperature Range Set Unacknowledged
#define MM_MSG_LIGHT_CTL_TEMP_RAN_SET_UNACK (0x6C82)

/// ***************** Message IDs for Light HSL Model *******************

/// Light HSL Get
#define MM_MSG_LIGHT_HSL_GET            (0x6D82)
/// Light HSL Hue Get
#define MM_MSG_LIGHT_HSL_HUE_GET        (0x6E82)
/// Light HSL Hue Set
#define MM_MSG_LIGHT_HSL_HUE_SET        (0x6F82)
/// Light HSL Hue Set Unacknowledged
#define MM_MSG_LIGHT_HSL_HUE_SET_UNACK  (0x7082)
/// Light HSL Hue Status
#define MM_MSG_LIGHT_HSL_HUE_STATUS     (0x7182)
/// Light HSL Saturation Get
#define MM_MSG_LIGHT_SAT_GET            (0x7282)
/// Light HSL Saturation Set
#define MM_MSG_LIGHT_SAT_SET            (0x7382)
/// Light HSL Saturation Set Unacknowledged
#define MM_MSG_LIGHT_SAT_SET_UNACK      (0x7482)
/// Light HSL Saturation Status
#define MM_MSG_LIGHT_SAT_STATUS         (0x7582)
/// Light HSL Set
#define MM_MSG_LIGHT_HSL_SET            (0x7682)
/// Light HSL Set Unacknowledged
#define MM_MSG_LIGHT_HSL_SET_UNACK      (0x7782)
/// Light HSL Status
#define MM_MSG_LIGHT_HSL_STATUS         (0x7882)
/// Light HSL Target Get
#define MM_MSG_LIGHT_HSL_TAR_GET        (0x7982)
/// Light HSL Target Status
#define MM_MSG_LIGHT_HSL_TAR_STATUS     (0x7A82)
/// Light HSL Default Get
#define MM_MSG_LIGHT_HSL_DEF_GET        (0x7B82)
/// Light HSL Default Status
#define MM_MSG_LIGHT_HSL_DEF_STATUS     (0x7C82)
/// Light HSL Range Get
#define MM_MSG_LIGHT_HSL_RAN_GET        (0x7D82)
/// Light HSL Range Status
#define MM_MSG_LIGHT_HSL_RAN_STATUS     (0x7E82)

/// ***************** Message IDs for Light HSL Setup Model *******************

/// Light HSL Default Set
#define MM_MSG_LIGHT_HSL_DEF_SET        (0x7F82)
/// Light HSL Default Set Unacknowledged
#define MM_MSG_LIGHT_HSL_DEF_SET_UNACK  (0x8082)
/// Light HSL Range Set
#define MM_MSG_LIGHT_HSL_RAN_SET        (0x8182)
/// Light HSL Range Set Unacknowledged
#define MM_MSG_LIGHT_HSL_RAN_SET_UNACK  (0x8282)

/// ***************** Message IDs for Light xyL Model *******************

/// Light xyL Get
#define MM_MSG_LIGHT_XYL_GET            (0x8382)
/// Light xyL Set
#define MM_MSG_LIGHT_XYL_SET            (0x8482)
/// Light xyL Set Unacknowledged
#define MM_MSG_LIGHT_XYL_SET_UNACK      (0x8582)
/// Light xyL Status
#define MM_MSG_LIGHT_XYL_STATUS         (0x8682)
/// Light xyL Target Get
#define MM_MSG_LIGHT_XYL_TAR_GET        (0x8782)
/// Light xyL Target Status
#define MM_MSG_LIGHT_XYL_TAR_STATUS     (0x8882)
/// Light xyL Default Get
#define MM_MSG_LIGHT_XYL_DEF_GET        (0x8982)
/// Light xyL Default Status
#define MM_MSG_LIGHT_XYL_DEF_STATUS     (0x8A82)
/// Light xyL Range Get
#define MM_MSG_LIGHT_XYL_RAN_GET        (0x8B82)
/// Light xyL Range Status
#define MM_MSG_LIGTH_XYL_RAN_STATUS     (0x8C82)

/// ***************** Message IDs for Light xyL Setup Model *******************

/// Light xyL Default Set
#define MM_MSG_LIGHT_XYL_DEF_SET        (0x8D82)
/// Light xyL Default Set Unacknowledged
#define MM_MSG_LIGHT_XYL_DEF_SET_UNACK  (0x8E82)
/// Light xyL Range Set
#define MM_MSG_LIGHT_XYL_RAN_SET        (0x8F82)
/// Light xyL Range Set Unacknowledged
#define MM_MSG_LIGHT_XYL_RAN_SET_UNACK  (0x9082)

/// ***************** Message IDs for Light Control Model *******************

/// Light LC Mode Get
#define MM_MSG_LIGHT_LCM_GET            (0x9182)
/// Light LC Mode Set
#define MM_MSG_LIGHT_LCM_SET            (0x9282)
/// Light LC Mode Set Unacknowledged
#define MM_MSG_LIGHT_LCM_SET_UNACK      (0x9382)
/// Light LC Mode Status
#define MM_MSG_LIGHT_LCM_STATUS         (0x9482)
/// Light LC OM Get
#define MM_MSG_LIGHT_LCOM_GET           (0x9582)
/// Light LC OM Set
#define MM_MSG_LIGHT_LCOM_SET           (0x9682)
/// Light LC OM Set Unacknowledged
#define MM_MSG_LIGHT_LCOM_SET_UNACK     (0x9782)
/// Light LC OM Status
#define MM_MSG_LIGHT_LCOM_STATUS        (0x9882)
/// Light LC Light Onoff Get
#define MM_MSG_LIGHT_LC_LOO_GET         (0x9982)
/// Light LC Light Onoff Set
#define MM_MSG_LIGHT_LC_LOO_SET         (0x9A82)
/// Light LC Light Onoff Set Unacknowledged
#define MM_MSG_LIGHT_LC_LOO_SET_UNACK   (0x9B82)
/// Light LC Light Onoff Status
#define MM_MSG_LIGHT_LC_LOO_STATUS      (0x9C82)
/// Light LC Property Get
#define MM_MSG_LIGHT_LC_PROP_GET        (0x9D82)
/// Light LC Property Set
#define MM_MSG_LIGHT_LC_PROP_SET        (0x62)
/// Light LC Property Set Unacknowledged
#define MM_MSG_LIGHT_LC_PROP_SET_UNACK  (0x63)
/// Light LC Property Status
#define MM_MSG_LIGHT_LC_PROP_STATUS     (0x64)

/// ********************* Message IDs for Vendor Tmall Model ****************************

/// Vendor Tmall Attr Get
#define MM_MSG_VND_TMALL_GET            (0x01A8D0)
/// Vendor Tmall Attr Set
#define MM_MSG_VND_TMALL_SET            (0x01A8D1)
/// Vendor Tmall Attr Set Unacknowledged
#define MM_MSG_VND_TMALL_SET_UNACK      (0x01A8D2)
/// Vendor Tmall Attr Status
#define MM_MSG_VND_TMALL_STATUS         (0x01A8D3)
/// Vendor Tmall Attr Indication:   Client <- Server
#define MM_MSG_VND_TMALL_IND            (0x01A8D4)
/// Vendor Tmall Attr Confirmation: Client -> Server
#define MM_MSG_VND_TMALL_CFM            (0x01A8D5)
/// Vendor Tmall Transparent Message, i.e. Tunnel message
#define MM_MSG_VND_TMALL_TUN            (0x01A8CF)

/// @} MM_DEFINES

#endif /* MM_DEFINES_ */
