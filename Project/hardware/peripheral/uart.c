/*********************************************************************
 * @file uart.c
 * @brief 
 * @version 1.0
 * @date Wed 19 Nov 2014 04:11:47 PM CST
 * @author liqiang
 *
 * @note 
 */

/*********************************************************************
 * INCLUDES
 */
#include "features.h"
#include "hs66xx.h"
#include "uart.h"
#include "cpm.h"


/*********************************************************************
 * MACROS
 */
#define MODE_X_DIV          16
#define ISO7816_RX_MODE     1
#define ISO7816_TX_MODE     0

#define UART_TIMEOUT        (1024 * 1024 * 2)

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    uart_rx_callback_t      rx_cb;
    uart_tx_cmp_callback_t  tx_cmp_cb;
}uart_callback_t;

typedef struct
{
    uart_callback_t uart[2];
}uart_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */

static uart_env_t uart_env;

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief uart_check_irq()
 *
 * @param[in] uart  
 *
 * @return 
 **/
static void uart_check_irq(HS_UART_Type *uart, uart_callback_t *uart_cb)
{
    uint8_t status;
    volatile uint8_t ch;

    while (1)
    {
        status = uart->IIR & UART_IIR_ID;
        switch (status)
        {
            case UART_IIR_RDI:
                while(uart->LSR & UART_LSR_DR)
                {
                    ch = uart->RBR;
                    if (uart_cb->rx_cb)
                        uart_cb->rx_cb(ch);
                }
                break;

            case UART_IIR_THRI:
                uart->IER &= ~UART_IER_THRI;
                if (uart_cb->tx_cmp_cb)
                    uart_cb->tx_cmp_cb();
                break;

            case UART_IIR_NO_INT:
                return;

            case UART_IIR_BDI:
                status = uart->USR;
                break;

            case UART_IIR_RLSI:
                ch = uart->LSR;
                break;

            case UART_IIR_CTI:
                ch = uart->LSR;
                ch = uart->RBR;
                break;

            default:
                break;
        }
    }
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief uart initialize
 *
 * @param[in] uart  uart object
 * @param[in] baud_rate  baud rate
 * @param[in] flow_ctrl  flow control (Only UART1 support)
 * @param[in] uart_rx_cb  receive callback
 *
 * @return None
 **/
void uart_open(HS_UART_Type *uart, uint32_t baud_rate, uart_flow_ctrl_t flow_ctrl, uart_rx_callback_t uart_rx_cb)
{
    uint16_t baud_divisor;

    if(uart == HS_UART0)
    {
        // Reset and Bypass UART0
        register_set1(&HS_PSO->UART0_CFG, CPM_UART_SOFT_RESET_MASK);
        register_set0(&HS_PSO->UART0_CFG, CPM_UART_DIV_SEL_MASK | CPM_UART_GATE_EN_MASK);
        HS_PSO_UPD_RDY();

        uart_env.uart[0].tx_cmp_cb = NULL;
        uart_env.uart[0].rx_cb = NULL;
    }
    else
    {
        // Reset and Bypass UART1
        register_set1(&HS_PSO->UART1_CFG, CPM_UART_SOFT_RESET_MASK);
        register_set0(&HS_PSO->UART1_CFG, CPM_UART_DIV_SEL_MASK | CPM_UART_GATE_EN_MASK);
        HS_PSO_UPD_RDY();

        uart_env.uart[1].tx_cmp_cb = NULL;
        uart_env.uart[1].rx_cb = NULL;
    }

    /* Compute divisor value. Normally, we should simply return:
     *   NS16550_CLK / MODE_X_DIV / baudrate
     * but we need to round that value by adding 0.5.
     * Rounding is especially important at high baud rates.
     */

    if (cpm_get_clock(CPM_TOP_CLK)>64000000 && baud_rate<19200)
        baud_divisor = 4; // make sure 9600bps is supported
    else
        baud_divisor = 1;

    if (uart == HS_UART0)
        cpm_set_clock(CPM_UART0_CLK, baud_divisor*baud_rate*MODE_X_DIV);
    else
        cpm_set_clock(CPM_UART1_CLK, baud_divisor*baud_rate*MODE_X_DIV);

    // Disable LCR and irq
    uart->LCR = 0x00;
    uart->IER = 0;

    if (uart == HS_UART0)
    {
        uart->MCR = 0x00;
        uart->FCR = UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT;
    }
    else
    {
        // Auto RTS – Becomes active when the following occurs:
        //   - Auto Flow Control is selected during configuration
        //   - FIFOs are implemented
        //   - RTS (MCR[1] bit and MCR[5]bit are both set)
        //   - FIFOs are enabled (FCR[0]) bit is set)
        //   - SIR mode is disabled (MCR[6] bit is not set)
        if(flow_ctrl == UART_FLOW_CTRL_ENABLED)
            uart->MCR = UART_MCR_RTS | UART_MCR_AFCE;
        else
            uart->MCR = 0x00;

        uart->FCR = UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT |
            UART_FCR_FIFO_EN | UART_FCR_TRIGGER_1;
    }

    /* Baud rate setting.*/
    uart->LCR = UART_LCR_DLAB;
    uart->DLL = baud_divisor & 0xff;
    uart->DLH = (baud_divisor >> 8) & 0xff;

    /* 8 data, 1 stop, no parity */
    uart->LCR = UART_LCR_8N1;

    /* Set UARTs int mask */
    if(uart_rx_cb)
    {
        uart->IER = UART_IER_RLSI | UART_IER_RDI;

        if(uart == HS_UART0)
        {
            NVIC_ClearPendingIRQ(UART0_IRQn);
            NVIC_SetPriority(UART0_IRQn, IRQ_PRIORITY_HIGH);
            NVIC_EnableIRQ(UART0_IRQn);
        }
        else
        {
            NVIC_ClearPendingIRQ(UART1_IRQn);
            NVIC_SetPriority(UART1_IRQn, IRQ_PRIORITY_HIGH);
            NVIC_EnableIRQ(UART1_IRQn);
        }
    }
    else
    {
        uart->IER = 0;

        if(uart == HS_UART0)
            NVIC_DisableIRQ(UART0_IRQn);
        else
            NVIC_DisableIRQ(UART1_IRQn);
    }

    if(uart == HS_UART0)
    {
        uart_env.uart[0].tx_cmp_cb = NULL;
        uart_env.uart[0].rx_cb = uart_rx_cb;
    }
    else
    {
        uart_env.uart[1].tx_cmp_cb = NULL;
        uart_env.uart[1].rx_cb = uart_rx_cb;
    }
}

/**
 * @brief uart_close()
 *
 * @param[in] uart  
 *
 * @return 
 **/
void uart_close(HS_UART_Type *uart)
{
    uart->LCR = 0x00;
    uart->IER = 0;             /* Disable 16550 Interrupts */
    uart->FCR = UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT;

    if(uart == HS_UART0)
        register_set1(&HS_PSO->UART0_CFG, CPM_UART_GATE_EN_MASK);
    else
        register_set1(&HS_PSO->UART1_CFG, CPM_UART_GATE_EN_MASK);
    HS_PSO_UPD_RDY();
}

/**
 * @brief uart send with noblock
 *
 * @param[in] uart  uart object
 * @param[in] buf  transmit data buffer
 * @param[in] length  transmit data length
 * @param[in] txcb  transmit complete callback
 *
 * @return None
 **/
void uart_send_noblock(HS_UART_Type *uart, uint8_t **buf, unsigned *length, uart_tx_cmp_callback_t txcb)
{
    uint32_t count = *length;
    uint32_t fifo_len = 0;

    if(uart == HS_UART0)
    {
        uart_env.uart[0].tx_cmp_cb = txcb;
        fifo_len = 1;
    }
    else if(uart == HS_UART1)
    {
        uart_env.uart[1].tx_cmp_cb = txcb;
        fifo_len = 16;
    }
    else
    {
        return ;
    }

    if (count >= fifo_len)
        count = fifo_len;

    (*length) -= count;

    do
    {
        uart->THR = *((*buf)++);
        count--;
    }while(count);

    uart->IER |= UART_IER_THRI;
}

/**
 * @brief uart wait send finish @ref uart_send_noblock
 *
 * @param[in] uart  uart object
 *
 * @return None
 **/
void uart_wait_send_finish(HS_UART_Type *uart)
{
    while (!(uart->LSR & UART_LSR_TEMT));
}

/**
 * @brief uart send with block
 *
 * @param[in] uart  uart object
 * @param[in] buf  transmit data buffer
 * @param[in] length  transmit data length
 *
 * @return None
 **/
void uart_send_block(HS_UART_Type *uart, const uint8_t *buf, unsigned length)
{
    unsigned i;

    for(i=0; i<length; ++i)
    {
        while (!(uart->LSR & UART_LSR_THRE));
        uart->THR = buf[i];
    }

    while (!(uart->LSR & UART_LSR_TEMT));
}

/**
 * @brief uart empty fifo
 *
 * @param[in] uart  uart object
 *
 * @return None
 **/
void uart_empty_fifo(HS_UART_Type *uart)
{
    while (uart->LSR & UART_LSR_DR) /* Read all characters out of the fifo */
    {
        if(uart == HS_UART0)
        {
            if(uart_env.uart[0].rx_cb)
                uart_env.uart[0].rx_cb(uart->RBR);
        }
        else if(uart == HS_UART1)
        {
          if(uart_env.uart[1].rx_cb){
            uart_env.uart[1].rx_cb(uart->RBR);
          }
          else{
            uart->RBR;
          }
        }
        else
        {
            ;
        }
    }
}

int uart_is_empty(HS_UART_Type *uart)
{
    if(uart->LSR & UART_LSR_DR)
    {
        return 0;
    }
    return 1;
}

int uart_pio_rx(HS_UART_Type *uart, uint8_t *buf, uint32_t length) 
{
    uint32_t recv_length = 0;
    uint32_t time_ount = 0;
    while(recv_length < length)
    {
        if(uart->LSR & (UART_LSR_OE | UART_LSR_ERR))
        {
            break;
        }

        if (uart->LSR & UART_LSR_DR)
        {
            *buf++ = uart->RBR;
            ++recv_length;
        }

        if(++time_ount >= UART_TIMEOUT)
            break;
    }

    return recv_length;
}

void UART0_IRQHandler(void)
{
    uart_check_irq(HS_UART0, &uart_env.uart[0]);
}

void UART1_IRQHandler(void)
{
    uart_check_irq(HS_UART1, &uart_env.uart[1]);
}

/**
 * @brief iso7816 initialize
 *
 * @param[in] clock  7816 clock
 * @param[in] etu  7816 ETU: default is 372
 * @param[in] rx_cb  receive event callback
 *
 * @return None
 **/
void iso7816_init(uint32_t clock, uint32_t etu, uart_rx_callback_t rx_cb)
{
    uint32_t baud_divisor = etu - 1;

    uart_env.uart[1].tx_cmp_cb = NULL;
    uart_env.uart[1].rx_cb = NULL;

    // Disable LCR and irq
    HS_UART1->LCR = 0x00;
    HS_UART1->IER = 0;

    cpm_set_clock(CPM_UART1_CLK, clock);

    HS_UART1->MCR = UART_MCR_RTS;
    HS_UART1->FCR = UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_FIFO_EN | UART_FCR_TRIGGER_1;

    /* Baud rate setting.*/
    HS_UART1->LCR = UART_LCR_DLAB;
    HS_UART1->DLL = baud_divisor & 0xff;
    HS_UART1->DLH = (baud_divisor >> 8) & 0xff;

    HS_UART1->ISO7816.TRX_EN = ISO7816_RX_MODE;
    HS_UART1->ISO7816.ENABLE = 1;
    HS_UART1->ISO7816.RETRANS_EN = 0;
    HS_UART1->ISO7816.SAMPLE_DLY = etu/2;

    /* 8 data, 1 stop, even parity */
    HS_UART1->LCR = UART_LCR_WLS_8 | UART_LCR_PEN | UART_LCR_EPS;// | UART_LCR_STB;

    /* Set UARTs int mask */
    if(rx_cb)
    {
        HS_UART1->IER = UART_IER_RLSI | UART_IER_RDI;
        NVIC_ClearPendingIRQ(UART1_IRQn);
        NVIC_SetPriority(UART1_IRQn, IRQ_PRIORITY_HIGH);
        NVIC_EnableIRQ(UART1_IRQn);
    }
    else
    {
        HS_UART1->IER = 0;
        NVIC_DisableIRQ(UART1_IRQn);
    }

    uart_env.uart[1].tx_cmp_cb = NULL;
    uart_env.uart[1].rx_cb = rx_cb;
}

/**
 * @brief iso7816 send data with block
 *
 * @param[in] buf  send data buffer
 * @param[in] length  send data length
 *
 * @return None
 **/
void iso7816_send_block(const uint8_t *buf, unsigned length)
{
    unsigned i;

    HS_UART1->ISO7816.TRX_EN = ISO7816_TX_MODE;

    for(i=0; i<length; ++i)
    {
        while (!(HS_UART1->LSR & UART_LSR_THRE));
        HS_UART1->THR = buf[i];
    }

    while (!(HS_UART1->LSR & UART_LSR_TEMT));

    HS_UART1->ISO7816.TRX_EN = ISO7816_RX_MODE;
}

