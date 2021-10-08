/**
 * @file co_allocator.h
 * @brief allocator
 * @date Sat 17 Jan 2015 01:52:06 PM CST
 * @author liqiang
 *
 * @defgroup CO_ALLOCATOR Allocator
 * @ingroup COROUTINE
 * @brief Allocator Module
 * @details Provide malloc, calloc, realloc, free function
 *
 * @{
 */

#ifndef __CO_ALLOCATOR_H__
#define __CO_ALLOCATOR_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "stdint.h"
#include "co.h"

/*********************************************************************
 * MACROS
 */

/// @cond
#ifdef CONFIG_HEAP_DEBUG
#define CONFIG_ALLOC_USE_HOOK
#define CONFIG_ALLOC_USE_CO_MEM
#define CONFIG_ALLOC_CALLER_API
#define CONFIG_ALLOC_CALLER_API_FUNC_PARAM
#else
#define CONFIG_ALLOC_USE_HOOK
//#define CONFIG_ALLOC_USE_CO_MEM
#define CONFIG_ALLOC_CALLER_API
//#define CONFIG_ALLOC_CALLER_API_FUNC_PARAM
#endif
/// @endcond

/*********************************************************************
 * TYPEDEFS
 */
/// @cond

#ifdef CONFIG_ALLOC_USE_HOOK

#ifdef CONFIG_ALLOC_CALLER_API
#ifdef CONFIG_ALLOC_CALLER_API_FUNC_PARAM
typedef void * (* co_allocator_malloc_t) (uint32_t size,              uint8_t type, const char *pfile, const char *pfunction, int line);
typedef void * (* co_allocator_calloc_t) (uint32_t size,              uint8_t type, const char *pfile, const char *pfunction, int line);
typedef void * (* co_allocator_realloc_t)(void *pfree, uint32_t size, uint8_t type, const char *pfile, const char *pfunction, int line);
#else
typedef void * (* co_allocator_malloc_t) (uint32_t size,              uint8_t type, const char *pfile, int line);
typedef void * (* co_allocator_calloc_t) (uint32_t size,              uint8_t type, const char *pfile, int line);
typedef void * (* co_allocator_realloc_t)(void *pfree, uint32_t size, uint8_t type, const char *pfile, int line);
#endif
#else
typedef void * (* co_allocator_malloc_t) (uint32_t size, uint8_t type);
typedef void * (* co_allocator_calloc_t) (uint32_t size, uint8_t type);
typedef void * (* co_allocator_realloc_t)(void *pfree, uint32_t size, uint8_t type);
#endif

typedef void (* co_allocator_init_t)(uint8_t type, uint8_t *first_availably_addr, uint16_t size);
typedef void (* co_allocator_free_t)(void *pfree);
typedef bool (* co_check_malloc)(uint32_t size, uint8_t type);
typedef bool (* co_mem_is_empty)(uint8_t type);
typedef bool (* co_is_free)(void* mem_ptr);

typedef struct
{
    co_allocator_init_t    init;
    co_allocator_malloc_t  malloc;
    co_allocator_calloc_t  calloc;
    co_allocator_realloc_t realloc;
    co_allocator_free_t    free;

    co_check_malloc        check_malloc;
    co_mem_is_empty        mem_is_empty;
    co_is_free             is_free;
}co_allocator_t;

typedef void (* co_allocator_mem_info_callback_t)(uint8_t **base, uint32_t *size);

typedef struct
{
    co_allocator_t allocator;
}co_allocator_env_t;

/*********************************************************************
 * EXTERN VARIABLES
 */

extern co_allocator_env_t co_allocator_env;

/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief co_allocator_init()
 *
 * @return 
 **/
void co_allocator_init(void);

/// @endcond

/**
 * @brief register a new allocator
 *
 * @note Must be called before stack initialization
 *
 * @param[in] allocator  allocator object
 * @param[in] mem_info_cb  memery info callback
 *
 * @return None
 **/
void co_allocator_register(co_allocator_t *allocator, co_allocator_mem_info_callback_t mem_info_cb);

/*********************************************************************
 * MACROS FUNCTION
 */

/// @cond
#ifdef CONFIG_ALLOC_CALLER_API
#ifdef CONFIG_ALLOC_CALLER_API_FUNC_PARAM
#define ke_malloc(s,t)    ((co_allocator_env.allocator.malloc) ((s),      (t), __FILE__, __FUNCTION__, __LINE__))
#define co_malloc(s)      ((co_allocator_env.allocator.malloc) ((s),      (0), __FILE__, __FUNCTION__, __LINE__))
#define co_calloc(s)      ((co_allocator_env.allocator.calloc) ((s),      (0), __FILE__, __FUNCTION__, __LINE__))
#define co_realloc(p,s)   ((co_allocator_env.allocator.realloc)((p), (s), (0), __FILE__, __FUNCTION__, __LINE__))
#else
#define ke_malloc(s,t)    ((co_allocator_env.allocator.malloc) ((s),      (t), __FILE__, __LINE__))
#define co_malloc(s)      ((co_allocator_env.allocator.malloc) ((s),      (0), __FILE__, __LINE__))
#define co_calloc(s)      ((co_allocator_env.allocator.calloc) ((s),      (0), __FILE__, __LINE__))
#define co_realloc(p,s)   ((co_allocator_env.allocator.realloc)((p), (s), (0), __FILE__, __LINE__))
#endif
#else
/// @endcond

/**
 * @brief malloc implement
 *
 * @param[in] s  malloc size
 *
 * @return malloc buffer
 **/
#define co_malloc(s)        ((co_allocator_env.allocator.malloc) ((s), (0)))
/// @cond
#define ke_malloc(s, t)     ((co_allocator_env.allocator.malloc) ((s), (t)))
/// @endcond

/**
 * @brief calloc implement
 *
 * @param[in] s  calloc size
 *
 * @return calloc buffer
 **/
#define co_calloc(s)       ((co_allocator_env.allocator.calloc) ((s), (0)))

/**
 * @brief realloc implement
 *
 * @param[in] p  realloc buffer
 * @param[in] s  realloc size
 *
 * @return realloc buffer
 **/
#define co_realloc(p, s)   ((co_allocator_env.allocator.realloc)((p), (s), (0)))

/// @cond
#endif
/// @endcond

/**
 * @brief free implement
 *
 * @param[in] p  free size
 *
 * @return None
 **/
#define co_free(p)             ((co_allocator_env.allocator.free)   ((p)))
/// @cond
#define ke_free(p)             ((co_allocator_env.allocator.free)   ((p)))
/// @endcond

/// @cond
#define ke_mem_init(t, h, s)   ((co_allocator_env.allocator.init)        ((t), (h), (s)))
#define ke_check_malloc(s, t)  ((co_allocator_env.allocator.check_malloc)((s), (t)))
#define ke_mem_is_empty(t)     ((co_allocator_env.allocator.mem_is_empty)((t)))
#define ke_is_free(p)          ((co_allocator_env.allocator.is_free)     ((p)))
/// @endcond

/// @cond
#else

#define ke_mem_init(t, h, s)   rw_ke_mem_init((t), (h), (s))
#define ke_malloc(s,t)         rw_ke_malloc((s), (t))
#define ke_free(p)             rw_ke_free((p))
#define ke_check_malloc(s, t)  rw_ke_check_malloc((s), (t))
#define ke_mem_is_empty(t)     rw_ke_mem_is_empty((t))
#define ke_is_free(p)          rw_ke_is_free((p))
#define co_malloc(s)           rw_ke_malloc((s),      (0))
#define co_free(p)             rw_ke_free((p))

#endif
/// @endcond

#ifdef __cplusplus
}
#endif

#endif

/** @} */

