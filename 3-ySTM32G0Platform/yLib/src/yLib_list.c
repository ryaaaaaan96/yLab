/**
  ******************************************************************************
  * @file       yLib_list.c
  * @brief      Linux内核风格的双向循环链表实现
  * @author     Ryan
  * @version    V2.0.0
  * @date       2025/06/16
  * @note       
  ******************************************************************************
  */

#include "yLib_list.h"

/* 
 * 双向循环链表主要使用内联函数实现，这里提供一些辅助函数
 */

/**
 * @brief 合并两个链表
 * @param list1 第一个链表头
 * @param list2 第二个链表头
 * @note 将list2合并到list1后面，list2变为空链表
 */
void ylib_list_splice(struct ylib_list_head *list1, struct ylib_list_head *list2)
{
    if (!ylib_list_empty(list2)) {
        struct ylib_list_head *first = list2->next;
        struct ylib_list_head *last = list2->prev;
        struct ylib_list_head *at = list1->next;

        first->prev = list1;
        list1->next = first;

        last->next = at;
        at->prev = last;

        YLIB_INIT_LIST_HEAD(list2);
    }
}

/**
 * @brief 将链表合并到指定位置之后
 * @param list 要合并的链表
 * @param head 目标位置
 */
void ylib_list_splice_tail(struct ylib_list_head *list, struct ylib_list_head *head)
{
    if (!ylib_list_empty(list)) {
        struct ylib_list_head *first = list->next;
        struct ylib_list_head *last = list->prev;
        struct ylib_list_head *at = head->prev;

        first->prev = at;
        at->next = first;

        last->next = head;
        head->prev = last;

        YLIB_INIT_LIST_HEAD(list);
    }
}

/**
 * @brief 分割链表
 * @param head 原链表头
 * @param entry 分割点
 * @param new_head 新链表头
 * @note 将从entry开始的部分移动到new_head
 */
void ylib_list_cut_position(struct ylib_list_head *new_head,
                            struct ylib_list_head *head,
                            struct ylib_list_head *entry)
{
    if (ylib_list_empty(head)) {
        YLIB_INIT_LIST_HEAD(new_head);
        return;
    }

    if (head->next == entry && head->prev == entry) {
        YLIB_INIT_LIST_HEAD(new_head);
        return;
    }

    new_head->next = head->next;
    new_head->next->prev = new_head;
    new_head->prev = entry;
    entry->next = new_head;
    head->next = entry->next;
    head->next->prev = head;
}

/**
 * @brief 计算链表长度
 * @param head 链表头
 * @return 链表中元素个数
 */
size_t ylib_list_count(struct ylib_list_head *head)
{
    struct ylib_list_head *pos;
    size_t count = 0;

    ylib_list_for_each(pos, head) {
        count++;
    }

    return count;
}

/**
 * @brief 检查链表是否只有一个元素
 * @param head 链表头
 * @return 1表示只有一个元素，0表示不是
 */
int ylib_list_is_singular(const struct ylib_list_head *head)
{
    return !ylib_list_empty(head) && (head->next == head->prev);
}

/**
 * @brief 旋转链表（将第一个元素移到最后）
 * @param head 链表头
 */
void ylib_list_rotate_left(struct ylib_list_head *head)
{
    struct ylib_list_head *first;

    if (!ylib_list_empty(head)) {
        first = head->next;
        ylib_list_move_tail(first, head);
    }
}
