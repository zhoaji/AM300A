/*********************************************************************
 * @file co_mem.h
 * @brief 
 * @version 1.0
 * @date 14/11/4 13:50:42
 * @author liqiang
 *
 * @note 
 */

#ifndef __CO_MEM_H__
#define __CO_MEM_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "co.h"

/*********************************************************************
 * MACROS
 */

#ifdef CONFIG_HEAP_DEBUG
#define MEM_DEBUG
#endif

#ifdef CONFIG_ALLOC_CALLER_API
#define MEM_CALLER_API
#endif

#ifdef CONFIG_ALLOC_CALLER_API_FUNC_PARAM
#define MEM_CALLER_API_FUNC_PARAM
#endif

#ifdef CONFIG_HEAP_DEBUG
#define MEM_CALLER_RECORD
#endif

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
    int cur_used_size;
    int cur_idle_size;
    int extr_struct_size;

    void *first_available_addr;
    int mem_pool_size;

    int max_mem_pool_usage_size;
}mem_info_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief mem_init()
 *
 * @param[in] firstAvailablyAddr  
 * @param[in] size  
 *
 * @return 
 **/
void mem_init(void *firstAvailablyAddr, int size);

/**
 * @brief mem_free()
 *
 * @param[in] pfree  
 *
 * @return 
 **/
void mem_free(void *pfree);

/**
 * @brief mem_inquire_info()
 *
 * @param[in] pmemInfo  
 *
 * @return 
 **/
void mem_inquire_info(mem_info_t *pmemInfo);

/**
 * @brief mem_idle_size()
 *
 * @return 
 **/
int mem_idle_size(void);

/**
 * @brief mem_is_ok()
 *
 * @return 
 **/
bool mem_is_ok(void);

/*********************************************************************
 * MACROS
 */

#ifdef MEM_CALLER_API
#ifdef MEM_CALLER_API_FUNC_PARAM
void *mem_malloc(int size, const char *pfile, const char *pfunction, int line);
void *mem_calloc(int size, const char *pfile, const char *pfunction, int line);
void *mem_realloc(void *pfree, int size, const char *pfile, const char *pfunction, int line);
#define MEM_MALLOC(s)     mem_malloc (s,    __FILE__, __FUNCTION__, __LINE__)
#define MEM_CALLOC(s)     mem_calloc (s,    __FILE__, __FUNCTION__, __LINE__)
#define MEM_REALLOC(p, s) mem_realloc(p, s, __FILE__, __FUNCTION__, __LINE__)
#else
void *mem_malloc(int size, const char *pfile, int line);
void *mem_calloc(int size, const char *pfile, int line);
void *mem_realloc(void *pfree, int size, const char *pfile, int line);
#define MEM_MALLOC(s)     mem_malloc (s,    __FILE__, __LINE__)
#define MEM_CALLOC(s)     mem_calloc (s,    __FILE__, __LINE__)
#define MEM_REALLOC(p, s) mem_realloc(p, s, __FILE__, __LINE__)
#endif
#else
void *mem_malloc(int size);
void *mem_calloc(int size);
void *mem_realloc(void *pfree, int size);
#define MEM_MALLOC(s)     mem_malloc (s)
#define MEM_CALLOC(s)     mem_calloc (s)
#define MEM_REALLOC(p, s) mem_realloc(p, s)
#endif

#define MEM_FREE(p)       mem_free(p)

/*********************************************************************
 * EXTERN FUNCTIONS MACRO
 */

//#define co_malloc                                      MEM_MALLOC
//#define co_calloc                                      MEM_CALLOC
//#define co_realloc                                     MEM_REALLOC
//#define co_free                                        MEM_FREE
//
//#define co_mem_init                                    mem_init
//#define co_mem_inquire_info                            mem_inquire_info


#ifdef __cplusplus
}
#endif

#endif

