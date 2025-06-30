/**
 ******************************************************************************
 * @file       yLib_mempool.h
 * @brief      uC/OS风格的内存池管理器
 * @author     Ryan
 * @version    V2.0.0
 * @date       2025/06/16
 * @note
 ******************************************************************************
 */
#ifndef YLIB_MEMPOOL_H
#define YLIB_MEMPOOL_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "yLib_def.h"

/**
 * @brief 内存池错误码（仿照uC/OS）
 */
#define YLIB_MEM_NO_ERR 0u       /* 无错误 */
#define YLIB_MEM_INVALID_POOL 1u /* 无效的内存池 */
#define YLIB_MEM_INVALID_BLKS 2u /* 无效的块数量 */
#define YLIB_MEM_INVALID_SIZE 3u /* 无效的块大小 */
#define YLIB_MEM_NO_FREE_BLKS 4u /* 没有空闲块 */
#define YLIB_MEM_FULL 5u         /* 内存池已满 */
#define YLIB_MEM_INVALID_PBLK 6u /* 无效的块指针 */
#define YLIB_MEM_INVALID_ADDR 7u /* 无效的地址 */
#define YLIB_MEM_INVALID_PART 8u /* 无效的内存分区 */
#define YLIB_MEM_INVALID_OPT 9u  /* 无效的选项 */

/**
 * @brief 内存池魔术数字
 */
#define YLIB_MEM_MAGIC 0x4D454D21 /* "MEM!" */

    /**
     * @brief 内存分区控制块（仿照uC/OS的OS_MEM）
     */
    typedef struct ylib_mem
    {
        void *MemAddr;                /* 内存分区起始地址 */
        struct ylib_mem *MemFreeList; /* 空闲内存块链表指针 */
        uint32_t MemBlkSize;          /* 内存块大小（字节） */
        uint32_t MemNBlks;            /* 总的内存块数量 */
        uint32_t MemNFree;            /* 空闲内存块数量 */
#if YLIB_MEM_NAME_EN > 0u
        uint8_t *MemName; /* 内存分区名称 */
#endif
#if configHEAP_CHECK_ENABLE
        uint32_t MemMagic; /* 魔术数字用于检测 */
#endif
    } YLIB_MEM;

    /**
     * @brief 内存分区信息结构（仿照OSMemQuery返回的信息）
     */
    typedef struct
    {
        void *MemAddr;       /* 内存分区起始地址 */
        void *MemFreeList;   /* 空闲内存块链表指针 */
        uint32_t MemBlkSize; /* 内存块大小（字节） */
        uint32_t MemNBlks;   /* 总的内存块数量 */
        uint32_t MemNFree;   /* 空闲内存块数量 */
        uint32_t MemNUsed;   /* 已使用内存块数量 */
    } YLIB_MEM_DATA;

    /* uC/OS风格的核心API */

    /**
     * @brief 创建内存分区（仿照OSMemCreate）
     * @param addr 内存分区起始地址
     * @param nblks 内存块数量
     * @param blksize 每个内存块大小（字节）
     * @param perr 错误码指针
     * @return 内存分区控制块指针
     */
    YLIB_MEM *YLibMemCreate(void *addr, uint32_t nblks, uint32_t blksize, uint8_t *perr);

    /**
     * @brief 从内存分区获取内存块（仿照OSMemGet）
     * @param pmem 内存分区控制块指针
     * @param perr 错误码指针
     * @return 内存块指针，失败返回NULL
     */
    void *YLibMemGet(YLIB_MEM *pmem, uint8_t *perr);

    /**
     * @brief 释放内存块到内存分区（仿照OSMemPut）
     * @param pmem 内存分区控制块指针
     * @param pblk 要释放的内存块指针
     * @return 错误码
     */
    uint8_t YLibMemPut(YLIB_MEM *pmem, void *pblk);

    /**
     * @brief 获取内存分区信息（仿照OSMemQuery）
     * @param pmem 内存分区控制块指针
     * @param p_mem_data 返回的内存信息结构指针
     * @return 错误码
     */
    uint8_t YLibMemQuery(YLIB_MEM *pmem, YLIB_MEM_DATA *p_mem_data);

#if YLIB_MEM_NAME_EN > 0u
    /**
     * @brief 设置内存分区名称（仿照OSMemNameSet）
     * @param pmem 内存分区控制块指针
     * @param pname 名称字符串
     * @param perr 错误码指针
     */
    void YLibMemNameSet(YLIB_MEM *pmem, uint8_t *pname, uint8_t *perr);

    /**
     * @brief 获取内存分区名称（仿照OSMemNameGet）
     * @param pmem 内存分区控制块指针
     * @param pname 返回名称的缓冲区指针
     * @param perr 错误码指针
     */
    void YLibMemNameGet(YLIB_MEM *pmem, uint8_t **pname, uint8_t *perr);
#endif

    /* 现代风格API（兼容性接口） */
    typedef YLIB_MEM ylib_mempool_t;

    /**
     * @brief 创建内存池（现代接口）
     * @param block_size 块大小
     * @param block_count 块数量
     * @return 内存池句柄，失败返回NULL
     */
    ylib_mempool_t *ylib_mempool_create(size_t block_size, size_t block_count);

    /**
     * @brief 从内存池分配块
     * @param pool 内存池句柄
     * @return 分配的内存块，失败返回NULL
     */
    void *ylib_mempool_alloc(ylib_mempool_t *pool);

    /**
     * @brief 释放内存块到内存池
     * @param pool 内存池句柄
     * @param ptr 内存块指针
     * @return 0成功，其他失败
     */
    int ylib_mempool_free(ylib_mempool_t *pool, void *ptr);

    /**
     * @brief 销毁内存池
     * @param pool 内存池句柄
     */
    void ylib_mempool_destroy(ylib_mempool_t *pool);

    /**
     * @brief 获取内存池统计信息
     * @param pool 内存池句柄
     * @param free_blocks 返回空闲块数量
     * @param total_blocks 返回总块数量
     * @return 0成功，其他失败
     */
    int ylib_mempool_get_stats(ylib_mempool_t *pool, size_t *free_blocks, size_t *total_blocks);

/* 辅助宏定义 */

/**
 * @brief 静态创建内存池（仿照uC/OS的静态内存分区创建）
 * @param name 内存池名称
 * @param type 内存块类型
 * @param count 内存块数量
 */
#define YLIB_MEM_PARTITION_DECLARE(name, type, count) \
    static type name##_pool_buffer[count];            \
    static YLIB_MEM name##_mem_partition

/**
 * @brief 初始化静态内存池
 * @param name 内存池名称
 * @param type 内存块类型
 * @param count 内存块数量
 * @param perr 错误码指针
 */
#define YLIB_MEM_PARTITION_INIT(name, type, count, perr) \
    YLibMemCreate(name##_pool_buffer, count, sizeof(type), perr)

/**
 * @brief 计算内存分区所需的总内存大小
 * @param nblks 块数量
 * @param blksize 块大小
 */
#define YLIB_MEM_PARTITION_SIZE(nblks, blksize) \
    ((nblks) * ylib_heap_align_up(blksize))

/* 调试和统计功能 */
#if configHEAP_CHECK_ENABLE
    /**
     * @brief 检查内存分区完整性
     * @param pmem 内存分区控制块指针
     * @return 1表示完整，0表示损坏
     */
    int YLibMemCheckIntegrity(YLIB_MEM *pmem);

    /**
     * @brief 打印内存分区信息
     * @param pmem 内存分区控制块指针
     */
    void YLibMemPrintInfo(YLIB_MEM *pmem);
#endif

/* 兼容uC/OS的宏定义 */
#define OSMemCreate(addr, nblks, blksize, perr) YLibMemCreate(addr, nblks, blksize, perr)
#define OSMemGet(pmem, perr) YLibMemGet(pmem, perr)
#define OSMemPut(pmem, pblk) YLibMemPut(pmem, pblk)
#define OSMemQuery(pmem, p_mem_data) YLibMemQuery(pmem, p_mem_data)

#if YLIB_MEM_NAME_EN > 0u
#define OSMemNameSet(pmem, pname, perr) YLibMemNameSet(pmem, pname, perr)
#define OSMemNameGet(pmem, pname, perr) YLibMemNameGet(pmem, pname, perr)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* YLIB_MEMPOOL_H */
