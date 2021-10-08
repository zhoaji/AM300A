/**
 * @file i2c.c
 * @brief
 * @date Wed 31 May 2017 07:15:09 PM CST
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
#define I2C_SLAVE_DEFAULT_ADDR  0x62

#define I2C_TX_FIFO_DEPTH       16
#define I2C_RX_FIFO_DEPTH       16

#define I2CD_NO_ERROR           0x00   /**< @brief No error.            */
#define I2CD_BUS_ERROR          0x01   /**< @brief Bus Error.           */
#define I2CD_ARBITRATION_LOST   0x02   /**< @brief Arbitration Lost.    */
#define I2CD_ACK_FAILURE        0x04   /**< @brief Acknowledge Failure. */
#define I2CD_OVERRUN            0x08   /**< @brief Overrun/Underrun.    */
#define I2CD_PEC_ERROR          0x10   /**< @brief PEC Error in reception. */
#define I2CD_TIMEOUT            0x20   /**< @brief Hardware timeout.    */
#define I2CD_SMB_ALERT          0x40   /**< @brief SMBus Alert.         */

#define I2C_INTR_DEFAULT_MASK   (I2C_INTR_RX_FULL | I2C_INTR_TX_EMPTY | I2C_INTR_TX_ABRT | I2C_INTR_STOP_DET)
#define I2C_INTR_SLAVE_MASK     (I2C_INTR_RD_REQ | I2C_INTR_TX_ABRT | I2C_INTR_RX_FULL | I2C_INTR_STOP_DET)

#define I2C_DEFAULT_TIMEOUT     660000 // 1s@CPU64MHz

#define I2C_DEAD_WORKAROUND

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    uint8_t *tx_buf1;
    uint32_t tx_len1;
    uint8_t *tx_buf2;
    uint32_t tx_len2;

    uint8_t *rx_buf;
    uint32_t rx_len;
}i2c_op_t;

#ifdef I2C_DEAD_WORKAROUND
typedef struct
{
    uint32_t speed;
}i2c_env_t;
#endif

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
#ifdef I2C_DEAD_WORKAROUND
static i2c_env_t i2c_env;
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

#ifdef I2C_DEAD_WORKAROUND
/**
 * @brief  i2c dead workaround
 **/
static void i2c_dead_workaround(void)
{
    int unlock_timeout = 200;

    // get pin number
    int scl_pin = pinmux_search(PINMUX_I2C_MST_SCK_CFG);
    int sda_pin = pinmux_search(PINMUX_I2C_MST_SDA_CFG);
    co_assert (scl_pin>=0 && sda_pin>=0);

    // pinmux to  gpio
    pinmux_config(scl_pin, PINMUX_GPIO_MODE_CFG);
    pinmux_config(sda_pin, PINMUX_GPIO_MODE_CFG);

    // gpio mode
    gpio_set_direction(BITMASK(scl_pin), GPIO_OUTPUT);
    gpio_set_direction(BITMASK(sda_pin), GPIO_INPUT);

    // unlock
    while((gpio_read(BITMASK(sda_pin)) == 0) && (--unlock_timeout > 0))
    {
        gpio_toggle(BITMASK(scl_pin));
        co_delay_10us(10);
    }

    // reset
    i2c_open(I2C_MODE_MASTER, i2c_env.speed);

    // restore
    pinmux_config(scl_pin, PINMUX_I2C_MST_SCK_CFG);
    pinmux_config(sda_pin, PINMUX_I2C_MST_SDA_CFG);
}
#endif

/**
 * @brief i2c_set_mode()
 *
 * @param[in] mode
 *
 * @return
 **/
static void i2c_set_mode(i2c_mode_t mode)
{
    switch(mode)
    {
        case I2C_MODE_MASTER:
            HS_I2C->CON = I2C_CON_MASTER | I2C_CON_SLAVE_DISABLE | I2C_CON_RESTART_EN;
            break;
        case I2C_MODE_SLAVE:
            HS_I2C->CON = I2C_CON_RESTART_EN;
            break;
        case I2C_MODE_SMBUS_DEVICE:
            HS_I2C->CON = I2C_CON_10BITADDR_SLAVE | I2C_CON_RESTART_EN;
            break;
        case I2C_MODE_SMBUS_HOST:
            HS_I2C->CON = I2C_CON_MASTER | I2C_CON_SLAVE_DISABLE | I2C_CON_10BITADDR_MASTER | I2C_CON_RESTART_EN;
            break;
    }
}

/**
 * @brief i2c_scl_hcnt()
 *
 * @param[in] ic_clk
 * @param[in] tsymbol
 * @param[in] tf
 * @param[in] cond
 * @param[in] offset
 *
 * @return
 **/
static uint32_t i2c_scl_hcnt(uint32_t ic_clk, uint32_t tsymbol, uint32_t tf, int cond, int offset)
{
    /*
     * DesignWare I2C core doesn't seem to have solid strategy to meet
     * the tHD;STA timing spec.  Configuring _HCNT based on tHIGH spec
     * will result in violation of the tHD;STA spec.
     */
    if (cond)
        /*
         * Conditional expression:
         *
         *   IC_[FS]S_SCL_HCNT + (1+4+3) >= IC_CLK * tHIGH
         *
         * This is based on the manuals, and represents an ideal
         * configuration.  The resulting I2C bus speed will be
         * faster than any of the others.
         *
         * If your hardware is free from tHD;STA issue, try this one.
         */
        return (ic_clk * tsymbol + 5000) / 10000 - 8 + offset;
    else
        /*
         * Conditional expression:
         *
         *   IC_[FS]S_SCL_HCNT + 3 >= IC_CLK * (tHD;STA + tf)
         *
         * This is just experimental rule; the tHD;STA period turned
         * out to be proportinal to (_HCNT + 3).  With this setting,
         * we could meet both tHIGH and tHD;STA timing specs.
         *
         * If unsure, you'd better to take this alternative.
         *
         * The reason why we need to take into account "tf" here,
         * is the same as described in i2c_lld_scl_lcnt().
         */
        return (ic_clk * (tsymbol + tf) + 5000) / 10000 - 3 + offset;
}

/**
 * @brief i2c_scl_lcnt()
 *
 * @param[in] ic_clk
 * @param[in] tLOW
 * @param[in] tf
 * @param[in] offset
 *
 * @return
 **/
static uint32_t i2c_scl_lcnt(uint32_t ic_clk, uint32_t tlow, uint32_t tf, int offset)
{
    /*
     * Conditional expression:
     *
     *   IC_[FS]S_SCL_LCNT + 1 >= IC_CLK * (tLOW + tf)
     *
     * DW I2C core starts counting the SCL CNTs for the LOW period
     * of the SCL clock (tLOW) as soon as it pulls the SCL line.
     * In order to meet the tLOW timing spec, we need to take into
     * account the fall time of SCL signal (tf).  Default tf value
     * should be 0.3 us, for safety.
     */
    return ((ic_clk * (tlow + tf) + 5000) / 10000) - 1 + offset;
}

/**
 * @brief i2c_set_speed()
 *
 * @param[in] i2cp
 *
 * @return
 **/
static void i2c_set_speed(uint32_t speed)
{
    uint32_t clk = cpm_get_clock(CPM_I2C_CLK);

    if (speed <= 100000) //100kbps
    {
        /* set standard and fast speed deviders for high/low periods */
        /* Standard-mode @100k period=10us */
        HS_I2C->SS_SCL_HCNT = i2c_scl_hcnt(clk/1000,
                40,/* tHD;STA = tHIGH = 4.0 us */
                3, /* tf = 0.3 us */
                0, /* 0: default, 1: Ideal */
                0);/* No offset */
        HS_I2C->SS_SCL_LCNT = i2c_scl_lcnt(clk/1000,
                47,/* tLOW = 4.7 us */
                3, /* tf = 0.3 us */
                0);/* No offset */

        /* Standard mode clock_div calculate: Tlow/Thigh = 1/1.*/
        /* Sets the Maximum Rise Time for standard mode.*/
        HS_I2C->CON = (HS_I2C->CON & ~I2C_CON_SPEED_MASK) | I2C_CON_SPEED_STD;
    }
    else// if (speed <= 400000) //400kbps
    {
        /* Fast-mode @400k period=2.5us */
        HS_I2C->FS_SCL_HCNT = i2c_scl_hcnt(clk/1000,
                6, /* tHD;STA = tHIGH = 0.6 us */
                3, /* tf = 0.3 us */
                0, /* 0: default, 1: Ideal */
                0);/* No offset */
        HS_I2C->FS_SCL_LCNT = i2c_scl_lcnt(clk/1000,
                13,/* tLOW = 1.3 us */
                3, /* tf = 0.3 us */
                0);/* No offset */

        /* Sets the Maximum Rise Time for fast mode.*/
        HS_I2C->CON = (HS_I2C->CON & ~I2C_CON_SPEED_MASK) | I2C_CON_SPEED_FAST;
    }
}

/**
 * @brief i2c_read_clear_intrbits()
 *
 * @param[in] err  
 *
 * @return 
 **/
static uint32_t i2c_read_clear_intrbits(uint32_t *err)
{
    uint32_t dummy, stat = HS_I2C->INTR_STAT;
    uint32_t errors = 0;

    /* Do not use the IC_CLR_INTR register to clear interrupts. */
    if (stat & I2C_INTR_RX_UNDER)
    {
        errors |= I2CD_OVERRUN;
        dummy = HS_I2C->CLR_RX_UNDER;
    }

    if (stat & I2C_INTR_RX_OVER)
    {
        errors |= I2CD_OVERRUN;
        dummy = HS_I2C->CLR_RX_OVER;
    }

    if (stat & I2C_INTR_TX_OVER)
    {
        errors |= I2CD_OVERRUN;
        dummy = HS_I2C->CLR_TX_OVER;
    }

    if (stat & I2C_INTR_RD_REQ)
        dummy = HS_I2C->CLR_RD_REQ;

    if (stat & I2C_INTR_TX_ABRT)
    {
        /*
         * The IC_TX_ABRT_SOURCE register is cleared whenever
         * the IC_CLR_TX_ABRT is read.  Preserve it beforehand.
         */
        dummy = HS_I2C->TX_ABRT_SOURCE;
        if (dummy & I2C_TX_ARB_LOST)
            errors |= I2CD_ARBITRATION_LOST;
        if (dummy & 0x1f/*I2C_TX_ABRT_xxx_NOACK*/)
            errors |= I2CD_ACK_FAILURE;
        if (dummy & 0xfe0)
            errors |= I2CD_BUS_ERROR; /* it is trigged by wrong sw behaviours */
        dummy = HS_I2C->CLR_TX_ABRT;
    }

    if (stat & I2C_INTR_RX_DONE)
        dummy = HS_I2C->CLR_RX_DONE;

    if (stat & I2C_INTR_ACTIVITY)
        dummy = HS_I2C->CLR_ACTIVITY;

    if (stat & I2C_INTR_STOP_DET)
        dummy = HS_I2C->CLR_STOP_DET;

    if (stat & I2C_INTR_START_DET)
        dummy = HS_I2C->CLR_START_DET;

    if (stat & I2C_INTR_GEN_CALL)
        dummy = HS_I2C->CLR_GEN_CALL;

    if(err)
        *err = errors;

    (void)dummy;
    return stat;
}

/**
 * @brief i2c_read_pio()
 *
 * @param[in] op  
 *
 * @return 
 **/
static void i2c_read_pio(i2c_op_t *op)
{
    uint32_t rx_valid;

    if(op->tx_len1>0 || op->tx_len2>0)
        return;

    if(op->rx_len==0)
        return;

    rx_valid = HS_I2C->RXFLR;

    for (; op->rx_len > 0 && rx_valid > 0; op->rx_len--, rx_valid--)
        *op->rx_buf++ = HS_I2C->DATA_CMD;
}

/**
 * @brief i2c_xfer_pio()
 *
 * @param[in] op  
 *
 * @return 
 **/
static void i2c_xfer_pio(i2c_op_t *op)
{
    uint32_t tx_limit;

    if(op->tx_len1==0 && op->tx_len2==0)
        return;

    tx_limit = I2C_TX_FIFO_DEPTH - HS_I2C->TXFLR;

    for(; op->tx_len1 > 0 && tx_limit > 0; tx_limit--,op->tx_len1--)
        HS_I2C->DATA_CMD = *op->tx_buf1++;

    if(op->tx_len1 == 0)
    {
        for(; op->tx_len2 > 0 && tx_limit > 0; tx_limit--,op->tx_len2--)
            HS_I2C->DATA_CMD = *op->tx_buf2++;
    }

    if(op->tx_len1==0 && op->tx_len2==0)
    {
        if(op->rx_len > 0)
        {
            // BUG: tx to rx must be delay
            co_delay_us(9);
            HS_I2C->CON1 = op->rx_len | I2C_CON1_RX_ENABLE | I2C_CON1_READBYTES_UPDATE;
        }
    }
}

/**
 * @brief i2c_serve_interrupt()
 *
 * @param[in] op  
 * @param[in] errors  
 *
 * @return 
 **/
static bool i2c_serve_interrupt(i2c_op_t *op, uint32_t *errors)
{
    uint32_t stat;

    stat = i2c_read_clear_intrbits(errors);

    if (stat & I2C_INTR_RX_FULL)
        i2c_read_pio(op);

    if (stat & (I2C_INTR_TX_EMPTY | I2C_INTR_RD_REQ))
        i2c_xfer_pio(op);

    if (stat & I2C_INTR_STOP_DET)
    {
        HS_I2C->ENABLE = 0;
        return true;
    }

    return false;
}

/**
 * @brief i2c_master_transmit_ex()
 *
 * @param[in] addr  
 * @param[in] op  
 * @param[in] timeout  
 *
 * @return 
 **/
static bool i2c_master_transmit_ex(uint16_t addr, i2c_op_t *op, uint32_t timeout)
{
    uint32_t errors = 0;
    uint32_t count = 0;
    bool res;

    HS_I2C->ENABLE = 0;
    HS_I2C->TAR = addr;
    HS_I2C->CON1 = op->tx_len1 ? I2C_CON1_TX_ENABLE : (op->rx_len | I2C_CON1_RX_ENABLE | I2C_CON1_READBYTES_UPDATE);
    HS_I2C->ENABLE = 1;
    HS_I2C->INTR_MASK = I2C_INTR_DEFAULT_MASK;

    while(1)
    {
        res = i2c_serve_interrupt(op, &errors);
        if(res)
            break;

        if(timeout && count++>timeout)
        {
            errors |= I2CD_TIMEOUT;

#ifdef I2C_DEAD_WORKAROUND
            i2c_dead_workaround();
#endif

            break;
        }
    }

    return errors ? false : true;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief i2c initialize
 *
 * @param[in] mode  mode
 * @param[in] speed  speed
 *
 * @return None
 **/
void i2c_open(i2c_mode_t mode, uint32_t speed)
{
    // Open clock
    register_set1(&HS_PSO->I2C_CFG, CPM_I2C_SOFT_RESET_MASK);
    register_set0(&HS_PSO->I2C_CFG, CPM_I2C_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    HS_I2C->ENABLE = 0;

    i2c_set_mode(mode);
    i2c_set_speed(speed);

    HS_I2C->TX_TL = 0;/* tx fifo has one byte or below then trigger the tx empty interrupt.*/
    HS_I2C->RX_TL = 0;/* rx fifo has received one byte then trigger the rx full interrupt.*/

    if(I2C_MODE_SLAVE == mode)
    {
        /* setting I2C slave addr */
        HS_I2C->SAR = I2C_SLAVE_DEFAULT_ADDR;
    }

//    tx_fifo_depth = ((HS_I2C->COMP_PARAM_1 >> 16) & 0xff) + 1;
//    rx_fifo_depth = ((HS_I2C->COMP_PARAM_1 >> 8) & 0xff) + 1;

    /* disable interrupts */
    HS_I2C->INTR_MASK = 0;

#ifdef I2C_DEAD_WORKAROUND
    i2c_env.speed = speed;
#endif
}

/**
 * @brief i2c close
 *
 * @return None
 **/
void i2c_close(void)
{
    // Open clock
    register_set1(&HS_PSO->I2C_CFG, CPM_I2C_GATE_EN_MASK);
    HS_PSO_UPD_RDY();
}

/**
 * @brief i2c_master_transmit()
 *
 * @param[in] addr  
 * @param[in] tx_buf  
 * @param[in] tx_len  
 * @param[in] rx_buf  
 * @param[in] rx_len  
 *
 * @return 
 **/
bool i2c_master_transmit(uint16_t addr, uint8_t *tx_buf, uint32_t tx_len, uint8_t *rx_buf, uint32_t rx_len)
{
    i2c_op_t op = {tx_buf, tx_len, NULL, 0, rx_buf, rx_len};
    return i2c_master_transmit_ex(addr, &op, I2C_DEFAULT_TIMEOUT);
}

/**
 * @brief i2c master write data
 *
 * @param[in] addr  slave address
 * @param[in] tx_buf  transmit data buffer
 * @param[in] tx_len  transmit data length
 *
 * @retval true success
 * @retval false fail
 **/
bool i2c_master_write(uint16_t addr, uint8_t *tx_buf, uint32_t tx_len)
{
    i2c_op_t op = {tx_buf, tx_len, NULL, 0, NULL, 0};
    return i2c_master_transmit_ex(addr, &op, I2C_DEFAULT_TIMEOUT);
}

/**
 * @brief i2c master read data
 *
 * @param[in] addr  slave address
 * @param[in] rx_buf  receive data buffer
 * @param[in] rx_len  receive buffer length
 *
 * @retval true success
 * @retval false fail
 **/
bool i2c_master_read(uint16_t addr, uint8_t *rx_buf, uint32_t rx_len)
{
    i2c_op_t op = {NULL, 0, NULL, 0, rx_buf, rx_len};
    return i2c_master_transmit_ex(addr, &op, I2C_DEFAULT_TIMEOUT);
}

/**
 * @brief i2c master read memery (EEPROM)
 *
 * @param[in] addr  I2C address
 * @param[in] offset  memery offset
 * @param[in] alen  memery offset bytes
 * @param[in] rx_buf  receive data buffer
 * @param[in] rx_len  receive data lenght
 *
 * @retval true success
 * @retval false fail
 **/
bool i2c_master_read_mem(uint16_t addr, uint32_t offset, uint32_t alen, uint8_t *rx_buf, uint32_t rx_len)
{
    i2c_op_t op = {(uint8_t *)&offset, alen, NULL, 0, rx_buf, rx_len};
    return i2c_master_transmit_ex(addr, &op, I2C_DEFAULT_TIMEOUT);
}

/**
 * @brief i2c master write memery (EEPROM)
 *
 * @param[in] addr  I2C address
 * @param[in] offset  memery offset
 * @param[in] alen  memery offset bytes
 * @param[in] tx_buf  transmit data buffer
 * @param[in] tx_len  transmit data length
 *
 * @retval true success
 * @retval false fail
 **/
bool i2c_master_write_mem(uint16_t addr, uint32_t offset, uint32_t alen, uint8_t *tx_buf, uint32_t tx_len)
{
    i2c_op_t op = {(uint8_t *)&offset, alen, tx_buf, tx_len, NULL, 0};
    return i2c_master_transmit_ex(addr, &op, I2C_DEFAULT_TIMEOUT);
}

/** @} */


