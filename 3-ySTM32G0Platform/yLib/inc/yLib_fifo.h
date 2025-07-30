/**
  ******************************************************************************
  * @file       yLib_fifo.h
  * @brief      FIFO队列实现（基于环形缓冲区）
  * @author     Ryan
  * @version    V2.0.0
  * @date       2025/06/16
  * @note       
  ******************************************************************************
  */
#ifndef YLIB_FIFO_H
#define YLIB_FIFO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "yLib_def.h"

/**
 * @brief FIFO队列结构体
 */
struct ylib_fifo {
    unsigned int in;        /* 写入位置 */
    unsigned int out;       /* 读取位置 */
    unsigned int mask;      /* 大小掩码（必须是2的幂次-1） */
    void *data;            /* 数据缓冲区 */
};

/**
 * @brief 初始化FIFO
 * @param fifo FIFO指针
 * @param buffer 数据缓冲区
 * @param size 缓冲区大小（必须是2的幂次）
 * @return 0成功，-1失败
 */
static inline int ylib_fifo_init(struct ylib_fifo *fifo, void *buffer, unsigned int size)
{
    /* 检查size是否为2的幂次 */
    if (!size || (size & (size - 1)))
        return -1;
    
    fifo->in = 0;
    fifo->out = 0;
    fifo->mask = size - 1;
    fifo->data = buffer;
    
    return 0;
}

/**
 * @brief 分配并初始化FIFO
 * @param size 缓冲区大小（必须是2的幂次）
 * @param element_size 元素大小
 * @return FIFO指针，失败返回NULL
 */
struct ylib_fifo *ylib_fifo_alloc(unsigned int size, size_t element_size);

/**
 * @brief 释放FIFO
 * @param fifo FIFO指针
 */
void ylib_fifo_free(struct ylib_fifo *fifo);

/**
 * @brief 重置FIFO
 * @param fifo FIFO指针
 */
static inline void ylib_fifo_reset(struct ylib_fifo *fifo)
{
    fifo->in = fifo->out = 0;
}

/**
 * @brief 获取FIFO中元素个数
 * @param fifo FIFO指针
 * @return 元素个数
 */
static inline unsigned int ylib_fifo_len(struct ylib_fifo *fifo)
{
    return fifo->in - fifo->out;
}

/**
 * @brief 获取FIFO剩余空间
 * @param fifo FIFO指针
 * @return 剩余空间大小
 */
static inline unsigned int ylib_fifo_avail(struct ylib_fifo *fifo)
{
    return (fifo->mask + 1) - (fifo->in - fifo->out);
}

/**
 * @brief 检查FIFO是否为空
 * @param fifo FIFO指针
 * @return 1表示空，0表示非空
 */
static inline int ylib_fifo_is_empty(struct ylib_fifo *fifo)
{
    return fifo->in == fifo->out;
}

/**
 * @brief 检查FIFO是否已满
 * @param fifo FIFO指针
 * @return 1表示满，0表示未满
 */
static inline int ylib_fifo_is_full(struct ylib_fifo *fifo)
{
    return ylib_fifo_len(fifo) > fifo->mask;
}

/**
 * @brief 将数据放入FIFO
 * @param fifo FIFO指针
 * @param buffer 数据缓冲区
 * @param len 数据长度
 * @param element_size 元素大小
 * @return 实际写入的元素个数
 */
unsigned int ylib_fifo_in(struct ylib_fifo *fifo, const void *buffer, 
                          unsigned int len, size_t element_size);

/**
 * @brief 从FIFO取出数据
 * @param fifo FIFO指针
 * @param buffer 数据缓冲区
 * @param len 要读取的长度
 * @param element_size 元素大小
 * @return 实际读取的元素个数
 */
unsigned int ylib_fifo_out(struct ylib_fifo *fifo, void *buffer,
                           unsigned int len, size_t element_size);

/**
 * @brief 查看FIFO数据但不取出
 * @param fifo FIFO指针
 * @param buffer 数据缓冲区
 * @param len 要查看的长度
 * @param element_size 元素大小
 * @return 实际查看的元素个数
 */
unsigned int ylib_fifo_peek(struct ylib_fifo *fifo, void *buffer,
                            unsigned int len, size_t element_size);

/**
 * @brief 跳过指定数量的元素
 * @param fifo FIFO指针
 * @param len 要跳过的元素个数
 * @return 实际跳过的元素个数
 */
unsigned int ylib_fifo_skip(struct ylib_fifo *fifo, unsigned int len);

/* 类型安全的宏定义 */

/**
 * @brief 定义类型安全的FIFO
 * @param type 元素类型
 * @param size 大小（必须是2的幂次）
 */
#define DECLARE_YLIB_FIFO(type, size) \
struct { \
    struct ylib_fifo fifo; \
    type buffer[size]; \
}

/**
 * @brief 初始化类型安全的FIFO
 * @param fifo FIFO变量
 */
#define INIT_YLIB_FIFO(fifo) \
    ylib_fifo_init(&(fifo).fifo, (fifo).buffer, ARRAY_SIZE((fifo).buffer))

/**
 * @brief 类型安全的入队操作
 * @param fifo FIFO变量
 * @param val 要入队的值
 * @return 成功入队的元素个数
 */
#define ylib_fifo_put(fifo, val) \
({ \
    typeof((fifo)->buffer[0]) __tmp = val; \
    ylib_fifo_in(&(fifo)->fifo, &__tmp, 1, sizeof(__tmp)); \
})

/**
 * @brief 类型安全的出队操作
 * @param fifo FIFO变量
 * @param val 存储出队值的变量
 * @return 成功出队的元素个数
 */
#define ylib_fifo_get(fifo, val) \
    ylib_fifo_out(&(fifo)->fifo, val, 1, sizeof(*(val)))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* YLIB_FIFO_H */
