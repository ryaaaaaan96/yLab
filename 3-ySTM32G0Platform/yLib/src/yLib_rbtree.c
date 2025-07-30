/**
  ******************************************************************************
  * @file       yLib_rbtree.c
  * @brief      红黑树实现（核心算法，兼容Linux内核风格）
  * @author     Ryan
  * @version    V2.0.0
  * @date       2025/06/16
  * @note      
  ******************************************************************************
  */

#include "yLib_rbtree.h"

/*
 * 红黑树核心算法实现，参考Linux内核实现，适当精简
 */

static void ylib_rb_rotate_left(struct ylib_rb_node *node, struct ylib_rb_root *root)
{
    struct ylib_rb_node *right = node->rb_right;
    struct ylib_rb_node *parent = ylib_rb_parent(node);

    node->rb_right = right->rb_left;
    if (right->rb_left)
        ylib_rb_set_parent(right->rb_left, node);
    right->rb_left = node;
    ylib_rb_set_parent(right, parent);

    if (parent) {
        if (node == parent->rb_left)
            parent->rb_left = right;
        else
            parent->rb_right = right;
    } else {
        root->rb_node = right;
    }
    ylib_rb_set_parent(node, right);
}

static void ylib_rb_rotate_right(struct ylib_rb_node *node, struct ylib_rb_root *root)
{
    struct ylib_rb_node *left = node->rb_left;
    struct ylib_rb_node *parent = ylib_rb_parent(node);

    node->rb_left = left->rb_right;
    if (left->rb_right)
        ylib_rb_set_parent(left->rb_right, node);
    left->rb_right = node;
    ylib_rb_set_parent(left, parent);

    if (parent) {
        if (node == parent->rb_right)
            parent->rb_right = left;
        else
            parent->rb_left = left;
    } else {
        root->rb_node = left;
    }
    ylib_rb_set_parent(node, left);
}

void ylib_rb_link_node(struct ylib_rb_node *node, struct ylib_rb_node *parent,
                      struct ylib_rb_node **rb_link)
{
    node->__rb_parent_color = (unsigned long)parent;
    node->rb_left = node->rb_right = NULL;
    *rb_link = node;
}

void ylib_rb_insert_color(struct ylib_rb_node *node, struct ylib_rb_root *root)
{
    struct ylib_rb_node *parent, *gparent;

    while ((parent = ylib_rb_parent(node)) && ylib_rb_is_red(parent)) {
        gparent = ylib_rb_parent(parent);
        if (parent == gparent->rb_left) {
            struct ylib_rb_node *uncle = gparent->rb_right;
            if (uncle && ylib_rb_is_red(uncle)) {
                ylib_rb_set_color(uncle, YLIB_RB_BLACK);
                ylib_rb_set_color(parent, YLIB_RB_BLACK);
                ylib_rb_set_color(gparent, YLIB_RB_RED);
                node = gparent;
                continue;
            }
            if (parent->rb_right == node) {
                node = parent;
                ylib_rb_rotate_left(node, root);
                parent = ylib_rb_parent(node);
                gparent = ylib_rb_parent(parent);
            }
            ylib_rb_set_color(parent, YLIB_RB_BLACK);
            ylib_rb_set_color(gparent, YLIB_RB_RED);
            ylib_rb_rotate_right(gparent, root);
        } else {
            struct ylib_rb_node *uncle = gparent->rb_left;
            if (uncle && ylib_rb_is_red(uncle)) {
                ylib_rb_set_color(uncle, YLIB_RB_BLACK);
                ylib_rb_set_color(parent, YLIB_RB_BLACK);
                ylib_rb_set_color(gparent, YLIB_RB_RED);
                node = gparent;
                continue;
            }
            if (parent->rb_left == node) {
                node = parent;
                ylib_rb_rotate_right(node, root);
                parent = ylib_rb_parent(node);
                gparent = ylib_rb_parent(parent);
            }
            ylib_rb_set_color(parent, YLIB_RB_BLACK);
            ylib_rb_set_color(gparent, YLIB_RB_RED);
            ylib_rb_rotate_left(gparent, root);
        }
    }
    ylib_rb_set_color(root->rb_node, YLIB_RB_BLACK);
}

void ylib_rb_erase(struct ylib_rb_node *node, struct ylib_rb_root *root)
{
    struct ylib_rb_node *child, *parent;
    int color;

    if (!node->rb_left)
        child = node->rb_right;
    else if (!node->rb_right)
        child = node->rb_left;
    else {
        struct ylib_rb_node *old = node, *left;
        node = node->rb_right;
        while ((left = node->rb_left) != NULL)
            node = left;
        if (ylib_rb_parent(old)) {
            if (ylib_rb_parent(old)->rb_left == old)
                ylib_rb_parent(old)->rb_left = node;
            else
                ylib_rb_parent(old)->rb_right = node;
        } else {
            root->rb_node = node;
        }
        child = node->rb_right;
        parent = ylib_rb_parent(node);
        color = ylib_rb_color(node);
        if (parent == old) {
            parent = node;
        } else {
            if (child)
                ylib_rb_set_parent(child, parent);
            parent->rb_left = child;
            node->rb_right = old->rb_right;
            ylib_rb_set_parent(old->rb_right, node);
        }
        node->__rb_parent_color = old->__rb_parent_color;
        node->rb_left = old->rb_left;
        ylib_rb_set_parent(old->rb_left, node);
        goto color_fixup;
    }
    parent = ylib_rb_parent(node);
    color = ylib_rb_color(node);
    if (child)
        ylib_rb_set_parent(child, parent);
    if (parent) {
        if (parent->rb_left == node)
            parent->rb_left = child;
        else
            parent->rb_right = child;
    } else {
        root->rb_node = child;
    }
color_fixup:
    if (color == YLIB_RB_BLACK) {
        while (child != root->rb_node && (!child || ylib_rb_is_black(child))) {
            if (child == parent->rb_left) {
                struct ylib_rb_node *sibling = parent->rb_right;
                if (ylib_rb_is_red(sibling)) {
                    ylib_rb_set_color(sibling, YLIB_RB_BLACK);
                    ylib_rb_set_color(parent, YLIB_RB_RED);
                    ylib_rb_rotate_left(parent, root);
                    sibling = parent->rb_right;
                }
                if ((!sibling->rb_left || ylib_rb_is_black(sibling->rb_left)) &&
                    (!sibling->rb_right || ylib_rb_is_black(sibling->rb_right))) {
                    ylib_rb_set_color(sibling, YLIB_RB_RED);
                    child = parent;
                    parent = ylib_rb_parent(child);
                } else {
                    if (!sibling->rb_right || ylib_rb_is_black(sibling->rb_right)) {
                        if (sibling->rb_left)
                            ylib_rb_set_color(sibling->rb_left, YLIB_RB_BLACK);
                        ylib_rb_set_color(sibling, YLIB_RB_RED);
                        ylib_rb_rotate_right(sibling, root);
                        sibling = parent->rb_right;
                    }
                    ylib_rb_set_color(sibling, ylib_rb_color(parent));
                    ylib_rb_set_color(parent, YLIB_RB_BLACK);
                    if (sibling->rb_right)
                        ylib_rb_set_color(sibling->rb_right, YLIB_RB_BLACK);
                    ylib_rb_rotate_left(parent, root);
                    child = root->rb_node;
                    break;
                }
            } else {
                struct ylib_rb_node *sibling = parent->rb_left;
                if (ylib_rb_is_red(sibling)) {
                    ylib_rb_set_color(sibling, YLIB_RB_BLACK);
                    ylib_rb_set_color(parent, YLIB_RB_RED);
                    ylib_rb_rotate_right(parent, root);
                    sibling = parent->rb_left;
                }
                if ((!sibling->rb_left || ylib_rb_is_black(sibling->rb_left)) &&
                    (!sibling->rb_right || ylib_rb_is_black(sibling->rb_right))) {
                    ylib_rb_set_color(sibling, YLIB_RB_RED);
                    child = parent;
                    parent = ylib_rb_parent(child);
                } else {
                    if (!sibling->rb_left || ylib_rb_is_black(sibling->rb_left)) {
                        if (sibling->rb_right)
                            ylib_rb_set_color(sibling->rb_right, YLIB_RB_BLACK);
                        ylib_rb_set_color(sibling, YLIB_RB_RED);
                        ylib_rb_rotate_left(sibling, root);
                        sibling = parent->rb_left;
                    }
                    ylib_rb_set_color(sibling, ylib_rb_color(parent));
                    ylib_rb_set_color(parent, YLIB_RB_BLACK);
                    if (sibling->rb_left)
                        ylib_rb_set_color(sibling->rb_left, YLIB_RB_BLACK);
                    ylib_rb_rotate_right(parent, root);
                    child = root->rb_node;
                    break;
                }
            }
        }
        if (child)
            ylib_rb_set_color(child, YLIB_RB_BLACK);
    }
}

void ylib_rb_replace_node(struct ylib_rb_node *victim, struct ylib_rb_node *new_node,
                         struct ylib_rb_root *root)
{
    struct ylib_rb_node *parent = ylib_rb_parent(victim);
    if (parent) {
        if (parent->rb_left == victim)
            parent->rb_left = new_node;
        else
            parent->rb_right = new_node;
    } else {
        root->rb_node = new_node;
    }
    if (new_node)
        ylib_rb_set_parent_color(new_node, parent, ylib_rb_color(victim));
}

struct ylib_rb_node *ylib_rb_first(const struct ylib_rb_root *root)
{
    struct ylib_rb_node *n = root->rb_node;
    if (!n)
        return NULL;
    while (n->rb_left)
        n = n->rb_left;
    return n;
}

struct ylib_rb_node *ylib_rb_last(const struct ylib_rb_root *root)
{
    struct ylib_rb_node *n = root->rb_node;
    if (!n)
        return NULL;
    while (n->rb_right)
        n = n->rb_right;
    return n;
}

struct ylib_rb_node *ylib_rb_next(const struct ylib_rb_node *node)
{
    struct ylib_rb_node *parent;
    if (node->rb_right) {
        node = node->rb_right;
        while (node->rb_left)
            node = node->rb_left;
        return (struct ylib_rb_node *)node;
    }
    while ((parent = ylib_rb_parent(node)) && node == parent->rb_right)
        node = parent;
    return (struct ylib_rb_node *)parent;
}

struct ylib_rb_node *ylib_rb_prev(const struct ylib_rb_node *node)
{
    struct ylib_rb_node *parent;
    if (node->rb_left) {
        node = node->rb_left;
        while (node->rb_right)
            node = node->rb_right;
        return (struct ylib_rb_node *)node;
    }
    while ((parent = ylib_rb_parent(node)) && node == parent->rb_left)
        node = parent;
    return (struct ylib_rb_node *)parent;
}

struct ylib_rb_node *ylib_rb_find(struct ylib_rb_root *root, const void *key,
                                  int (*cmp)(const void *key, const struct ylib_rb_node *node))
{
    struct ylib_rb_node *node = root->rb_node;
    int result;
    while (node) {
        result = cmp(key, node);
        if (result < 0)
            node = node->rb_left;
        else if (result > 0)
            node = node->rb_right;
        else
            return node;
    }
    return NULL;
}

int ylib_rb_insert(struct ylib_rb_root *root, struct ylib_rb_node *new_node,
                   int (*cmp)(const struct ylib_rb_node *a, const struct ylib_rb_node *b))
{
    struct ylib_rb_node **link = &root->rb_node, *parent = NULL;
    int result;
    while (*link) {
        parent = *link;
        result = cmp(new_node, parent);
        if (result < 0)
            link = &parent->rb_left;
        else if (result > 0)
            link = &parent->rb_right;
        else
            return -1; // 已存在
    }
    ylib_rb_link_node(new_node, parent, link);
    ylib_rb_insert_color(new_node, root);
    return 0;
}
