/*********************************************************************
 * @file cpm.c
 * @brief Clock & Power Module's low level routine
 * @version 1.0
 * @date Wed 19 Nov 2014 04:11:47 PM CST
 * @author liqiang
 *
 * @note 
 */

/*********************************************************************
 * INCLUDES
 */
#include "hs66xx.h"
#include "cpm.h"


/*********************************************************************
 * MACROS
 */

#ifdef CONFIG_HS6621
#define CPM_CPU_CFG_CHANGE_BEGIN()  CO_DISABLE_IRQ(); REGW0(HS_SYS->PINMUX, SYS_PINMUX_STAND_BY_SEL_MASK)
#define CPM_CPU_CFG_CHANGE_END()    cpm_wait_clock_align(); CO_RESTORE_IRQ()
#endif

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    struct
    {
        struct
        {
            uint32_t CPU_CFG;
            uint32_t RAM_CFG;
        }store_reg;
    }lowpower;
}cpm_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static __IO uint32_t * const cpm_clock_cfg_table[] =
{
    /* CPM_TOP_CLK     */ NULL,
    /* CPM_CPU_CLK     */ &HS_PSO->CPU_CFG,
    /* CPM_AHB_CLK     */ &HS_PSO->CPU_CFG,
    /* CPM_APB_CLK     */ &HS_PSO->CPU_CFG,
    /* CPM_ULP_RAM_CLK */ &HS_PSO->CPU_CFG,
    /* CPM_LP_RAM_CLK  */ &HS_PSO->CPU_CFG,
    /* CPM_ROM_CLK     */ &HS_PSO->CPU_CFG,
    /* CPM_SF0_CLK     */ &HS_PSO->SF0_CFG,
    /* CPM_SF1_CLK     */ &HS_PSO->SF1_CFG,
    /* CPM_TIM0_CLK    */ &HS_PSO->TIM_CFG[0],
    /* CPM_TIM1_CLK    */ &HS_PSO->TIM_CFG[1],
    /* CPM_TIM2_CLK    */ &HS_PSO->TIM_CFG[2],
    /* CPM_UART0_CLK   */ &HS_PSO->UART0_CFG,
    /* CPM_UART1_CLK   */ &HS_PSO->UART1_CFG,
    /* CPM_I2C_CLK     */ &HS_PSO->I2C_CFG,
    /* CPM_I2C1_CLK    */ &HS_PSO->I2C1_CFG,
    /* CPM_I2C2_CLK    */ &HS_PSO->I2C2_CFG,
    /* CPM_SPI_CLK     */ &HS_PSO->CPU_CFG, // with APB clock
    /* CPM_RTC_CLK     */ NULL, // Aways 32KHz
    /* CPM_WDT_CLK     */ NULL, // Aways 32KHz
    /* CPM_QDEC_CLK    */ &HS_PSO->QDEC_CFG,
    /* CPM_KPP_CLK     */ &HS_PSO->KPP_CFG,
    /* CPM_I2S_CLK     */ &HS_PSO->I2S_CFG,
    /* CPM_AUDIO_CLK   */ &HS_PSO->AUDIO_CFG,
#ifdef CONFIG_HS6621C
    /* CPM_SF2_CLK     */ &HS_PSO->SF2_CFG,
#endif
};

static cpm_env_t cpm_env;

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief cpm_get_top_clock()
 *
 * @return 
 **/
static uint32_t cpm_get_top_clock(void)
{
    uint32_t clk;

    if(HS_PMU->MISC_CTRL & PMU_MISC_CLK_64M_SEL_MASK)
    {
#ifdef CONFIG_HS6621
        CPM_ANA_CLK_ENABLE();

        uint32_t selclk = REGR(&HS_DAIF->SYSPLL_CNS1, MASK_POS(DAIF_SEL_CPUCLK));
        if(selclk == 0)
        {
            clk = 64000000; // RC32M*2 or XTAL32M*2
        }
        else
        {
            uint32_t vcodiv = selclk==1 ? 1 : 2;
            uint32_t selfreq = REGR(&HS_DAIF->SYSPLL_CNS0, MASK_POS(DAIF_SYSPLL_SEL_FREQ));
            clk = 16000000 * (5 + selfreq) / vcodiv; // PLL
        }

        CPM_ANA_CLK_RESTORE();
#else
        if (HS_PMU->XTAL32M_CNS0 & PMU_XTAL32M_SEL_CPUCLK_MASK)
            clk = 64000000; // RC32M*2 or XTAL32M*2
        else
            clk = 32000000; // XTAL32M
#endif
    }
    else
    {
        clk = 32000000; // RC32M
    }

    return clk;
}

#ifdef CONFIG_HS6621
/**
 * @brief cpm_wait_clock_align()
 *
 * @return 
 **/
static void cpm_wait_clock_align(void)
{
    uint32_t __nvic_iser0, __nvic_iser1;

    // stote
    __nvic_iser0 = NVIC->ISER[0];
    __nvic_iser1 = NVIC->ISER[1];

    // disable all irq
    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ICER[1] = 0xFFFFFFFF;

    // enable CPM irq
    NVIC_ClearPendingIRQ(CPM_IRQn);
    NVIC_EnableIRQ(CPM_IRQn);

    // Only CPU gate mode
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP;

    // param
    REGW(&HS_PSO->AHB_CFG, MASK_1REG(CPM_MCU_WAKEUP_CYCLE, 0x3F));

    // SWITCH
    REGW1(&HS_PSO->CPU_CFG, CPM_CPU_COEFF_CHANGE_MASK | CPM_CPU_INT_MASK_MASK);
    while(!(HS_PSO->CPU_CFG & CPM_CPU_COEFF_CHANGE_MASK));
    __WFI();
    co_assert(HS_PSO->CPU_CFG & CPM_CPU_WAKEUP_INT_STATUS_MASK);
    REGW0(&HS_PSO->CPU_CFG, CPM_CPU_COEFF_CHANGE_MASK | CPM_CPU_INT_MASK_MASK);

    // disable CPM irq
    NVIC_ClearPendingIRQ(CPM_IRQn); // Clear M3 pending (this pending can be self-cleared when IRQ-handler is processed)
    NVIC_DisableIRQ(CPM_IRQn);

    // restore
    NVIC->ISER[0] = __nvic_iser0;
    NVIC->ISER[1] = __nvic_iser1;
}

/**
 * @brief Must be in ROM
 *
 * @param[in] cfg  
 *
 * @return 
 **/
static __attribute__((noinline)) void cpm_cpu_cfg_set(uint32_t cfg)
{
    CPM_CPU_CFG_CHANGE_BEGIN();
    HS_PSO->CPU_CFG = cfg & (~CPM_CPU_COEFF_CHANGE_MASK); // make sure CHANGE=0
    CPM_CPU_CFG_CHANGE_END();
}

#else

/**
 * @brief  cpm cpu cfg set
 *
 * @param[in] cfg  cfg
 **/
static void cpm_cpu_cfg_set(uint32_t cfg)
{
    HS_PSO->CPU_CFG = cfg;
}
#endif

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief cpm_init()
 *
 * Don't use global variable
 *
 * @return 
 **/
void cpm_init(void)
{
#if 0
    __IO uint32_t cpu_cfg = 0;

    //
    // ROM clock must <= 102MHz
    // ULP SRAM clock must <= 32MHz
    //
    // cpu=32MHz fram=32MHz lram=32MHz
    register_set_raw(&cpu_cfg, MASK_11REG(
                CPM_AHB_DIV_COEFF, 0,       // default:0 ULP-RAM: LOW_RAM_CLK=(TOP_CLK/CPM_CPU_DIV_COEFF/CPM_AHB_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden, Max:36MHz
                CPM_AHB_CLK_PERIPH_EN, 1,   // default:1
                CPM_AHB_CLK_RAM_EN, 1,      // default:1
                CPM_APB_DIV_COEFF, 0,       // default:2 APB: 0:same as CPU
                CPM_CPU_DIV_COEFF, 2,       // default:2 CPU,LP-RAM: FAST-RAM=32MHz CPU_CLK=FAST_RAM_CLK=(TOP_CLK/CPM_CPU_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden
                CPM_CPU_INT_MASK, 1,        // Enable CPM int
                CPM_CPU_COEFF_CHANGE, 1,    // Enable CPM interrupt and disable other interrupt
                CPM_AHB_CLK_EN, 1,          // default:1 Enable LOW-RAM clock
                CPM_ROM_DIV_SEL, 0,         // default:0 ROM: 0:same as CPU
                CPM_CPU_DIV_EN, 1,          // default:1
                CPM_APB_CLK_EN, 1));        // default:1

    cpm_cpu_cfg_set(cpu_cfg);

    HS_PSO_UPD_RDY();
#endif
}

/**
 * @brief Get specified peripheral clock
 *
 * @param[in] type  clock source type
 *
 * @return clock in Hz
 */
uint32_t cpm_get_clock(cpm_clk_t type)
{
    uint32_t div = 0;
    uint32_t cfg = *cpm_clock_cfg_table[type];
    uint32_t top_clk = cpm_get_top_clock();

#ifdef CONFIG_HS6621
    uint32_t div_tmp = 0;
#endif

    switch (type)
    {
        case CPM_TOP_CLK:
            div = 1;
            break;

#ifdef CONFIG_HS6621
        case CPM_AHB_CLK:
        case CPM_LP_RAM_CLK:
        case CPM_CPU_CLK:
            // CPM_CPU_CLK = (TOP_CLK/CPM_CPU_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden
            div = register_get(&HS_PSO->CPU_CFG, MASK_POS(CPM_CPU_DIV_COEFF));
            div = div ? div : 1;
            break;

        case CPM_APB_CLK:
        case CPM_SPI_CLK:
            // CPM_APB_CLK = (TOP_CLK/CPM_CPU_DIV_COEFF/CPM_APB_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden
            div = register_get(&HS_PSO->CPU_CFG, MASK_POS(CPM_CPU_DIV_COEFF));
            div = div ? div : 1;
            div_tmp = register_get(&HS_PSO->CPU_CFG, MASK_POS(CPM_APB_DIV_COEFF));
            div_tmp = div_tmp ? div_tmp : 1;
            div = div * div_tmp;
            break;

        case CPM_ULP_RAM_CLK:
            // CPM_ULP_RAM_CLK = (TOP_CLK/CPM_CPU_DIV_COEFF/CPM_AHB_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden
            // Max:36MHz
            div = register_get(&HS_PSO->CPU_CFG, MASK_POS(CPM_CPU_DIV_COEFF));
            div = div ? div : 1;
            div_tmp = register_get(&HS_PSO->CPU_CFG, MASK_POS(CPM_AHB_DIV_COEFF));
            div_tmp = div_tmp ? div_tmp : 1;
            div = div * div_tmp;
            break;

        case CPM_ROM_CLK:
            // CPM_ROM_CLK: 2 or 1
            div = register_get(&HS_PSO->CPU_CFG, MASK_POS(CPM_ROM_DIV_SEL));
            div = div ? 2 : 1;
            break;
#else
        case CPM_SPI_CLK:
        case CPM_AHB_CLK:
        case CPM_APB_CLK:
            cfg = *cpm_clock_cfg_table[CPM_CPU_CLK];
        case CPM_CPU_CLK:
            if (!register_get(&cfg, MASK_POS(CPM_CPU_DIV_SEL)))
                div = 1; // Original clock
            else if (register_get(&cfg, MASK_POS(CPM_CPU_DIV_EN)))
                div = register_get(&cfg, MASK_POS(CPM_CPU_DIV_COEFF)); // Divided clock
            else
                div = 0; // No clock
            break;
#endif

        case CPM_I2C_CLK:
        case CPM_I2C1_CLK:
        case CPM_I2C2_CLK:
        case CPM_TIM0_CLK:
        case CPM_TIM1_CLK:
        case CPM_TIM2_CLK:
        case CPM_SF0_CLK:
        case CPM_SF1_CLK:
#ifdef CONFIG_HS6621C
        case CPM_SF2_CLK:
#endif
        case CPM_QDEC_CLK:
        case CPM_KPP_CLK:
            if (register_get(&cfg, MASK_POS(CPM_GATE_EN)))
                div = 0; // Gate
            else if (!register_get(&cfg, MASK_POS(CPM_DIV_SEL)))
                div = 1; // Original clock
            else if (register_get(&cfg, MASK_POS(CPM_DIV_EN)))
                div = register_get(&cfg, MASK_POS(CPM_DIV_COEFF)); // Divided clock
            else
                div = 0; // No clock
            break;

        case CPM_UART0_CLK:
        case CPM_UART1_CLK:
            if (register_get(&cfg, MASK_POS(CPM_UART_GATE_EN)))
                div = 0; // Gate
            else if (!register_get(&cfg, MASK_POS(CPM_UART_DIV_SEL)))
                div = 1; // Original clock
            else if (register_get(&cfg, MASK_POS(CPM_UART_DIV_EN)))
                // clk = (256*top_clk)/(256*int + frc)
                return ((uint64_t)top_clk<<8) / ((register_get(&cfg, MASK_POS(CPM_UART_DIV_COEFF_INT))<<8) +
                                        register_get(&cfg, MASK_POS(CPM_UART_DIV_COEFF_FRC)));
            else
                div = 0; // No clock
            break;

        case CPM_RTC_CLK:
        case CPM_WDT_CLK:
            return 32768;

        case CPM_I2S_CLK:
            // TODO
            break;

        case CPM_AUDIO_CLK:
            // TODO
            break;

        default:
            return 0;
    }

    return div ? top_clk/div : 0;
}

/**
 * @brief Set specified peripheral clock()
 *
 * @param[in] type  clock source type
 * @param[in] clk  clock in Hz. gate if 0
 *
 * @retval true success
 * @retval false fail
 **/
bool cpm_set_clock(cpm_clk_t type, uint32_t clk)
{
    __IO uint32_t *cfg = cpm_clock_cfg_table[type];
    uint32_t top_clk = cpm_get_top_clock();

#ifdef CONFIG_HS6621
    uint32_t div;
    __IO uint32_t reg;
#endif

    switch (type)
    {
        case CPM_TOP_CLK:
            break;

#ifdef CONFIG_HS6621
        case CPM_AHB_CLK:
        case CPM_LP_RAM_CLK:
        case CPM_CPU_CLK:
            // CPM_CPU_CLK = (TOP_CLK/CPM_CPU_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden
            co_assert(clk != 0);
            div = top_clk / clk;
            cpm_optimize_clock(top_clk/1000000, div); // optimize firstly
            div = div==1 ? 0 : div;
            reg = HS_PSO->CPU_CFG;
            register_set(&reg, MASK_1REG(CPM_CPU_DIV_COEFF, div));
            cpm_cpu_cfg_set(reg);
            break;

        case CPM_APB_CLK:
            // CPM_APB_CLK = (TOP_CLK/CPM_CPU_DIV_COEFF/CPM_APB_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden
            co_assert(clk != 0);
            div = cpm_get_clock(CPM_CPU_CLK) / clk;
            div = div==1 ? 0 : div;
            reg = HS_PSO->CPU_CFG;
            register_set(&reg, MASK_1REG(CPM_APB_DIV_COEFF, div));
            cpm_cpu_cfg_set(reg);
            break;

        case CPM_ULP_RAM_CLK:
            // CPM_ULP_RAM_CLK = (TOP_CLK/CPM_CPU_DIV_COEFF/CPM_AHB_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden
            co_assert(clk != 0);
            div = cpm_get_clock(CPM_CPU_CLK) / clk;
            div = div==1 ? 0 : div;
            reg = HS_PSO->CPU_CFG;
            register_set(&reg, MASK_1REG(CPM_AHB_DIV_COEFF, div));
            cpm_cpu_cfg_set(reg);
            break;

        case CPM_ROM_CLK:
            // CPM_ROM_CLK: 2 or 1
            co_assert(clk != 0);
            div = cpm_get_clock(CPM_CPU_CLK) / clk;
            div = div>=2 ? 1 : 0;
            reg = HS_PSO->CPU_CFG;
            register_set(&reg, MASK_1REG(CPM_ROM_DIV_SEL, div));
            cpm_cpu_cfg_set(reg);
            break;
#else
        case CPM_CPU_CLK:
        case CPM_AHB_CLK:
        case CPM_APB_CLK:
            co_assert(clk != 0);
            if(clk == top_clk)
                register_set(cfg, MASK_2REG(CPM_CPU_DIV_SEL,0, CPM_CPU_DIV_EN,1));
            else
                register_set(cfg, MASK_3REG(CPM_CPU_DIV_COEFF,top_clk/clk, CPM_CPU_DIV_SEL,1, CPM_CPU_DIV_EN,1));
            break;
#endif

        case CPM_I2C_CLK:  case CPM_I2C1_CLK: case CPM_I2C2_CLK:
        case CPM_TIM0_CLK: case CPM_TIM1_CLK: case CPM_TIM2_CLK:
        case CPM_SF0_CLK:  case CPM_SF1_CLK:
        case CPM_QDEC_CLK: case CPM_KPP_CLK:
            if(clk == 0)
                register_set1(cfg, CPM_GATE_EN_MASK); // Gate It
            else if(clk >= top_clk)
                register_set(cfg, MASK_3REG(CPM_DIV_SEL,0, CPM_DIV_EN,0, CPM_GATE_EN,0)); // Select original clock
            else
                register_set(cfg, MASK_4REG(CPM_DIV_COEFF,top_clk/clk,
                            CPM_DIV_SEL,1, CPM_DIV_EN,1, CPM_GATE_EN,0)); // Divided It
            break;

        case CPM_UART0_CLK:
        case CPM_UART1_CLK:
            if(clk == 0)
                register_set1(cfg, CPM_UART_GATE_EN_MASK); // Gate It
            else if(clk >= top_clk)
                register_set(cfg, MASK_3REG(CPM_UART_DIV_SEL,0, CPM_UART_DIV_EN,0, CPM_UART_GATE_EN,0));
            else
            {
                uint32_t div_x256 = ((uint64_t)top_clk << 8) / clk;
                uint32_t int_div = div_x256 >> 8;
                uint32_t frc_div = ((div_x256 % 256) << 8) / 256;
                register_set(cfg, MASK_5REG(CPM_UART_DIV_COEFF_FRC, frc_div,
                                            CPM_UART_DIV_COEFF_INT, int_div,
                                            CPM_UART_DIV_SEL,       1,
                                            CPM_UART_DIV_EN,        1,
                                            CPM_UART_GATE_EN,       0));
            }
            break;

        case CPM_AUDIO_CLK:
            // TODO
            break;

        case CPM_RTC_CLK: case CPM_SPI_CLK: case CPM_I2S_CLK:
        default:
            return false; // Don't div it
    }

    // Update it
    HS_PSO_UPD_RDY();

    return true;
}

/**
 * @brief Set specified peripheral clock()
 *
 * @param[in] type  clock source type
 * @param[in] div  div
 *
 * @retval true success
 * @retval false fail
 **/
bool cpm_set_clock_div(cpm_clk_t type, uint32_t div)
{
    __IO uint32_t *cfg = cpm_clock_cfg_table[type];

#ifdef CONFIG_HS6621
    __IO uint32_t reg;
#endif

    switch (type)
    {
        case CPM_TOP_CLK:
            break;

#ifdef CONFIG_HS6621
        case CPM_AHB_CLK:
        case CPM_LP_RAM_CLK:
        case CPM_CPU_CLK:
            // optimize firstly
            cpm_optimize_clock(cpm_get_top_clock()/1000000, div);
            // CPM_CPU_CLK = (TOP_CLK/CPM_CPU_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden
            div = div==1 ? 0 : div;
            reg = HS_PSO->CPU_CFG;
            register_set(&reg, MASK_1REG(CPM_CPU_DIV_COEFF, div));
            cpm_cpu_cfg_set(reg);
            break;

        case CPM_APB_CLK:
            // CPM_APB_CLK = (TOP_CLK/CPM_CPU_DIV_COEFF/CPM_APB_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden
            div = div==1 ? 0 : div;
            reg = HS_PSO->CPU_CFG;
            register_set(&reg, MASK_1REG(CPM_APB_DIV_COEFF, div));
            cpm_cpu_cfg_set(reg);
            break;

        case CPM_ULP_RAM_CLK:
            // CPM_ULP_RAM_CLK = (TOP_CLK/CPM_CPU_DIV_COEFF/CPM_AHB_DIV_COEFF); 0:bypass(FPGA not support 0); 1:forbiden
            div = div==1 ? 0 : div;
            reg = HS_PSO->CPU_CFG;
            register_set(&reg, MASK_1REG(CPM_AHB_DIV_COEFF, div));
            cpm_cpu_cfg_set(reg);
            break;

        case CPM_ROM_CLK:
            // CPM_ROM_CLK: 2 or 1
            div = div>=2 ? 1 : 0;
            reg = HS_PSO->CPU_CFG;
            register_set(&reg, MASK_1REG(CPM_ROM_DIV_SEL, div));
            cpm_cpu_cfg_set(reg);
            break;
#else
        case CPM_CPU_CLK:
        case CPM_AHB_CLK:
        case CPM_APB_CLK:
            if(div < 2)
                register_set(cfg, MASK_2REG(CPM_CPU_DIV_SEL,0, CPM_CPU_DIV_EN,1));
            else
                register_set(cfg, MASK_3REG(CPM_CPU_DIV_COEFF,div, CPM_CPU_DIV_SEL,1, CPM_CPU_DIV_EN,1));
            break;
#endif

        case CPM_I2C_CLK:  case CPM_I2C1_CLK: case CPM_I2C2_CLK:
        case CPM_TIM0_CLK: case CPM_TIM1_CLK: case CPM_TIM2_CLK:
        case CPM_SF0_CLK:  case CPM_SF1_CLK:
        case CPM_QDEC_CLK: case CPM_KPP_CLK:
            if(div <= 1)
                register_set(cfg, MASK_3REG(CPM_DIV_SEL,0, CPM_DIV_EN,0, CPM_GATE_EN,0));
            else
                register_set(cfg, MASK_4REG(CPM_DIV_COEFF,div,
                            CPM_DIV_SEL,1, CPM_DIV_EN,1, CPM_GATE_EN,0));
            break;

        case CPM_AUDIO_CLK:
            // TODO
            break;

        case CPM_UART0_CLK: case CPM_UART1_CLK: case CPM_RTC_CLK:
        case CPM_SPI_CLK:   case CPM_I2S_CLK:
        default:
            return false; // Don't div it
    }

    // Update it
    HS_PSO_UPD_RDY();

    return true;
}

#ifdef CONFIG_HS6621
/**
 * @brief  cpm optimize clock
 *
 * @param[in] want_topclk_mhz  want topclk mhz, 0:use reg value
 * @param[in] want_cpu_div  want cpu div, 0:use reg value
 **/
void cpm_optimize_clock(uint32_t want_topclk_mhz, uint32_t want_cpu_div)
{
    uint32_t topclk_mhz = want_topclk_mhz ? want_topclk_mhz : (cpm_get_top_clock()/1000000);
    uint32_t cpu_div = want_cpu_div ? want_cpu_div : REGR(&HS_PSO->CPU_CFG, MASK_POS(CPM_CPU_DIV_COEFF));
    uint32_t cpu_mhz = topclk_mhz / (cpu_div ? cpu_div : 1);
    uint32_t rom_div, ulp_div;
    uint32_t reg = HS_PSO->CPU_CFG;

    // ROM clock must <= 102MHz (default no div)
    rom_div = (topclk_mhz+101) / 102;
    rom_div = rom_div>=2 ? 1 : 0;

    // ULP SRAM clock must <= 32MHz (default no div)
    ulp_div = (cpu_mhz+31) / 32;
    ulp_div = ulp_div==1 ? 0 : ulp_div;

    // set
    register_set(&reg, MASK_2REG(CPM_ROM_DIV_SEL,rom_div, CPM_AHB_DIV_COEFF,ulp_div));
    cpm_cpu_cfg_set(reg);
}
#endif

/**
 * @brief cpm_store()
 *
 * Just for system call before sleep
 *
 * @return None
 **/
void cpm_store(void)
{
    cpm_env.lowpower.store_reg.CPU_CFG = HS_PSO->CPU_CFG;
    cpm_env.lowpower.store_reg.RAM_CFG = HS_PSO->RAM_CFG;

#ifdef CONFIG_HS6621
    uint32_t cpu_div = REGR(&cpm_env.lowpower.store_reg.CPU_CFG, MASK_POS(CPM_CPU_DIV_COEFF));
    REGW(&cpm_env.lowpower.store_reg.CPU_CFG, MASK_2REG(CPM_AHB_DIV_COEFF,cpu_div?0:2, CPM_ROM_DIV_SEL,0));
#endif
}

/**
 * @brief cpm_restore()
 *
 * Just for system call after sleep
 *
 * @return None
 **/
void cpm_restore(void)
{
    cpm_cpu_cfg_set(cpm_env.lowpower.store_reg.CPU_CFG);

    HS_PSO->RAM_CFG = cpm_env.lowpower.store_reg.RAM_CFG;
    HS_PSO_UPD_RDY();
}

