/**
 * @file kpp_sim.c
 * @brief 
 * @date Thu 02 Nov 2017 04:41:38 PM CST
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
#include "kpp_sim.h"

/*********************************************************************
 * MACROS
 */

/// keyboard pin max num
#define KPP_SIM_PIN_NUM                       31

#define KPP_SIM_RELEASE_COUNT_MAX             3

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    kpp_sim_config_t config;
    bool is_pressed;
    bool is_report_pressed;
    uint8_t release_count;
    kpp_data_t data;
    co_timer_t scan_timer;
    co_timer_t debounce_timer;
}kpp_sim_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static kpp_sim_env_t kpp_sim_env;

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief kpp_is_ghost_key()
 *
 * @param[in] data  
 *
 * @return 
 **/
static bool kpp_sim_is_ghost_key(const kpp_sim_data_t data)
{
    int i, j;
    uint8_t ghost;

    for(i=0; i<sizeof(kpp_sim_data_t); ++i)
    {
        if(data[i] == 0)
            continue;

        for(j=i+1; j<sizeof(kpp_sim_data_t); ++j)
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
 * @brief kpp_sim_depressed()
 *
 * @return 
 **/
static void kpp_sim_depressed(const kpp_data_t data)
{
    if(memcmp(kpp_sim_env.data, data, sizeof(kpp_data_t)))
    {
        kpp_sim_env.is_report_pressed = true;

        memcpy(kpp_sim_env.data, data, sizeof(kpp_data_t));

        if(kpp_sim_env.config.callback && !kpp_sim_is_ghost_key(data))
            kpp_sim_env.config.callback(KPP_SIM_EVENT_DEPRESS, data);
    }
}

/**
 * @brief kpp_sim_released()
 *
 * @return 
 **/
static void kpp_sim_released(void)
{
    if(kpp_sim_env.is_report_pressed)
    {
        kpp_sim_env.is_report_pressed = false;

        if(kpp_sim_env.config.callback)
            kpp_sim_env.config.callback(KPP_SIM_EVENT_RELEASE, NULL);

        memset(kpp_sim_env.data, 0, sizeof(kpp_data_t));
    }
}

/**
 * @brief kpp_sim_scan_row()
 *
 * @return 
 **/
static void kpp_sim_scan_row(void)
{
    uint32_t col, col_mask, col_index;
    uint32_t row, row_mask, row_index, row_value;
    bool depressed = false;
    kpp_data_t data = {0};
    uint32_t row_pin_mask = kpp_sim_env.config.row_pin_mask;
    uint32_t col_pin_mask = kpp_sim_env.config.col_pin_mask;
    uint32_t col_output_delay = kpp_sim_env.config.col_output_delay;

    // Disable IRQ
    gpio_set_interrupt(row_pin_mask, GPIO_TRIGGER_DISABLE);

    // All colume to HIGH
    pmu_pin_mode_set(col_pin_mask, PMU_PIN_MODE_PU);
    gpio_write(col_pin_mask, GPIO_HIGH);
    co_delay_us(col_output_delay);
    pmu_pin_mode_set(col_pin_mask, PMU_PIN_MODE_OD);

    // Scan
    col_index = 0;
    for(col=0; col<KPP_SIM_PIN_NUM; ++col)
    {
        col_mask = 1u << col;
        if(col_pin_mask & col_mask)
        {
            // Only current colume output LOW
            gpio_write(col_mask, GPIO_LOW);
            co_delay_us(col_output_delay);

            // Read value
            row_value = (~gpio_read(row_pin_mask)) & row_pin_mask;

            // Current colume to HIGH
            pmu_pin_mode_set(col_pin_mask, PMU_PIN_MODE_PU);
            gpio_write(col_pin_mask, GPIO_HIGH);

            // Check ROW
            row_index = 0;
            for(row=0; row<KPP_SIM_PIN_NUM; ++row)
            {
                row_mask = 1u << row;
                if(row_pin_mask & row_mask)
                {
                    if(row_value & row_mask)
                        data[col_index] |= 1u << row_index;

                    ++row_index;
                    if(row_index >= KPP_SIM_ROW_NUM)
                        break;
                }
            }

            if(data[col_index])
                depressed = true;

            // restore OD
            pmu_pin_mode_set(col_pin_mask, PMU_PIN_MODE_OD);

            ++col_index;
            if(col_index >= KPP_SIM_COL_NUM)
                break;
        }
    }

    // All colume to LOW
    gpio_write(col_pin_mask, GPIO_LOW);
    co_delay_us(col_output_delay+60);

    // Enable IRQ
    gpio_set_interrupt(row_pin_mask, GPIO_BOTH_EDGE);

    // May invalid IRQ
    if(depressed)
        kpp_sim_depressed(data);
}

/**
 * @brief kpp_scan_timer_handler()
 *
 * @param[in] id  
 * @param[in] param  
 *
 * @return 
 **/
static void kpp_sim_scan_timer_handler(co_timer_t *timer, void *param)
{
    uint32_t cur_row_level_mask = gpio_read(kpp_sim_env.config.row_pin_mask);

    if(cur_row_level_mask == kpp_sim_env.config.row_pin_mask)
        ++kpp_sim_env.release_count;
    else
        kpp_sim_env.release_count = 0;

    if(kpp_sim_env.release_count < KPP_SIM_RELEASE_COUNT_MAX)
    {
        kpp_sim_scan_row();
    }
    else
    {
        CO_DISABLE_IRQ();
        kpp_sim_env.is_pressed = false;
        CO_RESTORE_IRQ();

        kpp_sim_env.release_count = 0;

        co_timer_del(&kpp_sim_env.scan_timer);

        pmu_wakeup_pin_set(kpp_sim_env.config.row_pin_mask, PMU_PIN_WAKEUP_LOW_LEVEL);

        kpp_sim_released();
    }
}

/**
 * @brief kpp_sim_debounce_timer_handler()
 *
 * @param[in] id  
 * @param[in] param  
 *
 * @return 
 **/
static void kpp_sim_debounce_timer_handler(co_timer_t *timer, void *param)
{
    uint32_t cur_row_level_mask = gpio_read(kpp_sim_env.config.row_pin_mask);

    if(cur_row_level_mask != kpp_sim_env.config.row_pin_mask)
    {
        CO_DISABLE_IRQ();
        kpp_sim_env.is_pressed = true;
        CO_RESTORE_IRQ();

        co_timer_set(&kpp_sim_env.scan_timer, kpp_sim_env.config.scan_interval,
                TIMER_REPEAT, kpp_sim_scan_timer_handler, NULL);

        kpp_sim_scan_row();

        pmu_wakeup_pin_set(kpp_sim_env.config.row_pin_mask, PMU_PIN_WAKEUP_DISABLE);
    }

    pmu_lowpower_allow(PMU_LP_KPP);
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief encoder gpio IRQ handler, PUT THIS TO GPIO IRQ HANDLER
 *
 * @param[in] pin_mask  pin mask
 *
 * @return None
 **/
void kpp_sim_gpio_handler(uint32_t pin_mask)
{
    // depress
    if(!kpp_sim_env.is_pressed)
    {
        uint32_t cur_row_level_mask = gpio_read(kpp_sim_env.config.row_pin_mask);

        if(cur_row_level_mask != kpp_sim_env.config.row_pin_mask)
        {
            co_timer_set(&kpp_sim_env.debounce_timer, kpp_sim_env.config.scan_interval,
                    TIMER_ONE_SHOT, kpp_sim_debounce_timer_handler, NULL);

            pmu_lowpower_prevent(PMU_LP_KPP);
        }
    }
}

/**
 * @brief Keyboad config
 *
 * @param[in] config  configuration @ref kpp_sim_config_t
 *
 * @return None
 **/
void kpp_sim_config(const kpp_sim_config_t *config)
{
    // Init env
    kpp_sim_env.config = *config;
    kpp_sim_env.is_pressed = false;
    kpp_sim_env.is_report_pressed = false;
    kpp_sim_env.release_count = 0;
    memset(kpp_sim_env.data, 0, sizeof(kpp_data_t));

    // Colume (open-drain)
    gpio_set_direction(config->col_pin_mask, GPIO_OUTPUT);
    gpio_write(config->col_pin_mask, GPIO_LOW);

    // Row (pull-up)
    gpio_set_direction(config->row_pin_mask, GPIO_INPUT);
    gpio_set_interrupt(config->row_pin_mask, GPIO_BOTH_EDGE);

    // wakeup pin
    pmu_wakeup_pin_set(kpp_sim_env.config.row_pin_mask, PMU_PIN_WAKEUP_LOW_LEVEL);

    // Pin Mode
    pmu_pin_mode_set(kpp_sim_env.config.col_pin_mask, PMU_PIN_MODE_OD);
    pmu_pin_mode_set(kpp_sim_env.config.row_pin_mask, PMU_PIN_MODE_PU);
}

/** @} */


