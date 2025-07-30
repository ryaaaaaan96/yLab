/**
  ******************************************************************************
  * @file       yLib_ring.h
  * @brief      环形队列实现
  * @author     Ryan
  * @version    V2.0.0
  * @date       2025/06/16
  * @note       
  ******************************************************************************
  */
#ifndef YLIB_RING_H
#define YLIB_RING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "yLib_def.h"

/**
 * @brief 环形队列结构体
 */
struct ylib_ring {
    volatile unsigned int head;    /* 读取位置 */
    volatile unsigned int tail;    /* 写入位置 */
    unsigned int size;             /* 队列大小 */
    unsigned int mask;             /* 大小掩码 */
    void *ring;                    /* 数据缓冲区 */
};

/**
 * @brief 创建环形队列
 * @param size 队列大小（必须是2的幂次）
 * @param element_size 元素大小
 * @return 环形队列指针，失败返回NULL
 */
struct ylib_ring *ylib_ring_create(unsigned int size, size_t element_size);

/**
 * @brief 销毁环形队列
 * @param ring 环形队列指针
 */
void ylib_ring_destroy(struct ylib_ring *ring);

/**
 * @brief 初始化环形队列
 * @param ring 环形队列指针
 * @param buffer 数据缓冲区
 * @param size 队列大小（必须是2的幂次）
 * @return 0成功，-1失败
 */
static inline int ylib_ring_init(struct ylib_ring *ring, void *buffer, unsigned int size)
{
    /* 检查size是否为2的幂次 */
    if (!size || (size & (size - 1)))
        return -1;
    
    ring->head = 0;
    ring->tail = 0;
    ring->size = size;
    ring->mask = size - 1;
    ring->ring = buffer;
    
    return 0;
}

/**
 * @brief 重置环形队列
 * @param ring 环形队列指针
 */
static inline void ylib_ring_reset(struct ylib_ring *ring)
{
    ring->head = 0;
    ring->tail = 0;
}

/**
 * @brief 获取队列中元素个数
 * @param ring 环形队列指针
 * @return 元素个数
 */
static inline unsigned int ylib_ring_count(struct ylib_ring *ring)
{
    return ring->tail - ring->head;
}

/**
 * @brief 获取队列剩余空间
 * @param ring 环形队列指针
 * @return 剩余空间大小
 */
static inline unsigned int ylib_ring_free_count(struct ylib_ring *ring)
{
    return ring->size - ylib_ring_count(ring);
}

/**
 * @brief 检查队列是否为空
 * @param ring 环形队列指针
 * @return 1表示空，0表示非空
 */
static inline int ylib_ring_empty(struct ylib_ring *ring)
{
    return ring->head == ring->tail;
}

/**
 * @brief 检查队列是否已满
 * @param ring 环形队列指针
 * @return 1表示满，0表示未满
 */
static inline int ylib_ring_full(struct ylib_ring *ring)
{
    return ylib_ring_count(ring) == ring->size;
}

/**
 * @brief 入队操作（单个元素）
 * @param ring 环形队列指针
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败（队列已满）
 */
int ylib_ring_enqueue(struct ylib_ring *ring, const void *data, size_t element_size);

/**
 * @brief 出队操作（单个元素）
 * @param ring 环形队列指针
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败（队列为空）
 */
int ylib_ring_dequeue(struct ylib_ring *ring, void *data, size_t element_size);

/**
 * @brief 批量入队操作
 * @param ring 环形队列指针
 * @param data 数据缓冲区
 * @param count 元素个数
 * @param element_size 元素大小
 * @return 实际入队的元素个数
 */
unsigned int ylib_ring_enqueue_bulk(struct ylib_ring *ring, const void *data,
                                    unsigned int count, size_t element_size);

/**
 * @brief 批量出队操作
 * @param ring 环形队列指针
 * @param data 数据缓冲区
 * @param count 元素个数
 * @param element_size 元素大小
 * @return 实际出队的元素个数
 */
unsigned int ylib_ring_dequeue_bulk(struct ylib_ring *ring, void *data,
                                    unsigned int count, size_t element_size);

/**
 * @brief 查看队首元素但不出队
 * @param ring 环形队列指针
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败（队列为空）
 */
int ylib_ring_peek(struct ylib_ring *ring, void *data, size_t element_size);

/**
 * @brief 查看指定位置的元素
 * @param ring 环形队列指针
 * @param index 索引（0表示队首）
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败（索引越界）
 */
int ylib_ring_peek_at(struct ylib_ring *ring, unsigned int index, 
                      void *data, size_t element_size);

/* 多生产者/多消费者支持 */

/**
 * @brief 多生产者入队操作
 * @param ring 环形队列指针
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败
 */
int ylib_ring_mp_enqueue(struct ylib_ring *ring, const void *data, size_t element_size);

/**
 * @brief 多消费者出队操作
 * @param ring 环形队列指针
 * @param data 数据指针
 * @param element_size 元素大小
 * @return 1成功，0失败
 */
int ylib_ring_mc_dequeue(struct ylib_ring *ring, void *data, size_t element_size);

/* 类型安全的宏定义 */

/**
 * @brief 定义类型安全的环形队列
 * @param type 元素类型
 * @param size 大小（必须是2的幂次）
 */
#define DECLARE_YLIB_RING(type, size) \
struct { \
    struct ylib_ring ring; \
    type buffer[size]; \
}

/**
 * @brief 初始化类型安全的环形队列
 * @param ring_var 环形队列变量
 */
#define INIT_YLIB_RING(ring_var) \
    ylib_ring_init(&(ring_var).ring, (ring_var).buffer, ARRAY_SIZE((ring_var).buffer))

/**
 * @brief 类型安全的入队操作
 * @param ring_var 环形队列变量
 * @param val 要入队的值
 * @return 1成功，0失败
 */
#define ylib_ring_put(ring_var, val) \
({ \
    typeof((ring_var)->buffer[0]) __tmp = val; \
    ylib_ring_enqueue(&(ring_var)->ring, &__tmp, sizeof(__tmp)); \
})

/**
 * @brief 类型安全的出队操作
 * @param ring_var 环形队列变量
 * @param val 存储出队值的变量指针
 * @return 1成功，0失败
 */
#define ylib_ring_get(ring_var, val) \
    ylib_ring_dequeue(&(ring_var)->ring, val, sizeof(*(val)))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* YLIB_RING_H */
