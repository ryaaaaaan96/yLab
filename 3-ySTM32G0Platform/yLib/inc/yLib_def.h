/**
 ******************************************************************************
 * @file       yLib_def.h
 * @brief      提供 yLab库相关的底层定义
 * @author     Ryan
 * @version    V2.0.0
 * @date       2025/06/16
 * @note
 ******************************************************************************
 */
#ifndef YLIB_DEF_H
#define YLIB_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* include -------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "yLib_config.h" /* 统一配置文件 */

/**
 * @brief YLib库版本信息
 */
#define YLIB_VERSION_MAJOR 2
#define YLIB_VERSION_MINOR 0
#define YLIB_VERSION_PATCH 0
#define YLIB_VERSION_STR "2.0.0"

/**
 * @brief 编译器相关宏定义
 * @note 适配不同编译器的section、used、对齐、弱符号、内联等属性
 */
#if defined(__CC_ARM) || defined(__CLANG_ARM) /* ARM 编译器 */
#define YLIB_SECTION(x) __attribute__((section(x)))
#define YLIB_USED __attribute__((used))
#define YLIB_ALIGN(n) __attribute__((aligned(n)))
#define YLIB_WEAK __attribute__((weak))
#define YLIB_INLINE static __inline
#elif defined(__IAR_SYSTEMS_ICC__) /* IAR 编译器 */
#define YLIB_SECTION(x) @x
#define YLIB_USED __root
#define YLIB_PRAGMA(x) _Pragma(#x)
#define YLIB_ALIGN(n) YLAB_PRAGMA(data_alignment = n)
#define YLIB_WEAK __weak
#define YLIB_INLINE static inline
#elif defined(__GNUC__) /* GCC 编译器 */
#define YLIB_SECTION(x) __attribute__((section(x), used))
#define YLIB_USED __attribute__((used))
#define YLIB_ALIGN(n) __attribute__((aligned(n)))
#define YLIB_WEAK __attribute__((weak))
#define YLIB_INLINE static inline
#define YLIB_ALWAYSINLINE static __attribute__((always_inline))

#else
#error "当前编译器不受支持！"
#endif

/**
 * @brief 宏连接和命名工具
 */
#define CAT(a, b) a##b
#define PASTE(a, b) CAT(a, b)
#define JOIN(prefix, name) PASTE(prefix, PASTE(_, name))
#define _JOIN(prefix, name) PASTE(_, PASTE(prefix, PASTE(_, name)))

/**
 * @brief 通用交换宏，适用于任何类型
 * @param TYPE  要交换的变量的数据类型
 * @param a     指向第一个变量的指针
 * @param b     指向第二个变量的指针
 * @note 使用do-while(0)包装确保宏在任何上下文中都能安全使用
 */
#define SWAP(TYPE, a, b)  \
    do                    \
    {                     \
        TYPE temp = *(a); \
        *(a) = *(b);      \
        *(b) = temp;      \
    } while (0)

/**
 * @brief 无临时变量的异或交换宏（仅适用于整数类型）
 * @param a     指向第一个整数变量的指针
 * @param b     指向第二个整数变量的指针
 * @note 当a和b指向同一内存地址时会出错，使用前需要检查
 */
#define SWAP_XOR(a, b)    \
    do                    \
    {                     \
        if ((a) != (b))   \
        {                 \
            *(a) ^= *(b); \
            *(b) ^= *(a); \
            *(a) ^= *(b); \
        }                 \
    } while (0)

/**
 * @brief 指针交换宏
 * @param a     指向第一个指针的指针
 * @param b     指向第二个指针的指针
 */
#define SWAP_PTR(a, b)     \
    do                     \
    {                      \
        void *temp = *(a); \
        *(a) = *(b);       \
        *(b) = temp;       \
    } while (0)

/**
 * @brief 计算数组元素个数
 * @param a     要计算长度的数组
 * @note 只能用于数组，不能用于指针
 */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

/**
 * @brief 安全的数组长度计算宏（编译时检查）
 * @param a     要计算长度的数组
 * @note 如果传入指针会产生编译错误
 */
#define SAFE_ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)) + sizeof(*(a)) * 0)

/**
 * @brief 计算结构体成员的偏移量
 * @param type      结构体类型
 * @param member    成员名称
 */
#define OFFSETOF(type, member) ((size_t)&(((type *)0)->member))

/**
 * @brief 通过结构体成员获取结构体指针
 * @param ptr       指向成员的指针
 * @param type      结构体类型
 * @param member    成员名称
 */
#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - OFFSETOF(type, member)))

/**
 * @brief Linux内核风格的container_of宏
 * @param ptr       指向成员的指针
 * @param type      结构体类型
 * @param member    成员名称
 */
#define container_of(ptr, type, member) CONTAINER_OF(ptr, type, member)

/**
 * @brief 安全释放内存并调用自定义释放函数
 * @param ptr       要释放的指针
 * @param free_func 释放函数
 */
#define SAFE_FREE_FUNC(ptr, free_func) \
    do                                 \
    {                                  \
        if (ptr)                       \
        {                              \
            free_func(ptr);            \
            (ptr) = NULL;              \
        }                              \
    } while (0)

/**
 * @brief 运行时检查宏
 */
#define CHECK(x)          \
    do                    \
    {                     \
        if (!(x))         \
            return false; \
    } while (0)
#define CHECK_PTR(ptr)   \
    do                   \
    {                    \
        if (!(ptr))      \
            return NULL; \
    } while (0)
#define CHECK_RET(x, ret) \
    do                    \
    {                     \
        if (!(x))         \
            return (ret); \
    } while (0)
#define CHECK_GOTO(x, label) \
    do                       \
    {                        \
        if (!(x))            \
            goto label;      \
    } while (0)

/**
 * @brief 编译器特定的分支预测提示
 * @note  这些宏有助于在支持的编译器上优化分支预测
 */
#if __GNUC__ >= 3 && !defined _WIN32
#define LIKELY(x) __builtin_expect((long)(x) != 0, 1)
#define UNLIKELY(x) __builtin_expect((long)(x) != 0, 0)
#else
#define LIKELY(x) x
#define UNLIKELY(x) x
#endif

/**
 * @brief 最大值宏
 * @param a     要比较的第一个值
 * @param b     要比较的第二个值
 * @return      两个值中较大的一个
 * @note 使用typeof确保类型安全，避免副作用
 */
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/**
 * @brief 三值比较宏
 */
#define MAX3(a, b, c) MAX(MAX(a, b), c)
#define MIN3(a, b, c) MIN(MIN(a, b), c)

/**
 * @brief 绝对值宏
 */
#ifndef ABS
#if defined(__GNUC__) || defined(__clang__)
#define ABS(x) ({ typeof(x) _x = (x); _x < 0 ? -_x : _x; })
#else
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif
#endif

/**
 * @brief 限制值在指定范围内
 */
#define CLAMP(x, min, max) MAX(min, MIN(max, x))

/**
 * @brief 符号函数
 */
#define SIGN(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

/**
 * @brief 判断数值是否在范围内
 */
#define IN_RANGE(x, min, max) ((x) >= (min) && (x) <= (max))

/**
 * @brief 向上取整到最近的2的幂次
 */
#define ROUND_UP_POW2(x)    \
    ({                      \
        typeof(x) _x = (x); \
        _x--;               \
        _x |= _x >> 1;      \
        _x |= _x >> 2;      \
        _x |= _x >> 4;      \
        _x |= _x >> 8;      \
        _x |= _x >> 16;     \
        _x++;               \
        _x;                 \
    })

/**
 * @brief 向上对齐到指定边界
 */
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

/**
 * @brief 判断是否为2的幂次
 */
#define IS_POWER_OF_2(x) ((x) != 0 && ((x) & ((x) - 1)) == 0)

/**
 * @brief 位操作宏
 */
#define YLAB_BIT(n) (1UL << (n))
#define YLAB_SET_BIT(x, n) ((x) |= BIT(n))
#define YLAB_CLEAR_BIT(x, n) ((x) &= ~BIT(n))
#define YLAB_TOGGLE_BIT(x, n) ((x) ^= BIT(n))
#define YLAB_TEST_BIT(x, n) (((x) & BIT(n)) != 0)
#define YLAB_MASK(n) (BIT(n) - 1)
#define YLAB_SET_BITS(x, mask) ((x) |= (mask))
#define YLAB_CLEAR_BITS(x, mask) ((x) &= ~(mask))
#define YLAB_TOGGLE_BITS(x, mask) ((x) ^= (mask))

/**
 * @brief 字节序转换宏
 */
#define SWAP16(x) (((x) << 8) | ((x) >> 8))
#define SWAP32(x) (((x) << 24) | (((x) << 8) & 0x00FF0000) | \
                   (((x) >> 8) & 0x0000FF00) | ((x) >> 24))

/**
 * @brief 字符串相关宏
 */
#define STR(x) #x
#define XSTR(x) STR(x)
#define STRCAT(a, b) a##b
#define STRLEN_CONST(s) (sizeof(s) - 1)

/**
 * @brief 安全字符串操作宏
 */
#define SAFE_STRNCPY(dest, src, size)   \
    do                                  \
    {                                   \
        strncpy(dest, src, (size) - 1); \
        (dest)[(size) - 1] = '\0';      \
    } while (0)

#define SAFE_STRNCAT(dest, src, size)                  \
    do                                                 \
    {                                                  \
        size_t dest_len = strlen(dest);                \
        if (dest_len < (size) - 1)                     \
        {                                              \
            strncat(dest, src, (size) - dest_len - 1); \
        }                                              \
    } while (0)

/**
 * @brief 时间和性能测量宏
 */
#ifdef DEBUG
#include <time.h>
#define TIME_START() clock_t _start_time = clock()
#define TIME_END(msg)                                                           \
    do                                                                          \
    {                                                                           \
        clock_t _end_time = clock();                                            \
        double _elapsed = ((double)(_end_time - _start_time)) / CLOCKS_PER_SEC; \
        fprintf(stderr, "[TIME] %s: %.6f seconds\n", msg, _elapsed);            \
    } while (0)
#else
#define TIME_START() ((void)0)
#define TIME_END(msg) ((void)0)
#endif

/**
 * @brief 循环相关宏
 */
#define FOR_EACH(item, array)                                  \
    for (size_t keep = 1, count = 0, size = ARRAY_SIZE(array); \
         keep && count != size;                                \
         keep = !keep, count++)                                \
        for (item = (array) + count; keep; keep = !keep)

#define REPEAT(n) for (size_t _i = 0; _i < (n); _i++)

/**
 * @brief 类型检查宏
 */
#define SAME_TYPE(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define IS_ARRAY(a) (!SAME_TYPE((a), &(a)[0]))

/**
 * @brief 错误处理宏
 */
#define RETURN_IF_NULL(ptr) \
    do                      \
    {                       \
        if (!(ptr))         \
            return;         \
    } while (0)
#define RETURN_VAL_IF_NULL(ptr, val) \
    do                               \
    {                                \
        if (!(ptr))                  \
            return (val);            \
    } while (0)
#define GOTO_IF_NULL(ptr, label) \
    do                           \
    {                            \
        if (!(ptr))              \
            goto label;          \
    } while (0)

/**
 * @brief 通用工具宏
 */
#define STRINGIFY(x) #x
#define EXPAND(x) x
#define COUNT_ARGS(...) (sizeof((int[]){__VA_ARGS__}) / sizeof(int))

/**
 * @brief 颜色输出宏（仅在支持ANSI转义序列的终端中有效）
 */
#ifdef ENABLE_COLOR_OUTPUT
#define COLOR_RESET "\033[0m"
#define COLOR_BLACK "\033[30m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BOLD "\033[1m"

#define PRINT_RED(...) printf(COLOR_RED __VA_ARGS__ COLOR_RESET)
#define PRINT_GREEN(...) printf(COLOR_GREEN __VA_ARGS__ COLOR_RESET)
#define PRINT_YELLOW(...) printf(COLOR_YELLOW __VA_ARGS__ COLOR_RESET)
#define PRINT_BLUE(...) printf(COLOR_BLUE __VA_ARGS__ COLOR_RESET)
#define PRINT_BOLD(...) printf(COLOR_BOLD __VA_ARGS__ COLOR_RESET)
#else
#define COLOR_RESET ""
#define COLOR_BLACK ""
#define COLOR_RED ""
#define COLOR_GREEN ""
#define COLOR_YELLOW ""
#define COLOR_BLUE ""
#define COLOR_MAGENTA ""
#define COLOR_CYAN ""
#define COLOR_WHITE ""
#define COLOR_BOLD ""

#define PRINT_RED(...) printf(__VA_ARGS__)
#define PRINT_GREEN(...) printf(__VA_ARGS__)
#define PRINT_YELLOW(...) printf(__VA_ARGS__)
#define PRINT_BLUE(...) printf(__VA_ARGS__)
#define PRINT_BOLD(...) printf(__VA_ARGS__)
#endif

    /* Public Struct -----------------------------------------------------------*/

    /* Public enum -----------------------------------------------------------*/

    /* Public Variable ---------------------------------------------------------*/

    /* Public function prototypes -----------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* YLIB_DEF_H */
