/**
 ******************************************************************************
 * @file       yLib_mempool.c
 * @brief      uC/OS风格的内存池管理器实现
 * @author     Ryan
 * @version    V2.0.0
 * @date       2025/06/16
 * @note
 ******************************************************************************
 */

#include "yLib_mempool.h"
#include "yLib_heap.h"
#include <string.h>
#include <inttypes.h>

/*
 *********************************************************************************************************
 *                                    MEMORY PARTITION MANAGEMENT
 *
 * Note(s): 1) 这个模块仿照uC/OS-II的内存管理实现
 *          2) 采用固定大小的内存块分配策略
 *          3) 使用链表管理空闲块
 *********************************************************************************************************
 */

/**
 * @brief 创建内存分区（仿照OSMemCreate）
 * @param addr 内存分区起始地址
 * @param nblks 内存块数量
 * @param blksize 每个内存块大小（字节）
 * @param perr 错误码指针
 * @return 内存分区控制块指针
 */
YLIB_MEM *YLibMemCreate(void *addr, uint32_t nblks, uint32_t blksize, uint8_t *perr)
{
    YLIB_MEM *pmem;
    uint8_t *pblk;
    void **plink;
    uint32_t i;
    uint32_t aligned_blksize;

#ifdef YLIB_ARG_CHK_EN
    if (perr == NULL)
    {
        return NULL;
    }
    if (addr == NULL)
    {
        *perr = YLIB_MEM_INVALID_ADDR;
        return NULL;
    }
    if (nblks < 2u)
    {
        *perr = YLIB_MEM_INVALID_BLKS;
        return NULL;
    }
    if (blksize < sizeof(void *))
    {
        *perr = YLIB_MEM_INVALID_SIZE;
        return NULL;
    }
#endif

    /* 对齐块大小到指针大小的倍数 */
    aligned_blksize = ylib_heap_align_up(blksize);
    if (aligned_blksize < sizeof(void *))
    {
        aligned_blksize = sizeof(void *);
    }

    /* 分配内存分区控制块 */
    pmem = (YLIB_MEM *)ylib_malloc(sizeof(YLIB_MEM));
    if (pmem == NULL)
    {
        *perr = YLIB_MEM_NO_FREE_BLKS;
        return NULL;
    }

    /* 初始化内存分区控制块 */
    pmem->MemAddr = addr;
    pmem->MemFreeList = addr;
    pmem->MemBlkSize = aligned_blksize;
    pmem->MemNBlks = nblks;
    pmem->MemNFree = nblks;

#if YLIB_MEM_NAME_EN > 0u
    pmem->MemName = (uint8_t *)"?MEM";
#endif

#if configHEAP_CHECK_ENABLE
    pmem->MemMagic = YLIB_MEM_MAGIC;
#endif

    /* 链接所有内存块形成空闲链表 */
    plink = (void **)addr;
    pblk = (uint8_t *)addr;
    for (i = 0u; i < (nblks - 1u); i++)
    {
        pblk += aligned_blksize;
        *plink = (void *)pblk;
        plink = (void **)pblk;
    }
    *plink = NULL; /* 最后一个块指向NULL */

    *perr = YLIB_MEM_NO_ERR;
    return pmem;
}

/**
 * @brief 从内存分区获取内存块（仿照OSMemGet）
 * @param pmem 内存分区控制块指针
 * @param perr 错误码指针
 * @return 内存块指针，失败返回NULL
 */
void *YLibMemGet(YLIB_MEM *pmem, uint8_t *perr)
{
    void *pblk;

#ifdef YLIB_ARG_CHK_EN
    if (perr == NULL)
    {
        return NULL;
    }
    if (pmem == NULL)
    {
        *perr = YLIB_MEM_INVALID_POOL;
        return NULL;
    }
#endif

#if configHEAP_CHECK_ENABLE
    if (pmem->MemMagic != YLIB_MEM_MAGIC)
    {
        *perr = YLIB_MEM_INVALID_POOL;
        return NULL;
    }
#endif

    /* 检查是否有空闲块 */
    if (pmem->MemNFree == 0u)
    {
        *perr = YLIB_MEM_NO_FREE_BLKS;
        return NULL;
    }

    /* 从空闲链表头取出一个块 */
    pblk = pmem->MemFreeList;
    pmem->MemFreeList = *(void **)pblk;
    pmem->MemNFree--;

    *perr = YLIB_MEM_NO_ERR;
    return pblk;
}

/**
 * @brief 释放内存块到内存分区（仿照OSMemPut）
 * @param pmem 内存分区控制块指针
 * @param pblk 要释放的内存块指针
 * @return 错误码
 */
uint8_t YLibMemPut(YLIB_MEM *pmem, void *pblk)
{
    uint8_t *pblk_addr;
    uint8_t *pmem_start;
    uint8_t *pmem_end;

#ifdef YLIB_ARG_CHK_EN
    if (pmem == NULL)
    {
        return YLIB_MEM_INVALID_POOL;
    }
    if (pblk == NULL)
    {
        return YLIB_MEM_INVALID_PBLK;
    }
#endif

#if configHEAP_CHECK_ENABLE
    if (pmem->MemMagic != YLIB_MEM_MAGIC)
    {
        return YLIB_MEM_INVALID_POOL;
    }
#endif

    /* 检查内存块是否在有效范围内 */
    pblk_addr = (uint8_t *)pblk;
    pmem_start = (uint8_t *)pmem->MemAddr;
    pmem_end = pmem_start + (pmem->MemNBlks * pmem->MemBlkSize);

    if ((pblk_addr < pmem_start) || (pblk_addr >= pmem_end))
    {
        return YLIB_MEM_INVALID_PBLK;
    }

    /* 检查地址是否按块大小对齐 */
    if (((pblk_addr - pmem_start) % pmem->MemBlkSize) != 0u)
    {
        return YLIB_MEM_INVALID_PBLK;
    }

    /* 检查是否已满 */
    if (pmem->MemNFree >= pmem->MemNBlks)
    {
        return YLIB_MEM_FULL;
    }

    /* 将块插入到空闲链表头 */
    *(void **)pblk = pmem->MemFreeList;
    pmem->MemFreeList = pblk;
    pmem->MemNFree++;

    return YLIB_MEM_NO_ERR;
}

/**
 * @brief 获取内存分区信息（仿照OSMemQuery）
 * @param pmem 内存分区控制块指针
 * @param p_mem_data 返回的内存信息结构指针
 * @return 错误码
 */
uint8_t YLibMemQuery(YLIB_MEM *pmem, YLIB_MEM_DATA *p_mem_data)
{
#ifdef YLIB_ARG_CHK_EN
    if (pmem == NULL)
    {
        return YLIB_MEM_INVALID_POOL;
    }
    if (p_mem_data == NULL)
    {
        return YLIB_MEM_INVALID_ADDR;
    }
#endif

#if configHEAP_CHECK_ENABLE
    if (pmem->MemMagic != YLIB_MEM_MAGIC)
    {
        return YLIB_MEM_INVALID_POOL;
    }
#endif

    /* 复制内存分区信息 */
    p_mem_data->MemAddr = pmem->MemAddr;
    p_mem_data->MemFreeList = pmem->MemFreeList;
    p_mem_data->MemBlkSize = pmem->MemBlkSize;
    p_mem_data->MemNBlks = pmem->MemNBlks;
    p_mem_data->MemNFree = pmem->MemNFree;
    p_mem_data->MemNUsed = pmem->MemNBlks - pmem->MemNFree;

    return YLIB_MEM_NO_ERR;
}

#if YLIB_MEM_NAME_EN > 0u
/**
 * @brief 设置内存分区名称（仿照OSMemNameSet）
 * @param pmem 内存分区控制块指针
 * @param pname 名称字符串
 * @param perr 错误码指针
 */
void YLibMemNameSet(YLIB_MEM *pmem, uint8_t *pname, uint8_t *perr)
{
#ifdef YLIB_ARG_CHK_EN
    if (perr == NULL)
    {
        return;
    }
    if (pmem == NULL)
    {
        *perr = YLIB_MEM_INVALID_POOL;
        return;
    }
    if (pname == NULL)
    {
        *perr = YLIB_MEM_INVALID_ADDR;
        return;
    }
#endif

#if configHEAP_CHECK_ENABLE
    if (pmem->MemMagic != YLIB_MEM_MAGIC)
    {
        *perr = YLIB_MEM_INVALID_POOL;
        return;
    }
#endif

    pmem->MemName = pname;
    *perr = YLIB_MEM_NO_ERR;
}

/**
 * @brief 获取内存分区名称（仿照OSMemNameGet）
 * @param pmem 内存分区控制块指针
 * @param pname 返回名称的缓冲区指针
 * @param perr 错误码指针
 */
void YLibMemNameGet(YLIB_MEM *pmem, uint8_t **pname, uint8_t *perr)
{
#ifdef YLIB_ARG_CHK_EN
    if (perr == NULL)
    {
        return;
    }
    if (pmem == NULL)
    {
        *perr = YLIB_MEM_INVALID_POOL;
        return;
    }
    if (pname == NULL)
    {
        *perr = YLIB_MEM_INVALID_ADDR;
        return;
    }
#endif

#if configHEAP_CHECK_ENABLE
    if (pmem->MemMagic != YLIB_MEM_MAGIC)
    {
        *perr = YLIB_MEM_INVALID_POOL;
        return;
    }
#endif

    *pname = pmem->MemName;
    *perr = YLIB_MEM_NO_ERR;
}
#endif

#if configHEAP_CHECK_ENABLE
/**
 * @brief 检查内存分区完整性
 * @param pmem 内存分区控制块指针
 * @return 1表示完整，0表示损坏
 */
int YLibMemCheckIntegrity(YLIB_MEM *pmem)
{
    void *pblk;
    uint32_t count;
    uint8_t *pblk_addr;
    uint8_t *pmem_start;
    uint8_t *pmem_end;

    if (pmem == NULL)
    {
        return 0;
    }

    if (pmem->MemMagic != YLIB_MEM_MAGIC)
    {
        return 0;
    }

    /* 检查基本参数 */
    if (pmem->MemNFree > pmem->MemNBlks)
    {
        return 0;
    }

    /* 遍历空闲链表，检查每个节点 */
    pblk = pmem->MemFreeList;
    count = 0;
    pmem_start = (uint8_t *)pmem->MemAddr;
    pmem_end = pmem_start + (pmem->MemNBlks * pmem->MemBlkSize);

    while (pblk != NULL)
    {
        count++;

        /* 检查是否超出计数 */
        if (count > pmem->MemNFree)
        {
            return 0;
        }

        /* 检查地址范围 */
        pblk_addr = (uint8_t *)pblk;
        if ((pblk_addr < pmem_start) || (pblk_addr >= pmem_end))
        {
            return 0;
        }

        /* 检查地址对齐 */
        if (((pblk_addr - pmem_start) % pmem->MemBlkSize) != 0u)
        {
            return 0;
        }

        pblk = *(void **)pblk;
    }

    /* 检查计数是否匹配 */
    if (count != pmem->MemNFree)
    {
        return 0;
    }

    return 1;
}

/**
 * @brief 打印内存分区信息
 * @param pmem 内存分区控制块指针
 */
void YLibMemPrintInfo(YLIB_MEM *pmem)
{
    YLIB_MEM_DATA mem_data;
    uint8_t err;

    if (pmem == NULL)
    {
        printf("[YLIB MEM] Invalid memory partition\n");
        return;
    }

    err = YLibMemQuery(pmem, &mem_data);
    if (err != YLIB_MEM_NO_ERR)
    {
        printf("[YLIB MEM] Query failed, error: %d\n", err);
        return;
    }

    printf("[YLIB MEM] Memory Partition Info:\n");
    printf("  Address     : %p\n", mem_data.MemAddr);
    printf("  Block Size  : %" PRIu32 " bytes\n", mem_data.MemBlkSize);
    printf("  Total Blocks: %" PRIu32 "\n", mem_data.MemNBlks);
    printf("  Free Blocks : %" PRIu32 "\n", mem_data.MemNFree);
    printf("  Used Blocks : %" PRIu32 "\n", mem_data.MemNUsed);
    printf("  Utilization : %.1f%%\n",
           (float)mem_data.MemNUsed * 100.0f / (float)mem_data.MemNBlks);

#if YLIB_MEM_NAME_EN > 0u
    if (pmem->MemName)
    {
        printf("  Name        : %s\n", pmem->MemName);
    }
#endif
}
#endif

/*
 *********************************************************************************************************
 *                                    MODERN STYLE API (兼容性接口)
 *********************************************************************************************************
 */

/**
 * @brief 创建内存池（现代接口）
 * @param block_size 块大小
 * @param block_count 块数量
 * @return 内存池句柄，失败返回NULL
 */
ylib_mempool_t *ylib_mempool_create(size_t block_size, size_t block_count)
{
    void *pool_buffer;
    YLIB_MEM *pmem;
    uint8_t err;
    size_t total_size;

    if (block_size == 0 || block_count == 0)
    {
        return NULL;
    }

    /* 计算所需的总内存大小 */
    total_size = YLIB_MEM_PARTITION_SIZE(block_count, block_size);

    /* 分配内存池缓冲区 */
    pool_buffer = ylib_malloc(total_size);
    if (pool_buffer == NULL)
    {
        return NULL;
    }

    /* 创建内存分区 */
    pmem = YLibMemCreate(pool_buffer, (uint32_t)block_count, (uint32_t)block_size, &err);
    if (err != YLIB_MEM_NO_ERR)
    {
        ylib_free(pool_buffer);
        return NULL;
    }

    return pmem;
}

/**
 * @brief 从内存池分配块
 * @param pool 内存池句柄
 * @return 分配的内存块，失败返回NULL
 */
void *ylib_mempool_alloc(ylib_mempool_t *pool)
{
    uint8_t err;
    return YLibMemGet(pool, &err);
}

/**
 * @brief 释放内存块到内存池
 * @param pool 内存池句柄
 * @param ptr 内存块指针
 * @return 0成功，其他失败
 */
int ylib_mempool_free(ylib_mempool_t *pool, void *ptr)
{
    return (int)YLibMemPut(pool, ptr);
}

/**
 * @brief 销毁内存池
 * @param pool 内存池句柄
 */
void ylib_mempool_destroy(ylib_mempool_t *pool)
{
    if (pool)
    {
        ylib_free(pool->MemAddr); /* 释放内存池缓冲区 */
        ylib_free(pool);          /* 释放控制块 */
    }
}

/**
 * @brief 获取内存池统计信息
 * @param pool 内存池句柄
 * @param free_blocks 返回空闲块数量
 * @param total_blocks 返回总块数量
 * @return 0成功，其他失败
 */
int ylib_mempool_get_stats(ylib_mempool_t *pool, size_t *free_blocks, size_t *total_blocks)
{
    YLIB_MEM_DATA mem_data;
    uint8_t err;

    if (pool == NULL)
    {
        return -1;
    }

    err = YLibMemQuery(pool, &mem_data);
    if (err != YLIB_MEM_NO_ERR)
    {
        return (int)err;
    }

    if (free_blocks)
    {
        *free_blocks = mem_data.MemNFree;
    }

    if (total_blocks)
    {
        *total_blocks = mem_data.MemNBlks;
    }

    return 0;
}
