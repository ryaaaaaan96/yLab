/**
  ******************************************************************************
  * @file       yLib_fifo.c
  * @brief      FIFO队列实现
  * @author     Ryan
  * @version    V2.0.0
  * @date       2025/06/16
  * @note       
  ******************************************************************************
  */

#include "yLib_fifo.h"

/**
 * @brief 内部复制函数 - 从线性缓冲区复制到环形缓冲区
 * @param fifo FIFO指针
 * @param from 源缓冲区
 * @param len 长度
 * @param off 偏移
 * @param element_size 元素大小
 */
static void ylib_fifo_copy_in(struct ylib_fifo *fifo, const void *from,
                              unsigned int len, unsigned int off, size_t element_size)
{
    unsigned int size = fifo->mask + 1;
    unsigned int l;

    off &= fifo->mask;
    l = MIN(len, size - off);

    memcpy((char*)fifo->data + off * element_size, from, l * element_size);
    memcpy(fifo->data, (char*)from + l * element_size, (len - l) * element_size);
}

/**
 * @brief 内部复制函数 - 从环形缓冲区复制到线性缓冲区
 * @param fifo FIFO指针
 * @param to 目标缓冲区
 * @param len 长度
 * @param off 偏移
 * @param element_size 元素大小
 */
static void ylib_fifo_copy_out(struct ylib_fifo *fifo, void *to,
                               unsigned int len, unsigned int off, size_t element_size)
{
    unsigned int size = fifo->mask + 1;
    unsigned int l;

    off &= fifo->mask;
    l = MIN(len, size - off);
    
    memcpy(to, (char*)fifo->data + off * element_size, l * element_size);
    memcpy((char*)to + l * element_size, fifo->data, (len - l) * element_size);
}

/**
 * @brief 分配并初始化FIFO
 * @param size 缓冲区大小（必须是2的幂次）
 * @param element_size 元素大小
 * @return FIFO指针，失败返回NULL
 */
struct ylib_fifo *ylib_fifo_alloc(unsigned int size, size_t element_size)
{
    struct ylib_fifo *fifo;
    void *buffer;

    /* 检查size是否为2的幂次 */
    if (!size || (size & (size - 1)))
        return NULL;

    fifo = malloc(sizeof(struct ylib_fifo));
    if (!fifo)
        return NULL;

    buffer = malloc(size * element_size);
    if (!buffer) {
        free(fifo);
        return NULL;
    }

    if (ylib_fifo_init(fifo, buffer, size) < 0) {
        free(buffer);
        free(fifo);
        return NULL;
    }

    return fifo;
}

/**
 * @brief 释放FIFO
 * @param fifo FIFO指针
 */
void ylib_fifo_free(struct ylib_fifo *fifo)
{
    if (fifo) {
        free(fifo->data);
        free(fifo);
    }
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
                          unsigned int len, size_t element_size)
{
    unsigned int l;

    l = ylib_fifo_avail(fifo);
    if (len > l)
        len = l;

    ylib_fifo_copy_in(fifo, buffer, len, fifo->in, element_size);
    fifo->in += len;

    return len;
}

/**
 * @brief 从FIFO取出数据
 * @param fifo FIFO指针
 * @param buffer 数据缓冲区
 * @param len 要读取的长度
 * @param element_size 元素大小
 * @return 实际读取的元素个数
 */
unsigned int ylib_fifo_out(struct ylib_fifo *fifo, void *buffer,
                           unsigned int len, size_t element_size)
{
    len = MIN(len, fifo->in - fifo->out);

    ylib_fifo_copy_out(fifo, buffer, len, fifo->out, element_size);
    fifo->out += len;

    return len;
}

/**
 * @brief 查看FIFO数据但不取出
 * @param fifo FIFO指针
 * @param buffer 数据缓冲区
 * @param len 要查看的长度
 * @param element_size 元素大小
 * @return 实际查看的元素个数
 */
unsigned int ylib_fifo_peek(struct ylib_fifo *fifo, void *buffer,
                            unsigned int len, size_t element_size)
{
    len = MIN(len, fifo->in - fifo->out);

    ylib_fifo_copy_out(fifo, buffer, len, fifo->out, element_size);

    return len;
}

/**
 * @brief 跳过指定数量的元素
 * @param fifo FIFO指针
 * @param len 要跳过的元素个数
 * @param element_size 元素大小
 * @return 实际跳过的元素个数
 */
unsigned int ylib_fifo_skip(struct ylib_fifo *fifo, size_t len)
{
    len = MIN(len, fifo->in - fifo->out);
    fifo->out += len;
    return len;
}

/**
 * @brief min宏的简单实现
 */
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
