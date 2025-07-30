/**
 ******************************************************************************
 * @file       yLib_heap.h
 * @brief      FreeRTOS风格的动态内存管理器
 * @author     Ryan
 * @version    V2.0.0
 * @date       2025/06/16
 * @note
 ******************************************************************************
 */
#ifndef YLIB_HEAP_H
#define YLIB_HEAP_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "yLib_def.h"
#include "yLib_list.h"

/* 内存对齐宏 */
#define ylib_heap_align_up(size) (((size) + (configBYTE_ALIGNMENT - 1)) & ~(configBYTE_ALIGNMENT - 1))
#define ylib_heap_align_down(size) ((size) & ~(configBYTE_ALIGNMENT - 1))

/* 魔术数字用于检测内存损坏 */
#define YLIB_HEAP_MAGIC_FREE 0xDEADBEEF
#define YLIB_HEAP_MAGIC_ALLOC 0xABCDEF00

    /**
     * @brief 内存块头结构
     */
    typedef struct ylib_block_link
    {
        struct ylib_block_link *next_free_block; /* 下一个空闲块 */
        size_t block_size;                       /* 块大小（包含头部） */
#if configHEAP_CHECK_ENABLE
        uint32_t magic; /* 魔术数字 */
#endif
    } ylib_block_link_t;

    /**
     * @brief 堆统计信息
     */
    typedef struct
    {
        size_t total_heap_size;             /* 总堆大小 */
        size_t free_heap_size;              /* 空闲堆大小 */
        size_t minimum_ever_free_heap_size; /* 历史最小空闲大小 */
        size_t number_of_free_blocks;       /* 空闲块数量 */
        size_t max_block_size;              /* 最大空闲块大小 */
        size_t min_block_size;              /* 最小空闲块大小 */
        size_t successful_allocations;      /* 成功分配次数 */
        size_t successful_frees;            /* 成功释放次数 */
    } ylib_heap_stats_t;

    /**
     * @brief 堆管理器结构
     */
    typedef struct
    {
        uint8_t *heap_start;                 /* 堆起始地址 */
        uint8_t *heap_end;                   /* 堆结束地址 */
        ylib_block_link_t *free_blocks_list; /* 空闲块链表 */
        size_t heap_size;                    /* 堆大小 */
        bool initialized;                    /* 初始化标志 */
#if configHEAP_STATS_ENABLE
        ylib_heap_stats_t stats; /* 堆统计信息 */
#endif
    } ylib_heap_t;

    /* 全局堆管理器实例 */
    extern ylib_heap_t g_ylib_heap;

    /* 核心API */

    /**
     * @brief 初始化堆管理器
     * @param heap_buffer 堆缓冲区指针，NULL表示使用静态分配
     * @param heap_size 堆大小
     * @return 0成功，-1失败
     */
    int ylib_heap_init(void *heap_buffer, size_t heap_size);

    /**
     * @brief 分配内存
     * @param wanted_size 请求的内存大小
     * @return 分配的内存指针，失败返回NULL
     */
    void *ylib_malloc(size_t wanted_size);

    /**
     * @brief 释放内存
     * @param pv 要释放的内存指针
     */
    void ylib_free(void *pv);

    /**
     * @brief 重新分配内存
     * @param pv 原内存指针
     * @param wanted_size 新的内存大小
     * @return 新的内存指针，失败返回NULL
     */
    void *ylib_realloc(void *pv, size_t wanted_size);

    /**
     * @brief 分配并清零内存
     * @param num 元素数量
     * @param size 每个元素大小
     * @return 分配的内存指针，失败返回NULL
     */
    void *ylib_calloc(size_t num, size_t size);

    /**
     * @brief 获取空闲堆大小
     * @return 空闲堆大小
     */
    size_t ylib_get_free_heap_size(void);

    /**
     * @brief 获取历史最小空闲堆大小
     * @return 历史最小空闲堆大小
     */
    size_t ylib_get_minimum_ever_free_heap_size(void);

#if configHEAP_STATS_ENABLE
    /**
     * @brief 获取堆统计信息
     * @param stats 统计信息结构指针
     */
    void ylib_get_heap_stats(ylib_heap_stats_t *stats);
#endif

#if configHEAP_CHECK_ENABLE
    /**
     * @brief 检查堆完整性
     * @return 1表示完整，0表示损坏
     */
    int ylib_heap_check_integrity(void);

    /**
     * @brief 打印堆信息（调试用）
     */
    void ylib_heap_print_info(void);
#endif

    /* 高级API */

    /**
     * @brief 获取内存块大小
     * @param pv 内存指针
     * @return 内存块大小，失败返回0
     */
    size_t ylib_malloc_size(void *pv);

    /*
     * 注意：内存池相关函数在 yLib_mempool.h 中定义
     * 这里只提供与堆管理器集成的声明
     * 具体实现和完整API请参考 yLib_mempool.h
     */

    /* 钩子函数类型定义 */
    typedef void (*ylib_malloc_failed_hook_t)(void);
    typedef void (*ylib_free_hook_t)(void *pv, size_t size);
    typedef void (*ylib_malloc_hook_t)(void *pv, size_t size);

    /* 钩子函数设置 */
    void ylib_set_malloc_failed_hook(ylib_malloc_failed_hook_t hook);
    void ylib_set_free_hook(ylib_free_hook_t hook);
    void ylib_set_malloc_hook(ylib_malloc_hook_t hook);

    /* 内部辅助函数 */
    static inline void *ylib_heap_get_user_ptr(ylib_block_link_t *block)
    {
        return (void *)((uint8_t *)block + sizeof(ylib_block_link_t));
    }

    static inline ylib_block_link_t *ylib_heap_get_block_ptr(void *user_ptr)
    {
        return (ylib_block_link_t *)((uint8_t *)user_ptr - sizeof(ylib_block_link_t));
    }

/* 快速分配宏（类似FreeRTOS） */
#define pvPortMalloc(size) ylib_malloc(size)
#define vPortFree(ptr) ylib_free(ptr)
#define xPortGetFreeHeapSize() ylib_get_free_heap_size()
#define xPortGetMinimumEverFreeHeapSize() ylib_get_minimum_ever_free_heap_size()

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* YLIB_HEAP_H */
