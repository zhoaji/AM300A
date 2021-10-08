/**
 * @file calib_repair.c
 * @brief 
 * @date Fri, Mar 16, 2018  6:09:52 PM
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
#include "peripheral.h"
#include "co.h"

#ifndef CONFIG_WITHOUT_RW
#include "bb.h"
#include "ll.h"
#include "lld_int.h"
#endif

/*********************************************************************
 * MACROS
 */
//{BASE_COMPONENT_BEGIN
#define RF_INVALID_TEMPERATURE  1000
#define XTAL32M_INVALID_CAP     0xFF
//}BASE_COMPONENT_END

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
calib_repair_t calib_repair_env =
{
    /* temperature              */ 20,

#ifdef CONFIG_BOOTLOADER
    /* rc_rf_repair_delay_s     */ 0, // also ctrl rf/rc repair enable/disable
#else
    /* rc_rf_repair_delay_s     */ 60,
#endif
    /* rc_rf_repair_time        */ CO_INVALID_SYS_TIME,
    /* rc_repair_temperature    */ RF_INVALID_TEMPERATURE,
    /* rf_repair_temperature    */ RF_INVALID_TEMPERATURE,

    /* rc32k_repair_delay_ms    */ 1000*60*1,
    /* rc32k_repair_time        */ CO_INVALID_SYS_TIME,

    /* xtal32m_cap              */ XTAL32M_INVALID_CAP,

    /* delta_dvdd_1p0           */ 0,
    /* delta_vdd_1p2            */ 0,
    /* delta_dcdc_1p27          */ 0,
    /* delta_vcharge_1p0        */ 0,
};
#else //}BASE_COMPONENT_END
extern calib_repair_t calib_repair_env;
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief  calib repair voltage temperature
 *
 * @param[in] t  temperature
 **/
static void calib_repair_voltage_temperature(int16_t t)
{
    uint32_t ldo_v1p2_vbat;
    uint32_t dcdc_vout;

    if (t > 70)
    {
        ldo_v1p2_vbat = 2;
        dcdc_vout = 6;
    }
    else
    {
        ldo_v1p2_vbat = 0;
        dcdc_vout = 4;
    }

#ifndef CONFIG_HS6621
    // use 8dBm TX mode
    if (REGR(&HS_PMU->ANA_REG, MASK_POS(PMU_ANA_LDO_V1P0_PA_VBAT)) == 3)
    {
        ldo_v1p2_vbat = 3;
        dcdc_vout = 7;
    }
#endif

    // LDO
    REGW(&HS_PMU->ANA_REG, MASK_1REG(PMU_ANA_LDO_V1P2_A_VBAT,
                calib_repair_value_select(ldo_v1p2_vbat+calib_repair_env.delta_vdd_1p2,0,3)));
    // DCDC
    REGW(&HS_PMU->ANA_PD_1, MASK_1REG(PMU_DCDC_VOUT,
                calib_repair_value_select(dcdc_vout+calib_repair_env.delta_dcdc_1p27,0,7)));
}

/**
 * @brief  calib repair xtal32m temperature
 *
 * @param[in] t  temperature
 **/
static void calib_repair_xtal32m_temperature(int16_t t)
{
    int cur_cap;
    int fixed_cap;

    if (calib_repair_env.xtal32m_cap <= 5)
        return;

    cur_cap = REGR(&HS_PMU->CLK_CTRL_2, MASK_POS(PMU_REG_CTUNE_XTAL));

    if (calib_repair_env.xtal32m_cap == XTAL32M_INVALID_CAP)
        calib_repair_env.xtal32m_cap = cur_cap;

    if (t > 70)
        fixed_cap = calib_repair_env.xtal32m_cap - 5;
    else
        fixed_cap = calib_repair_env.xtal32m_cap;

    if (fixed_cap != cur_cap)
        REGW(&HS_PMU->CLK_CTRL_2, MASK_1REG(PMU_REG_CTUNE_XTAL, fixed_cap));
}

/**
 * @brief rf temperature repair check
 **/
static void calib_repair_rc_rf_temperature(uint32_t cur_time)
{
    if(!adc_is_running())
    {
        int16_t t = (int16_t)adc_temperature_read(NULL);
        int16_t delta_t_rc = calib_repair_env.rc_repair_temperature - t;
        int16_t pre_all_repair_t = calib_repair_env.rf_repair_temperature;
        bool topclk_recalibed = false;

        calib_repair_env.temperature = t;
        calib_repair_env.rc_rf_repair_time = cur_time;

        // if delta-temperature>30C, re-calib rc
        if (delta_t_rc>30 || delta_t_rc<-30)
        {
            calib_repair_env.rc_repair_temperature = t;

            pmu_topclk_recalib();
            topclk_recalibed = true;
        }

        // re-calib all
        if ((pre_all_repair_t<60 && t>70) || (pre_all_repair_t>70 && t<60))
        {
            calib_repair_env.rc_repair_temperature = t;
            calib_repair_env.rf_repair_temperature = t;

            if(!topclk_recalibed)
                pmu_topclk_recalib();

            calib_repair_voltage_temperature(t);

            calib_rf();
        }

        // xtal32m
        calib_repair_xtal32m_temperature(t);
    }
}

/**
 * @brief  calib repair rc32k lpclk drift temperature
 **/
static void calib_repair_rc32k_drift_temperature(void)
{
#ifndef CONFIG_WITHOUT_RW 
#ifdef CONFIG_HS6621
    // temperature
    if (calib_repair_env.temperature < -10)
        sc.lpclk_drift = 5000;
    else
        sc.lpclk_drift = 500;
#else
    if (calib_repair_env.temperature < -20)
        sc.lpclk_drift = 2000; // 1200
    else
        sc.lpclk_drift = 1200; // 800

#ifdef CONFIG_LOCAL_DRIFT_DYNAMIC
    static struct lld_local_drift_dyn_table local_drift_dyn_table[2];

    // if > 200ms
    local_drift_dyn_table[0].greater_clock_diff = RWIP_1MS_TIME_TO_CLOCK(200);
    local_drift_dyn_table[0].drift = 500;

    // else use lld_env.local_drift
    local_drift_dyn_table[1].drift = 0;

    lld_env.local_drift_dyn_table = local_drift_dyn_table;
#endif
#endif

    lld_env.local_drift = sc.lpclk_drift;
#endif
}

/**
 * @brief rc32k temperature repair check
 **/
static void calib_repair_rc32k_temperature(uint32_t cur_time)
{
    // check and re-select (will trigger calib_rc32k)
    pmu_select_32k(PMU_32K_SEL_RC);

    // save
    calib_repair_env.rc32k_repair_time = cur_time;

    // temperature
    calib_repair_rc32k_drift_temperature();
}


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief  calib repair rc rf init
 **/
void calib_repair_rc_rf_init(void)
{
    int16_t t;

    if(0 == calib_repair_env.rc_rf_repair_delay_s)
        return;

#ifdef CONFIG_USE_BASE_COMPONENT_SYMBOL
    bc_env.dep_calib_repair_rc_rf_temperature = calib_repair_rc_rf_temperature;
    bc_env.dep_calib_repair_rc32k_temperature = calib_repair_rc32k_temperature;
#endif

    co_assert(!adc_is_running());

    t = (int16_t)adc_temperature_read(NULL);

    calib_repair_env.temperature = t;
    calib_repair_env.rf_repair_temperature = t;
    calib_repair_env.rc_repair_temperature = t;
    calib_repair_env.rc32k_repair_time = calib_repair_env.rc_rf_repair_time = co_time();

    // xtal32m repair
    calib_repair_xtal32m_temperature(t);

    // voltage repair
    calib_repair_voltage_temperature(t);

    // rc32k drift repair
    if(pmu_select_32k_get() == PMU_32K_SEL_RC)
        calib_repair_rc32k_drift_temperature();
}

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
/**
 * @brief rf temperature repair check
 **/
void calib_repair_rc_rf_temperature_check(void)
{
    uint32_t pre_time_tmp;

    if(0 == calib_repair_env.rc_rf_repair_delay_s)
        return;

    pre_time_tmp = calib_repair_env.rc_rf_repair_time;

    if(co_time_delay_ms_past(calib_repair_env.rc_rf_repair_delay_s*1000, &pre_time_tmp))
        calib_repair_rc_rf_temperature(pre_time_tmp);
}

/**
 * @brief rc32k temperature repair check
 **/
void calib_repair_rc32k_temperature_check(void)
{
    uint32_t pre_time_tmp;

    if (0 == calib_repair_env.rc32k_repair_delay_ms)
        return;

    if(pmu_select_32k_get() != PMU_32K_SEL_RC)
        return;

    pre_time_tmp = calib_repair_env.rc32k_repair_time;

    if(co_time_delay_ms_past(calib_repair_env.rc32k_repair_delay_ms, &pre_time_tmp)) // delay is pasted
        calib_repair_rc32k_temperature(pre_time_tmp);
}
#endif //}BASE_COMPONENT_END

/**
 * @brief set repair_rc32k_temperature_delay
 *
 * @param[in] delay_ms  
 *
 * @return None
 **/
void calib_repair_rc32k_temperature_delay_set(uint32_t delay_ms)
{
    calib_repair_env.rc32k_repair_delay_ms = delay_ms;
}

/**
 * @brief  calib repiar rc rf temperature delay set
 *
 * @param[in] delay_ms  delay ms
 **/
void calib_repiar_rc_rf_temperature_delay_set(uint32_t delay_ms)
{
    calib_repair_env.rc_rf_repair_delay_s = delay_ms / 1000;
}

/**
 * @brief  calib repiar sys voltage
 *
 * @param[in] dvdd_1p0  dvdd 1p0
 * @param[in] dcdc_1p27  dcdc 1p27
 * @param[in] vcharge_1p0  vcharge 1p0
 **/
void calib_repiar_sys_voltage_set(float dvdd_1p0, float dcdc_1p27, float vcharge_1p0)
{
    // dvdd, vdd
    if      (dvdd_1p0 < 0.75) { calib_repair_env.delta_dvdd_1p0 =  6; calib_repair_env.delta_vdd_1p2 =  2; }
    else if (dvdd_1p0 < 0.80) { calib_repair_env.delta_dvdd_1p0 =  5; calib_repair_env.delta_vdd_1p2 =  2; }
    else if (dvdd_1p0 < 0.85) { calib_repair_env.delta_dvdd_1p0 =  4; calib_repair_env.delta_vdd_1p2 =  1; }
    else if (dvdd_1p0 < 0.90) { calib_repair_env.delta_dvdd_1p0 =  3; calib_repair_env.delta_vdd_1p2 =  1; }
    else if (dvdd_1p0 < 0.95) { calib_repair_env.delta_dvdd_1p0 =  2; calib_repair_env.delta_vdd_1p2 =  1; }
    else if (dvdd_1p0 < 1.00) { calib_repair_env.delta_dvdd_1p0 =  1; calib_repair_env.delta_vdd_1p2 =  0; }
    else if (dvdd_1p0 < 1.05) { calib_repair_env.delta_dvdd_1p0 =  0; calib_repair_env.delta_vdd_1p2 =  0; }
    else if (dvdd_1p0 < 1.10) { calib_repair_env.delta_dvdd_1p0 = -1; calib_repair_env.delta_vdd_1p2 = -1; }
    else if (dvdd_1p0 < 1.15) { calib_repair_env.delta_dvdd_1p0 = -2; calib_repair_env.delta_vdd_1p2 = -1; }
    else if (dvdd_1p0 < 1.20) { calib_repair_env.delta_dvdd_1p0 = -3; calib_repair_env.delta_vdd_1p2 = -1; }
    else if (dvdd_1p0 < 1.25) { calib_repair_env.delta_dvdd_1p0 = -4; calib_repair_env.delta_vdd_1p2 = -2; }
    else if (dvdd_1p0 < 1.30) { calib_repair_env.delta_dvdd_1p0 = -5; calib_repair_env.delta_vdd_1p2 = -2; }
    else                      { calib_repair_env.delta_dvdd_1p0 = -6; calib_repair_env.delta_vdd_1p2 = -2; }

    // dcdc
    if      (dcdc_1p27 < 1.15) calib_repair_env.delta_dcdc_1p27 =  3;
    else if (dcdc_1p27 < 1.21) calib_repair_env.delta_dcdc_1p27 =  2;
    else if (dcdc_1p27 < 1.27) calib_repair_env.delta_dcdc_1p27 =  1;
    else if (dcdc_1p27 < 1.33) calib_repair_env.delta_dcdc_1p27 =  0;
    else if (dcdc_1p27 < 1.39) calib_repair_env.delta_dcdc_1p27 = -1;
    else if (dcdc_1p27 < 1.45) calib_repair_env.delta_dcdc_1p27 = -2;
    else                       calib_repair_env.delta_dcdc_1p27 = -3;

    // vcharge
    calib_repair_env.delta_vcharge_1p0 = (1000 - (int)(vcharge_1p0*1000)) / 4;
}

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
/**
 * @brief  calib repair value select
 *
 * @param[in] calc  calc
 * @param[in] min  min
 * @param[in] max  max
 *
 * @return select
 **/
int calib_repair_value_select(int calc, int min, int max)
{
    if (calc < min)
        calc = min;
    else if (calc > max)
        calc = max;
    return calc;
}
#endif //}BASE_COMPONENT_END

/** @} */

