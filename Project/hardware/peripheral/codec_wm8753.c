/**
 * @file codec_wm8753.c
 * @brief 
 * @date Mon 05 Jun 2017 11:57:14 AM CST
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
#include "codec_wm8753.h"

/*********************************************************************
 * MACROS
 */
#define WM8753_DAC        0x01
#define WM8753_ADC        0x02
#define WM8753_PCM        0x03
#define WM8753_HIFI       0x04
#define WM8753_IOCTL      0x05
#define WM8753_SRATE1     0x06
#define WM8753_SRATE2     0x07
#define WM8753_LDAC       0x08
#define WM8753_RDAC       0x09
#define WM8753_BASS       0x0a
#define WM8753_TREBLE     0x0b
#define WM8753_ALC1       0x0c
#define WM8753_ALC2       0x0d
#define WM8753_ALC3       0x0e
#define WM8753_NGATE      0x0f
#define WM8753_LADC       0x10
#define WM8753_RADC       0x11
#define WM8753_ADCTL1     0x12
#define WM8753_3D         0x13
#define WM8753_PWR1       0x14
#define WM8753_PWR2       0x15
#define WM8753_PWR3       0x16
#define WM8753_PWR4       0x17
#define WM8753_ID         0x18
#define WM8753_INTPOL     0x19
#define WM8753_INTEN      0x1a
#define WM8753_GPIO1      0x1b
#define WM8753_GPIO2      0x1c
#define WM8753_RESET      0x1f
#define WM8753_RECMIX1    0x20
#define WM8753_RECMIX2    0x21
#define WM8753_LOUTM1     0x22
#define WM8753_LOUTM2     0x23
#define WM8753_ROUTM1     0x24
#define WM8753_ROUTM2     0x25
#define WM8753_MOUTM1     0x26
#define WM8753_MOUTM2     0x27
#define WM8753_LOUT1V     0x28
#define WM8753_ROUT1V     0x29
#define WM8753_LOUT2V     0x2a
#define WM8753_ROUT2V     0x2b
#define WM8753_MOUTV      0x2c
#define WM8753_OUTCTL     0x2d
#define WM8753_ADCIN      0x2e
#define WM8753_INCTL1     0x2f
#define WM8753_INCTL2     0x30
#define WM8753_LINVOL     0x31
#define WM8753_RINVOL     0x32
#define WM8753_MICBIAS    0x33
#define WM8753_CLOCK      0x34
#define WM8753_PLL1CTL1   0x35
#define WM8753_PLL1CTL2   0x36
#define WM8753_PLL1CTL3   0x37
#define WM8753_PLL1CTL4   0x38
#define WM8753_PLL2CTL1   0x39
#define WM8753_PLL2CTL2   0x3a
#define WM8753_PLL2CTL3   0x3b
#define WM8753_PLL2CTL4   0x3c
#define WM8753_BIASCTL    0x3d
#define WM8753_ADCTL2     0x3f

#define WM8753_PLL1       0
#define WM8753_PLL2       1

/* DAC control */
#define WM8753_DACMUTE      1

/* digital hi-fi audio interface format */
#define WM8753_WL_32        3
#define WM8753_WL_24        2
#define WM8753_WL_20        1
#define WM8753_WL_16        0

#define WM8753_FT_DSP       (3 << 0)
#define WM8753_FT_I2S       (2 << 0)
#define WM8753_FT_LEFT      (1 << 0)
#define WM8753_FT_RIGHT     (0 << 0)

/* power management 1*/
#define VMIDSEL_DISABLED    0
#define VMIDSEL_50K         1
#define VMIDSEL_500K        2
#define VMIDSEL_5K          3

/* power management 2 */
#define MICAMP1EN           1
#define MICAMP2EN           1
#define ALCMIX              1
#define PGAL                1
#define PGAR                1
#define ADCL                1
#define ADCR                1
#define RXMIX               1
#define LINEMIX             1

/* power management 3*/
#define LOUT1               1
#define ROUT1               1

/* power management 4 */
#define RIGHTMIX            1
#define LEFTMIX             1

#define VREF                1
#define MICB                1
#define VDAC                1
#define DACL                1
#define DACR                1
#define DIGENB              0

/* clock inputs */
#define WM8753_MCLK         0
#define WM8753_PCMCLK       1

/* clock divider id's */
#define WM8753_PCMDIV       0
#define WM8753_BCLKDIV      1
#define WM8753_VXCLKDIV     2

/* left mixer control 1 */
#define LD2LO               1
#define LM2LO               1

/* left output mixer control 1 */
#define RD2RO               1
#define RM2RO               1

/* PCM clock dividers */
#define WM8753_PCM_DIV_1    (0 << 6)
#define WM8753_PCM_DIV_3    (2 << 6)
#define WM8753_PCM_DIV_5_5  (3 << 6)
#define WM8753_PCM_DIV_2    (4 << 6)
#define WM8753_PCM_DIV_4    (5 << 6)
#define WM8753_PCM_DIV_6    (6 << 6)
#define WM8753_PCM_DIV_8    (7 << 6)

/* BCLK clock dividers */
#define WM8753_BCLK_DIV_1   (0 << 3)
#define WM8753_BCLK_DIV_2   (1 << 3)
#define WM8753_BCLK_DIV_4   (2 << 3)
#define WM8753_BCLK_DIV_8   (3 << 3)
#define WM8753_BCLK_DIV_16  (4 << 3)

/* VXCLK clock dividers */
#define WM8753_VXCLK_DIV_1  (0 << 6)
#define WM8753_VXCLK_DIV_2  (1 << 6)
#define WM8753_VXCLK_DIV_4  (2 << 6)
#define WM8753_VXCLK_DIV_8  (3 << 6)
#define WM8753_VXCLK_DIV_16 (4 << 6)

#define WM8753_DAI_HIFI     0
#define WM8753_DAI_VOICE    1

#define WM8753_MAC1ALC      1

#define WM8753_I2C_ADDR     (0x1a)

#define CODEC_SET_BITSVAL(val, s, e, bitval)      \
    do{                                                 \
        uint32_t mask;                                    \
        mask = ((1u<<((e)-(s)+1)) - 1) << (s);            \
        (val) = ((val)&~mask) | (((bitval)<<(s))&mask);   \
    }while(0)

/*********************************************************************
 * TYPEDEFS
 */
struct reg_default {
    uint8_t reg;
    uint16_t def;
};

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static const uint16_t wm8753_regs[] = {
    /* 0x00 */ 0x0000,
    /* 0x01 */ 0x0008,
    /* 0x02 */ 0x0000,
    /* 0x03 */ 0x000a,
    /* 0x04 */ 0x000a,
    /* 0x05 */ 0x0033,
    /* 0x06 */ 0x0000,
    /* 0x07 */ 0x0007,
    /* 0x08 */ 0x00ff,
    /* 0x09 */ 0x00ff,
    /* 0x0a */ 0x000f,
    /* 0x0b */ 0x000f,
    /* 0x0c */ 0x007b,
    /* 0x0d */ 0x0000,
    /* 0x0e */ 0x0032,
    /* 0x0f */ 0x0000,
    /* 0x10 */ 0x00c3,
    /* 0x11 */ 0x00c3,
    /* 0x12 */ 0x00c0,
    /* 0x13 */ 0x0000,
    /* 0x14 */ 0x0000,
    /* 0x15 */ 0x0000,
    /* 0x16 */ 0x0000,
    /* 0x17 */ 0x0000,
    /* 0x18 */ 0x0000,
    /* 0x19 */ 0x0000,
    /* 0x1a */ 0x0000,
    /* 0x1b */ 0x0000,
    /* 0x1c */ 0x0000,
    /* 0x1d */ 0x0000,
    /* 0x1e */ 0x0000,
    /* 0x1f */ 0x0000,
    /* 0x20 */ 0x0055,
    /* 0x21 */ 0x0005,
    /* 0x22 */ 0x0050,
    /* 0x23 */ 0x0055,
    /* 0x24 */ 0x0050,
    /* 0x25 */ 0x0055,
    /* 0x26 */ 0x0050,
    /* 0x27 */ 0x0055,
    /* 0x28 */ 0x0079,
    /* 0x29 */ 0x0079,
    /* 0x2a */ 0x0079,
    /* 0x2b */ 0x0079,
    /* 0x2c */ 0x0079,
    /* 0x2d */ 0x0000,
    /* 0x2e */ 0x0005, //adc input mode line
    /* 0x2f */ 0x0000,
    /* 0x30 */ 0x0000,
    /* 0x31 */ 0x0097,
    /* 0x32 */ 0x0097,
    /* 0x33 */ 0x0000,
    /* 0x34 */ 0x0004,
    /* 0x35 */ 0x0000,
    /* 0x36 */ 0x0083,
    /* 0x37 */ 0x0024,
    /* 0x38 */ 0x01ba,
    /* 0x39 */ 0x0000,
    /* 0x3a */ 0x0083,
    /* 0x3b */ 0x0024,
    /* 0x3c */ 0x01ba,
    /* 0x3d */ 0x0000,
    /* 0x3e */ 0x0000,
    /* 0x3f */ 0x0000,
};


/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief codec_wm8753_write()
 *
 * @param[in] reg_addr  
 * @param[in] val  
 *
 * @return 
 **/
static void codec_wm8753_write(uint8_t reg_addr, uint16_t val)
{
    bool res;
    uint8_t txbuf[2];

    txbuf[0] = (reg_addr<<1) | ((val>>8)&1);
    txbuf[1] = (uint8_t)(val&0xFF);

    res = i2c_master_write(WM8753_I2C_ADDR, txbuf, 2);
    while(!res);
}

/**
 * @brief codec_wm8753_set()
 *
 * @param[in] reg_addr  
 * @param[in] start  
 * @param[in] end  
 * @param[in] val  
 *
 * @return 
 **/
static void codec_wm8753_set(uint8_t reg_addr, uint8_t start, uint8_t end, uint16_t val)
{
    uint16_t reg_value = wm8753_regs[reg_addr];
    CODEC_SET_BITSVAL(reg_value, start, end, val);
    codec_wm8753_write(reg_addr, reg_value);
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief codec_wm8753_init()
 *
 * @param[in] mode  
 * @param[in] sample_rate  
 * @param[in] mclk  
 *
 * @return 
 **/
bool codec_wm8753_init(codec_wm8753_mode_t mode,
                       codec_wm8753_sample_rate_t sample_rate,
                       codec_wm8753_mclk_t mclk)

{
    uint16_t srate;

    /*@{
     * DAC sample rate=48KHz
     * MCLK=24MHz, so select USB mode and set SR[4:0]=5'b11100
     * Because MCLK=24MHz, so select USB mode
     @}*/
    switch(mclk)
    {
        case CODEC_WM8753_MCLK_12MHZ:
            switch(sample_rate)
            {
                case CODEC_WM8753_SAMPLE_RATE_8KHZ:  srate=0x0D; break;
                case CODEC_WM8753_SAMPLE_RATE_16KHZ: srate=0x15; break;
                case CODEC_WM8753_SAMPLE_RATE_24KHZ: srate=0x39; break;
                case CODEC_WM8753_SAMPLE_RATE_48KHZ: srate=0x01; break;
                case CODEC_WM8753_SAMPLE_RATE_96KHZ: srate=0x1D; break;
                default: return false;
            }
            break;

        case CODEC_WM8753_MCLK_24MHZ:
            switch(sample_rate)
            {
                /* 16K:0x0D 24K:0x11 48K:0x39 96K:0x01 IN MCLK=24MHz*/
                case CODEC_WM8753_SAMPLE_RATE_16KHZ: srate=0x0D; break;
                case CODEC_WM8753_SAMPLE_RATE_24KHZ: srate=0x11; break;
                case CODEC_WM8753_SAMPLE_RATE_48KHZ: srate=0x39; break;
                case CODEC_WM8753_SAMPLE_RATE_96KHZ: srate=0x01; break;
                default: return false;
            }
            break;

        default:
            return false;
    }

    codec_wm8753_set(WM8753_RESET, 0, 8, 0);
    codec_wm8753_set(WM8753_DAC, 3, 3, WM8753_DACMUTE);

    codec_wm8753_set(WM8753_PWR1, 2, 8, (VMIDSEL_50K << 5) | (VREF<<4) | (DACL << 1) | DACR);
    codec_wm8753_set(WM8753_PWR3, 7, 8, (LOUT1 << 1) | ROUT1);
    codec_wm8753_set(WM8753_PWR4, 0, 1, (LEFTMIX << 1)| RIGHTMIX);
    codec_wm8753_set(WM8753_LOUTM1, 4, 8, (LD2LO<<4) | (0<<3) | 0);  /*left mixer volume +6dB*/
    codec_wm8753_set(WM8753_ROUTM1, 4, 8, (RD2RO<<4) | (0<<3) | 0);  /*right mixer volume +6dB*/
    codec_wm8753_set(WM8753_LOUT1V, 0, 8, 0x179); /*0dB*/
    codec_wm8753_set(WM8753_ROUT1V, 0, 8, 0x179); /*0dB*/
    codec_wm8753_set(WM8753_SRATE1, 0, 5, srate);/*set DAC sample rate*/

    /*@{
     * wm8753 acts as master, it can provide BCLK and LRCK.
     */
    if(mode == CODEC_WM8753_MASTER)
    {
        codec_wm8753_set(WM8753_HIFI, 0, 6, (1u << 6) | (WM8753_WL_16 << 2) | WM8753_FT_I2S); /*Enable HiFi interface master mode*/
        //sclk: {0:mclk 1:mclk/2 2:mclk/4 3:mclk/8 4::mclk/16}
        codec_wm8753_set(WM8753_SRATE2, 3, 5, 2);/*set BCLK=24MHz/8=3MHz*/
    }

    /*}@*/
    codec_wm8753_set(WM8753_DAC, 3, 3, ~WM8753_DACMUTE);

    return true;
}

/** @} */


