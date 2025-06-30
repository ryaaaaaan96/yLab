/**
 ******************************************************************************
 * @file       yLib_heap.c
 * @brief      FreeRTOS风格的动态内存管理器实现
 * @author     Ryan
 * @version    V2.0.0
 * @date       2025/06/16
 * @note
 ******************************************************************************
 */

#include "yLib_heap.h"
#include <string.h>

#ifndef YLIB_HEAP_STATIC_BUFFER_SIZE
#define YLIB_HEAP_STATIC_BUFFER_SIZE configTOTAL_HEAP_SIZE
#endif

static uint8_t ylib_heap_static_buffer[YLIB_HEAP_STATIC_BUFFER_SIZE];

ylib_heap_t g_ylib_heap = {0};

static ylib_malloc_failed_hook_t malloc_failed_hook = NULL;
static ylib_free_hook_t free_hook = NULL;
static ylib_malloc_hook_t malloc_hook = NULL;

static void ylib_heap_insert_block(ylib_block_link_t *block)
{
    ylib_block_link_t *prev = NULL, *cur = g_ylib_heap.free_blocks_list;
    while (cur && cur < block)
    {
        prev = cur;
        cur = cur->next_free_block;
    }
    block->next_free_block = cur;
    if (prev)
    {
        prev->next_free_block = block;
    }
    else
    {
        g_ylib_heap.free_blocks_list = block;
    }
}

int ylib_heap_init(void *heap_buffer, size_t heap_size)
{
    if (heap_buffer == NULL)
    {
        heap_buffer = ylib_heap_static_buffer;
        heap_size = YLIB_HEAP_STATIC_BUFFER_SIZE;
    }
    heap_size = ylib_heap_align_down(heap_size);
    if (heap_size < sizeof(ylib_block_link_t) * 2)
        return -1;
    g_ylib_heap.heap_start = (uint8_t *)heap_buffer;
    g_ylib_heap.heap_end = g_ylib_heap.heap_start + heap_size;
    g_ylib_heap.heap_size = heap_size;
    g_ylib_heap.initialized = true;
    ylib_block_link_t *first = (ylib_block_link_t *)g_ylib_heap.heap_start;
    first->block_size = heap_size;
#if configHEAP_CHECK_ENABLE
    first->magic = YLIB_HEAP_MAGIC_FREE;
#endif
    first->next_free_block = NULL;
    g_ylib_heap.free_blocks_list = first;
#if configHEAP_STATS_ENABLE
    memset(&g_ylib_heap.stats, 0, sizeof(g_ylib_heap.stats));
    g_ylib_heap.stats.total_heap_size = heap_size;
    g_ylib_heap.stats.free_heap_size = heap_size;
    g_ylib_heap.stats.minimum_ever_free_heap_size = heap_size;
    g_ylib_heap.stats.number_of_free_blocks = 1;
    g_ylib_heap.stats.max_block_size = heap_size;
    g_ylib_heap.stats.min_block_size = heap_size;
#endif
    return 0;
}

void *ylib_malloc(size_t wanted_size)
{
    if (!g_ylib_heap.initialized)
        ylib_heap_init(NULL, 0);
    if (wanted_size == 0)
        return NULL;
    wanted_size = ylib_heap_align_up(wanted_size + sizeof(ylib_block_link_t));
    ylib_block_link_t *prev = NULL, *cur = g_ylib_heap.free_blocks_list;
    while (cur)
    {
        if (cur->block_size >= wanted_size)
        {
            if (cur->block_size - wanted_size > configMINIMAL_BLOCK_SIZE + sizeof(ylib_block_link_t))
            {
                ylib_block_link_t *split = (ylib_block_link_t *)((uint8_t *)cur + wanted_size);
                split->block_size = cur->block_size - wanted_size;
#if configHEAP_CHECK_ENABLE
                split->magic = YLIB_HEAP_MAGIC_FREE;
#endif
                split->next_free_block = cur->next_free_block;
                cur->block_size = wanted_size;
                cur->next_free_block = split;
            }
            else
            {
                wanted_size = cur->block_size;
            }
            if (prev)
            {
                prev->next_free_block = cur->next_free_block;
            }
            else
            {
                g_ylib_heap.free_blocks_list = cur->next_free_block;
            }
#if configHEAP_CHECK_ENABLE
            cur->magic = YLIB_HEAP_MAGIC_ALLOC;
#endif
#if configHEAP_STATS_ENABLE
            g_ylib_heap.stats.free_heap_size -= wanted_size;
            if (g_ylib_heap.stats.free_heap_size < g_ylib_heap.stats.minimum_ever_free_heap_size)
                g_ylib_heap.stats.minimum_ever_free_heap_size = g_ylib_heap.stats.free_heap_size;
            g_ylib_heap.stats.successful_allocations++;
#endif
            if (malloc_hook)
                malloc_hook(ylib_heap_get_user_ptr(cur), wanted_size - sizeof(ylib_block_link_t));
            return ylib_heap_get_user_ptr(cur);
        }
        prev = cur;
        cur = cur->next_free_block;
    }
    if (malloc_failed_hook)
        malloc_failed_hook();
    return NULL;
}

void ylib_free(void *pv)
{
    if (!pv)
        return;
    ylib_block_link_t *block = ylib_heap_get_block_ptr(pv);
#if configHEAP_CHECK_ENABLE
    if (block->magic != YLIB_HEAP_MAGIC_ALLOC)
        return;
    block->magic = YLIB_HEAP_MAGIC_FREE;
#endif
    ylib_heap_insert_block(block);
#if configHEAP_STATS_ENABLE
    g_ylib_heap.stats.free_heap_size += block->block_size;
    g_ylib_heap.stats.successful_frees++;
#endif
    if (free_hook)
        free_hook(pv, block->block_size - sizeof(ylib_block_link_t));
}

void *ylib_realloc(void *pv, size_t wanted_size)
{
    if (!pv)
        return ylib_malloc(wanted_size);
    if (wanted_size == 0)
    {
        ylib_free(pv);
        return NULL;
    }
    ylib_block_link_t *block = ylib_heap_get_block_ptr(pv);
    size_t old_size = block->block_size - sizeof(ylib_block_link_t);
    if (old_size >= wanted_size)
        return pv;
    void *new_ptr = ylib_malloc(wanted_size);
    if (new_ptr)
    {
        memcpy(new_ptr, pv, old_size);
        ylib_free(pv);
    }
    return new_ptr;
}

void *ylib_calloc(size_t num, size_t size)
{
    size_t total = num * size;
    void *ptr = ylib_malloc(total);
    if (ptr)
        memset(ptr, 0, total);
    return ptr;
}

size_t ylib_get_free_heap_size(void)
{
#if configHEAP_STATS_ENABLE
    return g_ylib_heap.stats.free_heap_size;
#else
    size_t total = 0;
    ylib_block_link_t *cur = g_ylib_heap.free_blocks_list;
    while (cur)
    {
        total += cur->block_size;
        cur = cur->next_free_block;
    }
    return total;
#endif
}

size_t ylib_get_minimum_ever_free_heap_size(void)
{
#if configHEAP_STATS_ENABLE
    return g_ylib_heap.stats.minimum_ever_free_heap_size;
#else
    return 0;
#endif
}

#if configHEAP_STATS_ENABLE
void ylib_get_heap_stats(ylib_heap_stats_t *stats)
{
    if (stats)
        *stats = g_ylib_heap.stats;
}
#endif

#if configHEAP_CHECK_ENABLE
int ylib_heap_check_integrity(void)
{
    ylib_block_link_t *cur = g_ylib_heap.free_blocks_list;
    while (cur)
    {
        if (cur->block_size == 0)
            return 0;
        if (cur->magic != YLIB_HEAP_MAGIC_FREE)
            return 0;
        cur = cur->next_free_block;
    }
    return 1;
}

void ylib_heap_print_info(void)
{
    ylib_block_link_t *cur = g_ylib_heap.free_blocks_list;
    size_t idx = 0;
    printf("[YLIB HEAP] Free blocks:\n");
    while (cur)
    {
        printf("  Block %zu: addr=%p, size=%zu\n", idx++, (void *)cur, cur->block_size);
        cur = cur->next_free_block;
    }
    printf("[YLIB HEAP] Free heap size: %zu\n", ylib_get_free_heap_size());
}
#endif

size_t ylib_malloc_size(void *pv)
{
    if (!pv)
        return 0;
    ylib_block_link_t *block = ylib_heap_get_block_ptr(pv);
    return block->block_size - sizeof(ylib_block_link_t);
}

void ylib_set_malloc_failed_hook(ylib_malloc_failed_hook_t hook) { malloc_failed_hook = hook; }
void ylib_set_free_hook(ylib_free_hook_t hook) { free_hook = hook; }
void ylib_set_malloc_hook(ylib_malloc_hook_t hook) { malloc_hook = hook; }

/* 内存池相关实现略，可按需补充 */
