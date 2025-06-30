/**
 ******************************************************************************
 * @file       yLib_ring.c
 * @brief      环形队列实现
 * @author     Ryan
 * @version    V2.0.0
 * @date       2025/06/16
 * @note
 ******************************************************************************
 */

#include "yLib_ring.h"

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

/* 原子操作的简单实现（单线程环境） */
#ifndef __atomic_load_n
#define __atomic_load_n(ptr, model) (*(ptr))
#endif

#ifndef __atomic_store_n
#define __atomic_store_n(ptr, val, model) (*(ptr) = (val))
#endif

#ifndef __atomic_compare_exchange_n
/* 使用内联函数替代GNU扩展的语句表达式 */
static inline int ylib_atomic_compare_exchange_uint(volatile unsigned int *ptr,
                                                    unsigned int *expected,
                                                    unsigned int desired)
{
    unsigned int old = *expected;
    unsigned int cur = *ptr;
    if (cur == old)
    {
        *ptr = desired;
        return 1;
    }
    else
    {
        *expected = cur;
        return 0;
    }
}

#define __atomic_compare_exchange_n(ptr, expected, desired, weak, success_model, failure_model) \
    ylib_atomic_compare_exchange_uint((ptr), (expected), (desired))
#endif

/**
 * @brief 创建环形队列
 * @param size 队列大小（必须是2的幂次）
 * @param element_size 元素大小
 * @return 环形队列指针，失败返回NULL
 */
struct ylib_ring *ylib_ring_create(unsigned int size, size_t element_size)
{
    struct ylib_ring *ring;
    void *buffer;

    /* 检查size是否为2的幂次 */
    if (!size || (size & (size - 1)))
        return NULL;

    ring = malloc(sizeof(struct ylib_ring));
    if (!ring)
        return NULL;

    buffer = malloc(size * element_size);
    if (!buffer)
    {
        free(ring);
        return NULL;
    }

    if (ylib_ring_init(ring, buffer, size) < 0)
    {
        free(buffer);
        free(ring);
        return NULL;
    }

    return ring;
}

/**
 * @brief 销毁环形队列
 * @param ring 环形队列指针
 */
void ylib_ring_destroy(struct ylib_ring *ring)
{
    if (ring)
    {
        free(ring->ring);
        free(ring);
    }
}

/**
 * @brief 入队操作（单个元素）
 * @param ring 环形队列指针
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败（队列已满）
 */
int ylib_ring_enqueue(struct ylib_ring *ring, const void *data, size_t element_size)
{
    unsigned int head = ring->head;
    unsigned int tail = ring->tail;
    unsigned int next_tail = (tail + 1) & ring->mask;

    /* 检查队列是否已满 */
    if (next_tail == head)
        return 0;

    /* 复制数据 */
    memcpy((char *)ring->ring + tail * element_size, data, element_size);

    /* 更新tail */
    ring->tail = next_tail;

    return 1;
}

/**
 * @brief 出队操作（单个元素）
 * @param ring 环形队列指针
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败（队列为空）
 */
int ylib_ring_dequeue(struct ylib_ring *ring, void *data, size_t element_size)
{
    unsigned int head = ring->head;
    unsigned int tail = ring->tail;

    /* 检查队列是否为空 */
    if (head == tail)
        return 0;

    /* 复制数据 */
    memcpy(data, (char *)ring->ring + head * element_size, element_size);

    /* 更新head */
    ring->head = (head + 1) & ring->mask;

    return 1;
}

/**
 * @brief 批量入队操作
 * @param ring 环形队列指针
 * @param data 数据缓冲区
 * @param count 元素个数
 * @param element_size 元素大小
 * @return 实际入队的元素个数
 */
unsigned int ylib_ring_enqueue_bulk(struct ylib_ring *ring, const void *data,
                                    unsigned int count, size_t element_size)
{
    unsigned int head = ring->head;
    unsigned int tail = ring->tail;
    unsigned int free_space = ring->size - ((tail - head) & ring->mask);
    unsigned int n, idx;

    if (count == 0)
        return 0;

    /* 限制入队数量 */
    n = MIN(count, free_space);
    if (n == 0)
        return 0;

    /* 分段复制数据 */
    idx = tail & ring->mask;
    if (idx + n <= ring->size)
    {
        /* 连续复制 */
        memcpy((char *)ring->ring + idx * element_size, data, n * element_size);
    }
    else
    {
        /* 分两段复制 */
        unsigned int first_part = ring->size - idx;
        memcpy((char *)ring->ring + idx * element_size, data, first_part * element_size);
        memcpy(ring->ring, (char *)data + first_part * element_size,
               (n - first_part) * element_size);
    }

    /* 更新tail */
    ring->tail = (tail + n) & ring->mask;

    return n;
}

/**
 * @brief 批量出队操作
 * @param ring 环形队列指针
 * @param data 数据缓冲区
 * @param count 元素个数
 * @param element_size 元素大小
 * @return 实际出队的元素个数
 */
unsigned int ylib_ring_dequeue_bulk(struct ylib_ring *ring, void *data,
                                    unsigned int count, size_t element_size)
{
    unsigned int head = ring->head;
    unsigned int tail = ring->tail;
    unsigned int available = (tail - head) & ring->mask;
    unsigned int n, idx;

    if (count == 0)
        return 0;

    /* 限制出队数量 */
    n = MIN(count, available);
    if (n == 0)
        return 0;

    /* 分段复制数据 */
    idx = head & ring->mask;
    if (idx + n <= ring->size)
    {
        /* 连续复制 */
        memcpy(data, (char *)ring->ring + idx * element_size, n * element_size);
    }
    else
    {
        /* 分两段复制 */
        unsigned int first_part = ring->size - idx;
        memcpy(data, (char *)ring->ring + idx * element_size, first_part * element_size);
        memcpy((char *)data + first_part * element_size, ring->ring,
               (n - first_part) * element_size);
    }

    /* 更新head */
    ring->head = (head + n) & ring->mask;

    return n;
}

/**
 * @brief 查看队首元素但不出队
 * @param ring 环形队列指针
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败（队列为空）
 */
int ylib_ring_peek(struct ylib_ring *ring, void *data, size_t element_size)
{
    unsigned int head = ring->head;
    unsigned int tail = ring->tail;

    /* 检查队列是否为空 */
    if (head == tail)
        return 0;

    /* 复制数据 */
    memcpy(data, (char *)ring->ring + head * element_size, element_size);

    return 1;
}

/**
 * @brief 查看指定位置的元素
 * @param ring 环形队列指针
 * @param index 索引（0表示队首）
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败（索引越界）
 */
int ylib_ring_peek_at(struct ylib_ring *ring, unsigned int index,
                      void *data, size_t element_size)
{
    unsigned int head = ring->head;
    unsigned int tail = ring->tail;
    unsigned int available = (tail - head) & ring->mask;

    /* 检查索引是否有效 */
    if (index >= available)
        return 0;

    /* 计算实际位置并复制数据 */
    unsigned int pos = (head + index) & ring->mask;
    memcpy(data, (char *)ring->ring + pos * element_size, element_size);

    return 1;
}

/**
 * @brief 多生产者入队操作
 * @param ring 环形队列指针
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败
 */
int ylib_ring_mp_enqueue(struct ylib_ring *ring, const void *data, size_t element_size)
{
    unsigned int head, tail, next_tail;
    int success;

    do
    {
        head = __atomic_load_n(&ring->head, __ATOMIC_ACQUIRE);
        tail = __atomic_load_n(&ring->tail, __ATOMIC_RELAXED);
        next_tail = (tail + 1) & ring->mask;

        /* 检查队列是否已满 */
        if (next_tail == head)
            return 0;

        success = __atomic_compare_exchange_n(&ring->tail, &tail, next_tail,
                                              0, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
    } while (!success);

    /* 复制数据 */
    memcpy((char *)ring->ring + tail * element_size, data, element_size);

    return 1;
}

/**
 * @brief 多消费者出队操作
 * @param ring 环形队列指针
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败
 */
int ylib_ring_mc_dequeue(struct ylib_ring *ring, void *data, size_t element_size)
{
    unsigned int head, tail, next_head;
    int success;

    do
    {
        head = __atomic_load_n(&ring->head, __ATOMIC_RELAXED);
        tail = __atomic_load_n(&ring->tail, __ATOMIC_ACQUIRE);

        /* 检查队列是否为空 */
        if (head == tail)
            return 0;

        next_head = (head + 1) & ring->mask;

        success = __atomic_compare_exchange_n(&ring->head, &head, next_head,
                                              0, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
    } while (!success);

    /* 复制数据 */
    memcpy(data, (char *)ring->ring + head * element_size, element_size);

    return 1;
}
