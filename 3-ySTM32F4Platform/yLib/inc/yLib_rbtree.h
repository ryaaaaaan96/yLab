/**
  ******************************************************************************
  * @file       yLib_rbtree.h
  * @brief      红黑树实现（基于Linux内核实现）
  * @author     Ryan
  * @version    V2.0.0
  * @date       2025/06/16
  * @note       
  ******************************************************************************
  */
#ifndef YLIB_RBTREE_H
#define YLIB_RBTREE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "yLib_def.h"

/**
 * @brief 红黑树节点颜色
 */
#define YLIB_RB_RED     0
#define YLIB_RB_BLACK   1

/**
 * @brief 红黑树节点结构
 */
struct ylib_rb_node {
    unsigned long  __rb_parent_color;
    struct ylib_rb_node *rb_right;
    struct ylib_rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));

/**
 * @brief 红黑树根结构
 */
struct ylib_rb_root {
    struct ylib_rb_node *rb_node;
};

/**
 * @brief 红黑树根初始化
 */
#define YLIB_RB_ROOT   (struct ylib_rb_root) { NULL, }

/**
 * @brief 获取父节点
 * @param node 节点指针
 * @return 父节点指针
 */
#define ylib_rb_parent(node) \
    ((struct ylib_rb_node *)((node)->__rb_parent_color & ~3))

/**
 * @brief 获取节点颜色
 * @param node 节点指针
 * @return 节点颜色
 */
#define ylib_rb_color(node) \
    ((node)->__rb_parent_color & 1)

/**
 * @brief 判断节点是否为红色
 * @param node 节点指针
 * @return 1表示红色，0表示黑色
 */
#define ylib_rb_is_red(node) \
    (!ylib_rb_color(node))

/**
 * @brief 判断节点是否为黑色
 * @param node 节点指针
 * @return 1表示黑色，0表示红色
 */
#define ylib_rb_is_black(node) \
    ylib_rb_color(node)

/**
 * @brief 设置父节点
 * @param node 节点指针
 * @param parent 父节点指针
 */
static inline void ylib_rb_set_parent(struct ylib_rb_node *node, 
                                      struct ylib_rb_node *parent)
{
    node->__rb_parent_color = ylib_rb_color(node) | (unsigned long)parent;
}

/**
 * @brief 设置节点颜色
 * @param node 节点指针
 * @param color 颜色
 */
static inline void ylib_rb_set_color(struct ylib_rb_node *node, int color)
{
    node->__rb_parent_color = (node->__rb_parent_color & ~1) | color;
}

/**
 * @brief 设置父节点和颜色
 * @param node 节点指针
 * @param parent 父节点指针
 * @param color 颜色
 */
static inline void ylib_rb_set_parent_color(struct ylib_rb_node *node,
                                            struct ylib_rb_node *parent, int color)
{
    node->__rb_parent_color = (unsigned long)parent | color;
}

/**
 * @brief 初始化红黑树节点
 * @param node 节点指针
 */
static inline void ylib_rb_init_node(struct ylib_rb_node *node)
{
    node->__rb_parent_color = 0;
    node->rb_left = NULL;
    node->rb_right = NULL;
}

/**
 * @brief 检查红黑树是否为空
 * @param root 树根指针
 * @return 1表示空，0表示非空
 */
static inline int ylib_rb_empty_root(struct ylib_rb_root *root)
{
    return root->rb_node == NULL;
}

/**
 * @brief 检查节点是否为空
 * @param node 节点指针
 * @return 1表示空，0表示非空
 */
static inline int ylib_rb_empty_node(struct ylib_rb_node *node)
{
    return node->__rb_parent_color == 0;
}

/**
 * @brief 清空节点
 * @param node 节点指针
 */
static inline void ylib_rb_clear_node(struct ylib_rb_node *node)
{
    node->__rb_parent_color = 0;
}

/* 核心操作函数 */

/**
 * @brief 插入节点并重新平衡树
 * @param node 要插入的节点
 * @param root 树根指针
 */
void ylib_rb_insert_color(struct ylib_rb_node *node, struct ylib_rb_root *root);

/**
 * @brief 删除节点并重新平衡树
 * @param node 要删除的节点
 * @param root 树根指针
 */
void ylib_rb_erase(struct ylib_rb_node *node, struct ylib_rb_root *root);

/**
 * @brief 替换节点
 * @param victim 被替换的节点
 * @param new_node 新节点
 * @param root 树根指针
 */
void ylib_rb_replace_node(struct ylib_rb_node *victim, struct ylib_rb_node *new_node,
                          struct ylib_rb_root *root);

/**
 * @brief 获取第一个节点（最小值）
 * @param root 树根指针
 * @return 第一个节点指针
 */
struct ylib_rb_node *ylib_rb_first(const struct ylib_rb_root *root);

/**
 * @brief 获取最后一个节点（最大值）
 * @param root 树根指针
 * @return 最后一个节点指针
 */
struct ylib_rb_node *ylib_rb_last(const struct ylib_rb_root *root);

/**
 * @brief 获取下一个节点
 * @param node 当前节点
 * @return 下一个节点指针
 */
struct ylib_rb_node *ylib_rb_next(const struct ylib_rb_node *node);

/**
 * @brief 获取前一个节点
 * @param node 当前节点
 * @return 前一个节点指针
 */
struct ylib_rb_node *ylib_rb_prev(const struct ylib_rb_node *node);

/* 链接和旋转操作（内部使用） */
void ylib_rb_link_node(struct ylib_rb_node *node, struct ylib_rb_node *parent,
                       struct ylib_rb_node **rb_link);

/* 遍历宏 */

/**
 * @brief 获取包含红黑树节点的结构体指针
 * @param ptr 节点指针
 * @param type 结构体类型
 * @param member 节点在结构体中的成员名
 */
#define ylib_rb_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * @brief 安全的获取包含红黑树节点的结构体指针
 * @param ptr 节点指针
 * @param type 结构体类型
 * @param member 节点在结构体中的成员名
 */
#define ylib_rb_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
       ____ptr ? ylib_rb_entry(____ptr, type, member) : NULL; \
    })

/**
 * @brief 遍历红黑树（正序）
 * @param pos 当前位置指针
 * @param root 树根指针
 */
#define ylib_rb_for_each(pos, root) \
    for (pos = ylib_rb_first(root); pos; pos = ylib_rb_next(pos))

/**
 * @brief 遍历红黑树（逆序）
 * @param pos 当前位置指针
 * @param root 树根指针
 */
#define ylib_rb_for_each_reverse(pos, root) \
    for (pos = ylib_rb_last(root); pos; pos = ylib_rb_prev(pos))

/**
 * @brief 安全遍历红黑树（正序，允许删除当前节点）
 * @param pos 当前位置指针
 * @param n 临时指针
 * @param root 树根指针
 */
#define ylib_rb_for_each_safe(pos, n, root) \
    for (pos = ylib_rb_first(root), n = pos ? ylib_rb_next(pos) : NULL; \
         pos; pos = n, n = pos ? ylib_rb_next(pos) : NULL)

/**
 * @brief 遍历红黑树中的实际数据（正序）
 * @param pos 当前数据指针
 * @param root 树根指针
 * @param member 红黑树节点成员名
 */
#define ylib_rb_for_each_entry(pos, root, member) \
    for (pos = ylib_rb_entry_safe(ylib_rb_first(root), typeof(*pos), member); \
         pos; pos = ylib_rb_entry_safe(ylib_rb_next(&pos->member), typeof(*pos), member))

/**
 * @brief 遍历红黑树中的实际数据（逆序）
 * @param pos 当前数据指针
 * @param root 树根指针
 * @param member 红黑树节点成员名
 */
#define ylib_rb_for_each_entry_reverse(pos, root, member) \
    for (pos = ylib_rb_entry_safe(ylib_rb_last(root), typeof(*pos), member); \
         pos; pos = ylib_rb_entry_safe(ylib_rb_prev(&pos->member), typeof(*pos), member))

/**
 * @brief 安全遍历红黑树中的实际数据（正序，允许删除当前节点）
 * @param pos 当前数据指针
 * @param n 临时指针
 * @param root 树根指针
 * @param member 红黑树节点成员名
 */
#define ylib_rb_for_each_entry_safe(pos, n, root, member) \
    for (pos = ylib_rb_entry_safe(ylib_rb_first(root), typeof(*pos), member), \
         n = ylib_rb_entry_safe(pos ? ylib_rb_next(&pos->member) : NULL, typeof(*pos), member); \
         pos; \
         pos = n, n = ylib_rb_entry_safe(pos ? ylib_rb_next(&pos->member) : NULL, typeof(*pos), member))

/* 辅助函数 */

/**
 * @brief 查找节点
 * @param root 树根指针
 * @param key 查找的键
 * @param cmp 比较函数
 * @return 找到的节点，未找到返回NULL
 */
struct ylib_rb_node *ylib_rb_find(struct ylib_rb_root *root, const void *key,
                                  int (*cmp)(const void *key, const struct ylib_rb_node *node));

/**
 * @brief 插入节点
 * @param root 树根指针
 * @param new_node 新节点
 * @param cmp 比较函数
 * @return 成功返回0，失败返回-1（节点已存在）
 */
int ylib_rb_insert(struct ylib_rb_root *root, struct ylib_rb_node *new_node,
                   int (*cmp)(const struct ylib_rb_node *a, const struct ylib_rb_node *b));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* YLIB_RBTREE_H */
