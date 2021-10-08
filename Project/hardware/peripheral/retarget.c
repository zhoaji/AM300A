/**
 * @file retarget.c
 * @brief 
 * @date 2016/04/15 19:35:33
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
#include <stdio.h>
#include "features.h"
#include "hs66xx.h"
#include "uart.h"

#ifdef CONFIG_SEGGER_RTT
#include "SEGGER_RTT.h"
#endif

/*********************************************************************
 * MACROS
 */
#define RETARGET_UART  HS_UART0

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

#if defined(__CC_ARM) ||  defined(__ICCARM__) || defined (__ARMCC_VERSION)

#if !defined(__GNUC__)
struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;
#endif

int ser_putchar (int c)
{
#ifdef CONFIG_SEGGER_RTT
    SEGGER_RTT_Write(0, (const char *)&c, 1);
#else
    if((char)c == '\n')
        uart_send_block(RETARGET_UART, (const uint8_t *)"\r", 1);
    uart_send_block(RETARGET_UART, (const uint8_t *)&c, 1);
#endif
    return c;
}

int ser_getchar (void)
{
  return (-1);
}

int fputc(int c, FILE *f)
{
    return ser_putchar(c);
}

int fgetc(FILE *f)
{
    return ser_getchar();
}

int ferror(FILE *f)
{
    return EOF;
}

void _ttywrch(int c)
{
    ser_putchar(c);
}

void _sys_exit(int return_code)
{
    while(1);
}

#elif defined(__GNUC__)

#include <sys/stat.h>

// Default _sbrk just return 'end' value (defined in linker).
caddr_t _sbrk(int incr)
{
#ifdef CONFIG_STACK_DEBUG
    // In Stack Debug mode, can't use the real the stack for sbrk, it will effect co_stack_check()
    static uint8_t fake_stack[2048];
    uint8_t * const stack_base  = fake_stack;
    uint8_t * const stack_limit = fake_stack + sizeof(fake_stack);
#else
    extern unsigned _stack;
    uint8_t * const stack_base  = (uint8_t *)&_stack;
    uint8_t * const stack_limit = (uint8_t *)__get_MSP();
#endif

    static uint8_t* psbrk = stack_base;
    uint8_t*        prev  = psbrk;
    uint8_t*        next  = psbrk + incr;

    if (next >= stack_limit)
        return (caddr_t)-1;

    psbrk = next;

    return (caddr_t) prev;
}

int _write(int file, const char * ptr, int len)
{
#ifdef CONFIG_SEGGER_RTT
    len = SEGGER_RTT_Write(0, ptr, len);
#else
    for(; len>0; --len, ++ptr)
    {
        if(*ptr == '\n')
            uart_send_block(RETARGET_UART, (const uint8_t *)"\r", 1);
        uart_send_block(RETARGET_UART, (const uint8_t *)ptr, 1);
    }
#endif
    return len;
}

int _read(int file, char * ptr, int len)
{
    return 0;
}

int _close(int file) {
    return -1;
}
int _lseek(int file, int offset, int whence) {
    return -1;
}
int _fstat(int file, struct stat *st) {
    return -1;
}
int _isatty(int file) {
    return 0;
}

#endif

/** @} */


