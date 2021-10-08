/**
 * @file kpp.c
 * @brief 
 * @date Wed 31 May 2017 07:15:36 PM CST
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

/*********************************************************************
 * MACROS
 */
#define KPP_COL_MASK                      0x0003FFFF
#define KPP_ROW_MASK                      0x000000FF

/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
    KPP_ROW_SCAN_COL_SEL = 0,
    KPP_COL_SCAN_ROW_SEL = 1,
}kpp_scan_mode_t;

typedef struct
{
    kpp_event_callback_t callback;
    uint16_t col_output_delay;
    uint16_t scan_interval;
    kpp_data_t data;
    bool depressed;
    co_timer_t timer;
} kpp_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static kpp_env_t kpp_env;

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief kpp_scan_timer_handler()
 *
 * @param[in] id  
 * @param[in] param  
 *
 * @return 
 **/
static void kpp_scan_timer_handler(co_timer_t *timer, void *param)
{
    register_set(&HS_KPP->KPSR, MASK_1REG(KPP_DEPRESS_INT_EN, 1));
}

/**
 * @brief kpp_is_ghost_key()
 *
 * @param[in] data  
 *
 * @return 
 **/
static bool kpp_is_ghost_key(const kpp_data_t data)
{
    int i, j;
    uint8_t ghost;

    for(i=0; i<sizeof(kpp_data_t); ++i)
    {
        if(data[i] == 0)
            continue;

        for(j=i+1; j<sizeof(kpp_data_t); ++j)
        {
            if(data[j] == 0)
                continue;

            ghost = data[i] & data[j];

            if(ghost && (ghost & (ghost - 1)))
                return true;
        }
    }

    return false;
}

/**
 * @brief kpp_depressed()
 *
 * @return 
 **/
static void kpp_depressed(const kpp_data_t data)
{
    bool report = true;

    kpp_env.depressed = true;

    if(kpp_env.scan_interval)
    {
        if(memcmp(kpp_env.data, data, KPP_COL_NUM)==0)
            report = false;
        else
            memcpy(kpp_env.data, data, KPP_COL_NUM);

        co_timer_set(&kpp_env.timer, kpp_env.scan_interval, TIMER_REPEAT, kpp_scan_timer_handler, NULL);
    }

    if(report && kpp_env.callback && !kpp_is_ghost_key(data))
        kpp_env.callback(KPP_EVENT_DEPRESS, data);

    pmu_lowpower_prevent(PMU_LP_KPP);
}

/**
 * @brief kpp_released()
 *
 * @return 
 **/
static void kpp_released(void)
{
    if(kpp_env.depressed)
    {
        kpp_env.depressed = false;

        if(kpp_env.scan_interval)
            co_timer_del(&kpp_env.timer);

        if(kpp_env.callback)
            kpp_env.callback(KPP_EVENT_RELEASE, NULL);

        memset(kpp_env.data, 0, sizeof(kpp_data_t));

        pmu_lowpower_allow(PMU_LP_KPP);
    }
}

/**
 * @brief kpp_scan_row()
 *
 * @return 
 **/
static void kpp_scan_row(void)
{
    uint32_t col, col_mask, row_value;
    uint32_t row_en_mask, col_en_mask;
    bool depressed = false;
    kpp_data_t data;
    uint32_t col_output_delay = kpp_env.col_output_delay;
    int i;

    const uint8_t io_table[] = {
        0, 1, 2, 3, 15, 16, 17, 18,
        19, 20, 21, 22, 23, 24, 25, 26,
        27, 28};
    uint32_t io_mask = 0;

    row_en_mask = register_get(&HS_KPP->KPCR, MASK_POS(KPP_ROW_EN));
    col_en_mask = register_get(&HS_KPP->KPCR, MASK_POS(KPP_COL_EN));

    register_set(&HS_KPP->KPSR, MASK_2REG(KPP_RELEASE_INT_EN, 0,
                                          KPP_DEPRESS_INT_EN, 0));

    for(i=0; i<KPP_COL_NUM && (col_en_mask & (1u<<i)); ++i)
        io_mask |= 1u<<io_table[i];

    pmu_pin_mode_set(io_mask, PMU_PIN_MODE_PU);
    HS_KPP->KPDR = KPP_COL_DATA_MASK | KPP_ROW_DATA_MASK;
    co_delay_us(col_output_delay);
    pmu_pin_mode_set(io_mask, PMU_PIN_MODE_OD);

    // scan
    for(col=0; col<KPP_COL_NUM; ++col)
    {
        col_mask = 1U<<col;
        if(col_en_mask & col_mask)
        {
            // only current col output 0
            register_set(&HS_KPP->KPDR, MASK_1REG(KPP_COL_DATA, KPP_COL_MASK & (~col_mask)));
            co_delay_us(col_output_delay);

//          if((HS_KPP->KPSR & KPP_DEPRESS_STATUS_MASK) == 0)
//              continue;

            row_value = register_get(&HS_KPP->KPDR, MASK_POS(KPP_ROW_DATA));

            row_value = (~row_value) & row_en_mask;

            data[col] = row_value;

            if(row_value)
                depressed = true;

            pmu_pin_mode_set(io_mask, PMU_PIN_MODE_PU);
            HS_KPP->KPDR = KPP_COL_DATA_MASK | KPP_ROW_DATA_MASK;
            co_delay_us(col_output_delay);
            pmu_pin_mode_set(io_mask, PMU_PIN_MODE_OD);
        }
        else
        {
            data[col] = 0;
        }
    }

    // Output 0
    HS_KPP->KPDR = 0;
    co_delay_us(col_output_delay+60);

    register_set(&HS_KPP->KPSR, MASK_2REG(KPP_RELEASE_INT_EN, 1,
                                          KPP_DEPRESS_INT_EN, kpp_env.scan_interval ? 0 : 1));

    // May invalid IRQ
    if(depressed)
        kpp_depressed(data);
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief Keyboad initialize
 *
 * @return None
 **/
void kpp_init(void)
{
    // Reset and Open clock
    register_set1(&HS_PSO->KPP_CFG, CPM_KPP_SOFT_RESET_MASK);
    register_set0(&HS_PSO->KPP_CFG, CPM_KPP_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    kpp_env.col_output_delay = 100;
    kpp_env.callback = NULL;
    kpp_env.depressed = false;
}

/**
 * @brief Keyboad config
 *
 * @param[in] config  configuration @ref kpp_config_t
 *
 * @return None
 **/
void kpp_config(const kpp_config_t *config)
{
    // Init
    kpp_env.col_output_delay = config->col_output_delay;
    kpp_env.scan_interval = config->scan_interval;
    kpp_env.callback = config->callback;

    // debounce delay=scan_interval
    register_set(&HS_PSO->KPP_CFG, MASK_4REG(CPM_KPP_DIV_COEFF, config->scan_interval*1000/281,
                                             CPM_KPP_DIV_SEL,   1,
                                             CPM_KPP_DIV_EN,    1,
                                             CPM_KPP_GATE_EN,   0));
    HS_PSO_UPD_RDY();

    // output 0
    HS_KPP->KPDR = 0;

    // enable col and row
    register_set(&HS_KPP->KPCR, MASK_2REG(KPP_COL_EN, config->col_mask,
                                          KPP_ROW_EN, config->row_mask));

    // dir and mode
    register_set(&HS_KPP->KDDR, MASK_3REG(KPP_SCAN_MODE, KPP_ROW_SCAN_COL_SEL,
                                          KPP_COL_DIR, KPP_COL_MASK, // 1:output 0:input
                                          KPP_ROW_DIR, 0));

    // Disable int
    register_set(&HS_KPP->KPSR, MASK_4REG(KPP_DEPRESS_SYNC_CLEAR, 1,
                                          KPP_DEPRESS_STATUS, 0,
                                          KPP_RELEASE_INT_EN, 0,
                                          KPP_DEPRESS_INT_EN, 0));

    // Enable IRQ
    NVIC_ClearPendingIRQ(KPP_DEPRESS_IRQn);
    NVIC_ClearPendingIRQ(KPP_RELEASE_IRQn);
    NVIC_SetPriority(KPP_DEPRESS_IRQn, IRQ_PRIORITY_LOW);
    NVIC_SetPriority(KPP_RELEASE_IRQn, IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(KPP_DEPRESS_IRQn);
    NVIC_EnableIRQ(KPP_RELEASE_IRQn);
}

/**
 * @brief Keyboad start
 *
 * @return None
 **/
void kpp_start(void)
{
    // enable int
    register_set(&HS_KPP->KPSR, MASK_4REG(KPP_DEPRESS_SYNC_CLEAR, 1,
                                          KPP_DEPRESS_STATUS, 0,
                                          KPP_RELEASE_INT_EN, 0,
                                          KPP_DEPRESS_INT_EN, 1));
}

/**
 * @brief Keyboad stop
 *
 * @return None
 **/
void kpp_stop(void)
{
    register_set(&HS_KPP->KPSR, MASK_2REG(KPP_RELEASE_INT_EN, 0,
                                          KPP_DEPRESS_INT_EN, 0));
}

/**
 * @brief KPP_DEPRESS_IRQHandler, the KPKD bit in the register KPSR
 *        will be automatically cleared when there is no key is pressed.
 *
 * @return NONE
 **/
void KPP_DEPRESS_IRQHandler(void)
{
    kpp_scan_row();
}

void KPP_RELEASE_IRQHandler(void)
{
    register_set(&HS_KPP->KPSR, MASK_2REG(KPP_RELEASE_INT_EN, 0,
                                          KPP_DEPRESS_INT_EN, 1));
    kpp_released();
}

/** @} */


