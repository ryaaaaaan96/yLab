/**
  ******************************************************************************
  * @file       yLib_list.h
  * @brief      Linux内核风格的双向循环链表实现
  * @author     Ryan
  * @version    V2.0.0
  * @date       2025/06/16
  * @note       
  ******************************************************************************
  */
#ifndef YLIB_LIST_H
#define YLIB_LIST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "yLib_def.h"

/**
 * @brief 双向循环链表头结构
 */
struct ylib_list_head {
    struct ylib_list_head *next, *prev;
};

/**
 * @brief 静态初始化链表头
 */
#define YLIB_LIST_HEAD_INIT(name) { &(name), &(name) }

/**
 * @brief 定义并初始化链表头
 */
#define YLIB_LIST_HEAD(name) \
    struct ylib_list_head name = YLIB_LIST_HEAD_INIT(name)

/**
 * @brief 初始化链表头
 * @param list 链表头指针
 */
static inline void YLIB_INIT_LIST_HEAD(struct ylib_list_head *list)
{
    list->next = list;
    list->prev = list;
}

/**
 * @brief 在两个已知节点之间插入新节点
 * @param new_node 新节点
 * @param prev 前一个节点
 * @param next 后一个节点
 */
static inline void __ylib_list_add(struct ylib_list_head *new_node,
                                   struct ylib_list_head *prev,
                                   struct ylib_list_head *next)
{
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}

/**
 * @brief 在指定节点后添加新节点
 * @param new_node 新节点
 * @param head 链表头或指定节点
 */
static inline void ylib_list_add(struct ylib_list_head *new_node, 
                                 struct ylib_list_head *head)
{
    __ylib_list_add(new_node, head, head->next);
}

/**
 * @brief 在指定节点前添加新节点（尾部添加）
 * @param new_node 新节点
 * @param head 链表头
 */
static inline void ylib_list_add_tail(struct ylib_list_head *new_node,
                                      struct ylib_list_head *head)
{
    __ylib_list_add(new_node, head->prev, head);
}

/**
 * @brief 删除两个节点之间的连接
 * @param prev 前一个节点
 * @param next 后一个节点
 */
static inline void __ylib_list_del(struct ylib_list_head *prev,
                                   struct ylib_list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * @brief 删除节点
 * @param entry 要删除的节点
 */
static inline void ylib_list_del(struct ylib_list_head *entry)
{
    __ylib_list_del(entry->prev, entry->next);
    entry->next = (struct ylib_list_head *)0xDEADBEEF;
    entry->prev = (struct ylib_list_head *)0xBEEFDEAD;
}

/**
 * @brief 删除节点并重新初始化
 * @param entry 要删除的节点
 */
static inline void ylib_list_del_init(struct ylib_list_head *entry)
{
    __ylib_list_del(entry->prev, entry->next);
    YLIB_INIT_LIST_HEAD(entry);
}

/**
 * @brief 移动节点到新位置
 * @param list 要移动的节点
 * @param head 目标链表头
 */
static inline void ylib_list_move(struct ylib_list_head *list,
                                  struct ylib_list_head *head)
{
    __ylib_list_del(list->prev, list->next);
    ylib_list_add(list, head);
}

/**
 * @brief 移动节点到链表尾部
 * @param list 要移动的节点
 * @param head 目标链表头
 */
static inline void ylib_list_move_tail(struct ylib_list_head *list,
                                       struct ylib_list_head *head)
{
    __ylib_list_del(list->prev, list->next);
    ylib_list_add_tail(list, head);
}

/**
 * @brief 检查链表是否为空
 * @param head 链表头
 * @return 1表示空，0表示非空
 */
static inline int ylib_list_empty(const struct ylib_list_head *head)
{
    return head->next == head;
}

/**
 * @brief 获取包含链表节点的结构体指针
 * @param ptr 链表节点指针
 * @param type 包含该节点的结构体类型
 * @param member 节点在结构体中的成员名
 */
#define ylib_list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * @brief 获取第一个元素
 * @param head 链表头
 * @param type 结构体类型
 * @param member 链表节点成员名
 */
#define ylib_list_first_entry(head, type, member) \
    ylib_list_entry((head)->next, type, member)

/**
 * @brief 获取最后一个元素
 * @param head 链表头
 * @param type 结构体类型
 * @param member 链表节点成员名
 */
#define ylib_list_last_entry(head, type, member) \
    ylib_list_entry((head)->prev, type, member)

/**
 * @brief 遍历链表
 * @param pos 当前位置指针
 * @param head 链表头
 */
#define ylib_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * @brief 安全遍历链表（允许删除当前节点）
 * @param pos 当前位置指针
 * @param n 临时指针
 * @param head 链表头
 */
#define ylib_list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

/**
 * @brief 遍历链表中的实际数据
 * @param pos 当前数据指针
 * @param head 链表头
 * @param member 链表节点成员名
 */
#define ylib_list_for_each_entry(pos, head, member) \
    for (pos = ylib_list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = ylib_list_entry(pos->member.next, typeof(*pos), member))

/**
 * @brief 安全遍历链表中的实际数据
 * @param pos 当前数据指针
 * @param n 临时指针
 * @param head 链表头
 * @param member 链表节点成员名
 */
#define ylib_list_for_each_entry_safe(pos, n, head, member) \
    for (pos = ylib_list_entry((head)->next, typeof(*pos), member), \
         n = ylib_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = n, n = ylib_list_entry(n->member.next, typeof(*n), member))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* YLIB_LIST_H */
