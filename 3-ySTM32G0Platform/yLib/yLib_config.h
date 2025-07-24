/**
 ******************************************************************************
 * @file       yLib_config.h
 * @brief      YLib库的统一配置文件
 * @author     Ryan
 * @version    V2.0.0
 * @date       2025/06/16
 * @note       此文件包含YLib库各个模块的配置选项
 ******************************************************************************
 */
#ifndef YLIB_CONFIG_H
#define YLIB_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* =============================================================================
 * 通用配置
 * =============================================================================
 */

/**
 * @brief 调试和日志配置
 */
#ifndef DEBUG
// #define DEBUG                   1       /* 取消注释以启用调试模式 */
#endif

#ifndef ENABLE_COLOR_OUTPUT
// #define ENABLE_COLOR_OUTPUT     1       /* 取消注释以启用彩色输出 */
#endif

/**
 * @brief 调试和日志宏定义
 */
#ifdef DEBUG
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#define DEBUG_LOG(...) fprintf(stderr, "[DEBUG] " __VA_ARGS__)
#define ERROR_LOG(...) fprintf(stderr, "[ERROR] " __VA_ARGS__)
#define WARN_LOG(...) fprintf(stderr, "[WARN] " __VA_ARGS__)
#define INFO_LOG(...) fprintf(stderr, "[INFO] " __VA_ARGS__)
#else
#define LOG(...) ((void)0)
#define DEBUG_LOG(...) ((void)0)
#define ERROR_LOG(...) ((void)0)
#define WARN_LOG(...) ((void)0)
#define INFO_LOG(...) ((void)0)
#endif

/**
 * @brief 带文件名和行号的调试宏定义
 */
#ifdef DEBUG
#define DBG_PRINT(fmt, ...) fprintf(stderr, "[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define DBG_ENTER() fprintf(stderr, "[ENTER] %s:%d %s()\n", __FILE__, __LINE__, __func__)
#define DBG_EXIT() fprintf(stderr, "[EXIT] %s:%d %s()\n", __FILE__, __LINE__, __func__)
#else
#define DBG_PRINT(fmt, ...) ((void)0)
#define DBG_ENTER() ((void)0)
#define DBG_EXIT() ((void)0)
#endif

/**
 * @brief 断言配置
 */
#ifndef NDEBUG
// #define NDEBUG                  1       /* 取消注释以禁用断言 */
#endif

/**
 * @brief 断言宏定义
 */
#if defined(_ASSERT_H) && !defined(NDEBUG)
#define ASSERT(x) ((void)0)
#define ASSERT_MSG(x, msg) ((void)0)
#define STATIC_ASSERT(x, msg) ((void)0)
#else
#define ASSERT(x) ((void)0)
#define ASSERT_MSG(x, msg) ((void)0)
#define STATIC_ASSERT(x, msg) ((void)0)
#endif

/* =============================================================================
 * 内存堆管理器配置 (yLib_heap)
 * =============================================================================
 */

/**
 * @brief 堆总大小配置
 */
#ifndef YLIB_TOTAL_HEAP_SIZE
#define YLIB_TOTAL_HEAP_SIZE (32 * 1024) /* 默认堆大小32KB */
#endif

/**
 * @brief 最小块大小配置
 */
#ifndef configMINIMAL_BLOCK_SIZE
#define configMINIMAL_BLOCK_SIZE (16) /* 最小块大小 */
#endif

/**
 * @brief 字节对齐配置
 */
#ifndef configBYTE_ALIGNMENT
#define configBYTE_ALIGNMENT (8) /* 字节对齐 */
#endif

/**
 * @brief 堆统计功能配置
 */
#ifndef configHEAP_STATS_ENABLE
#define configHEAP_STATS_ENABLE (1) /* 启用堆统计 */
#endif

/**
 * @brief 堆检查功能配置
 */
#ifndef configHEAP_CHECK_ENABLE
#define configHEAP_CHECK_ENABLE (1) /* 启用堆检查 */
#endif

/* =============================================================================
 * 内存池管理器配置 (yLib_mempool)
 * =============================================================================
 */

/**
 * @brief 内存分区名称功能配置
 */
#ifndef YLIB_MEM_NAME_EN
#define YLIB_MEM_NAME_EN 1u /* 启用内存分区名称 */
#endif

/**
 * @brief 内存分区名称最大长度配置
 */
#ifndef YLIB_MEM_NAME_SIZE
#define YLIB_MEM_NAME_SIZE 16u /* 内存分区名称最大长度 */
#endif

/* =============================================================================
 * FIFO队列配置 (yLib_fifo)
 * =============================================================================
 */

/**
 * @brief FIFO队列默认容量
 */
#ifndef YLIB_FIFO_DEFAULT_CAPACITY
#define YLIB_FIFO_DEFAULT_CAPACITY 256 /* 默认FIFO容量 */
#endif

/**
 * @brief FIFO线程安全配置
 */
#ifndef YLIB_FIFO_THREAD_SAFE
#define YLIB_FIFO_THREAD_SAFE 0 /* 0=不启用线程安全, 1=启用线程安全 */
#endif

/* =============================================================================
 * 环形缓冲区配置 (yLib_ring)
 * =============================================================================
 */

/**
 * @brief 环形缓冲区默认大小
 */
#ifndef YLIB_RING_DEFAULT_SIZE
#define YLIB_RING_DEFAULT_SIZE 1024 /* 默认环形缓冲区大小 */
#endif

/**
 * @brief 环形缓冲区是否启用原子操作
 */
#ifndef YLIB_RING_ATOMIC_OPS
#define YLIB_RING_ATOMIC_OPS 0 /* 0=不启用, 1=启用原子操作 */
#endif

/* =============================================================================
 * 红黑树配置 (yLib_rbtree)
 * =============================================================================
 */

/**
 * @brief 红黑树是否启用递归删除
 */
#ifndef YLIB_RBTREE_RECURSIVE_DELETE
#define YLIB_RBTREE_RECURSIVE_DELETE 1 /* 0=迭代删除, 1=递归删除 */
#endif

/**
 * @brief 红黑树验证功能配置
 */
#ifndef YLIB_RBTREE_VERIFY_ENABLE
#define YLIB_RBTREE_VERIFY_ENABLE 1 /* 启用红黑树验证 */
#endif

/* =============================================================================
 * 链表配置 (yLib_list)
 * =============================================================================
 */

/**
 * @brief 链表安全检查配置
 */
#ifndef YLIB_LIST_SAFE_CHECK
#define YLIB_LIST_SAFE_CHECK 1 /* 启用链表安全检查 */
#endif

/**
 * @brief 链表魔术数字验证
 */
#ifndef YLIB_LIST_MAGIC_CHECK
#define YLIB_LIST_MAGIC_CHECK 1 /* 启用魔术数字检查 */
#endif

/* =============================================================================
 * 性能和优化配置
 * =============================================================================
 */

/**
 * @brief 内存对齐优化
 */
#ifndef YLIB_MEMORY_ALIGNMENT
#define YLIB_MEMORY_ALIGNMENT 8 /* 内存对齐字节数 */
#endif

/**
 * @brief 缓存行大小配置
 */
#ifndef YLIB_CACHE_LINE_SIZE
#define YLIB_CACHE_LINE_SIZE 64 /* 缓存行大小 */
#endif

/**
 * @brief 是否启用分支预测优化
 */
#ifndef YLIB_BRANCH_PREDICTION
#define YLIB_BRANCH_PREDICTION 1 /* 启用分支预测优化 */
#endif

/* =============================================================================
 * 错误处理配置
 * =============================================================================
 */

/**
 * @brief 详细错误信息配置
 */
#ifndef YLIB_VERBOSE_ERRORS
#define YLIB_VERBOSE_ERRORS 1 /* 启用详细错误信息 */
#endif

/**
 * @brief 错误日志记录配置
 */
#ifndef YLIB_ERROR_LOGGING
#define YLIB_ERROR_LOGGING 1 /* 启用错误日志记录 */
#endif

/* =============================================================================
 * 统计和监控配置
 * =============================================================================
 */

/**
 * @brief 运行时统计配置
 */
#ifndef YLIB_RUNTIME_STATS
#define YLIB_RUNTIME_STATS 1 /* 启用运行时统计 */
#endif

/**
 * @brief 内存使用统计配置
 */
#ifndef YLIB_MEMORY_STATS
#define YLIB_MEMORY_STATS 1 /* 启用内存使用统计 */
#endif

/**
 * @brief 性能计数器配置
 */
#ifndef YLIB_PERFORMANCE_COUNTERS
#define YLIB_PERFORMANCE_COUNTERS 0 /* 0=禁用, 1=启用性能计数器 */
#endif

/* =============================================================================
 * 平台特定配置
 * =============================================================================
 */

/**
 * @brief 目标平台配置
 */
#ifndef YLIB_TARGET_PLATFORM
#define YLIB_TARGET_PLATFORM "Generic" /* 目标平台名称 */
#endif

/**
 * @brief 是否为嵌入式环境
 */
#ifndef YLIB_EMBEDDED_ENVIRONMENT
#define YLIB_EMBEDDED_ENVIRONMENT 0 /* 0=非嵌入式, 1=嵌入式环境 */
#endif

/**
 * @brief 栈大小限制配置
 */
#ifndef YLIB_STACK_SIZE_LIMIT
#define YLIB_STACK_SIZE_LIMIT (8 * 1024) /* 栈大小限制 */
#endif

#ifdef __cplusplus
}
#endif

#endif /* YLIB_CONFIG_H */
