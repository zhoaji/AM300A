/**
 * @file pinmux.c
 * @brief 
 * @date Thu 27 Apr 2017 07:21:13 PM CST
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
#include "hs66xx.h"
#include "pinmux.h"

/*********************************************************************
 * MACROS
 */
#ifdef CONFIG_HS6621
#define PINMUX_PIN_NUM 40
#else
#define PINMUX_PIN_NUM 32
#endif

#define PINMUX_REG_NUM (PINMUX_PIN_NUM/4 + 0)

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    uint32_t reg_save[PINMUX_REG_NUM];
}pinmux_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
#ifdef CONFIG_HS6621
static pinmux_env_t pinmux_env = {{0x0, 0x0, 0x0, 0x1a000000, 0x1a1a1a1a, 0x1a1a1a1a, 0x1a1a1a1a, 0x1a1a1a, 0x0, 0x0}};
#else
static pinmux_env_t pinmux_env = {{0x0, 0x0, 0x0, 0x1c000000, 0x1c1c1c1c, 0x1c1c1c1c, 0x1c1c1c1c, 0x1c1c1c}};
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief pinmux config
 *
 * @param[in] pin  pin: from 0 to 30
 * @param[in] func  pin function
 *
 * @return None
 **/
void pinmux_config(uint32_t pin, pinmux_t func)
{
    uint32_t i, p;

    if(pin >= PINMUX_PIN_NUM)
        return;

    // Index and place
    i = pin/4;
    p = (pin % 4) * 8;

    if(func > PINMUX_ANALOG_BEGIN_CFG)
        func = PINMUX_GPIO_MODE_CFG;

    if(func > PINMUX_MULTIPLEX_BEGIN_CFG)
        func -= (int)PINMUX_MULTIPLEX_BEGIN_CFG;

    register_set(&HS_SYS->PINMUX[i], SYS_PINMUX_MASK(func, p));

    // Save it for sleep
    pinmux_env.reg_save[i] = HS_SYS->PINMUX[i];
}

/**
 * @brief  pinmux search
 *
 * @param[in] func  func
 *
 * @return
 **/
int pinmux_search(pinmux_t func)
{
    int i;
    uint8_t *p = (uint8_t *)HS_SYS->PINMUX;

    if(func > PINMUX_ANALOG_BEGIN_CFG)
        func = PINMUX_GPIO_MODE_CFG;

    if(func > PINMUX_MULTIPLEX_BEGIN_CFG)
        func -= (int)PINMUX_MULTIPLEX_BEGIN_CFG;

    for (i=0; i<PINMUX_PIN_NUM; ++i)
    {
        if ((p[i]&0x7F) == (uint8_t)func)
            return i;
    }

    return -1;
}

/**
 * @brief pinmux restore
 *
 * Just for system call after sleep
 *
 * @return None
 **/
void pinmux_restore(void)
{
    int i;
    for(i=1; i<PINMUX_REG_NUM; ++i)
        HS_SYS->PINMUX[i] = pinmux_env.reg_save[i];

    // pinmux[0] include GATE_CPUCLK flag
    HS_SYS->PINMUX[0] = (HS_SYS->PINMUX[0] & SYS_PINMUX_SYSPLL_GT_CPUCLK_HW_CTRL_MASK) | pinmux_env.reg_save[0];
}

/** @} */


