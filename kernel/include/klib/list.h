#ifndef KLIB_LIST_H
#define KLIB_LIST_H

#include <base/stdbool.h>
#include <base/stddef.h>
#include <base/stdint.h>

/**
 * @brief Node structure for intrusive, circular, doubly-linked list
 * The node is embedded directly into the structure that owns it
 */
struct list_node {
  struct list_node *next;
  struct list_node *prev;
};

/**
 * @brief Intrusive circular doubly-linked list.
 * Uses an embedded sentinel node as the list head
 *
 * The sentinel does not contain user data and is not considered an element of
 * the list.
 * The list is considered empty when both prev and next pointers
 * point to the head node itself.
 */
struct list {
  struct list_node head; // sentinel node
};

/**
 * @brief Static initializer for an empty list
 *
 * @param name Name of the list structure
 */
#define LIST_INIT(name)                                                        \
  {                                                                            \
    .head = {.next = &(name).head, .prev = &(name).head }                      \
  }

/**
 * @brief Initialize an empty list
 *
 * @param list
 */
static inline void list_init(struct list *list) {
  list->head.next = &list->head;
  list->head.prev = &list->head;
}

/**
 * @brief Checks whether a list is empty
 *
 * @param list
 * @return true if contains no nodes, false otherwise
 */
static inline bool list_empty(struct list *list) {
  return list->head.next == &list->head;
}

/**
 * @brief Returns the first none in a list
 *
 * @param list
 * @return First node, or NULL if the list is empty
 */
static inline struct list_node *list_front(struct list *list) {
  return list_empty(list) ? NULL : list->head.next;
}

/**
 * @brief Returns the last none in a list
 *
 * @param list
 * @return Last node, or NULL if the list is empty
 */
static inline struct list_node *list_back(struct list *list) {
  return list_empty(list) ? NULL : list->head.prev;
}

/**
 * @brief Inserts a node immediately after another node
 *
 * @param node Existing node after which to insert
 * @param new Node to insert
 *
 * @pre new must not be linked to another list
 */
static inline void list_insert_after(struct list_node *node,
                                     struct list_node *new) {
  new->prev = node;
  new->next = node->next;

  node->next->prev = new;
  node->next = new;
}

/**
 * @brief Insert a node immediately before another node
 *
 * @param node Existing node before which to insert
 * @param new Node to insert
 *
 * @pre new must not be linked to another list
 */
static inline void list_insert_before(struct list_node *node,
                                      struct list_node *new) {
  new->next = node;
  new->prev = node->prev;

  node->prev->next = new;
  node->prev = new;
}

/**
 * @brief Remove a node from a list
 *
 * @param node Node to remove
 *
 * @pre node must be linked to a list
 * @post node is no longer linked to any list
 */
static inline void list_remove(struct list_node *node) {
  node->prev->next = node->next;
  node->next->prev = node->prev;

  node->next = NULL;
  node->prev = NULL;
}

/**
 * @brief Inserts a node at the front of a list (after head sentinel)
 *
 * @param list
 * @param node Node to insert
 *
 * @pre node must not be linked to another list
 */
static inline void list_insert_front(struct list *list,
                                     struct list_node *node) {
  list_insert_after(&list->head, node);
}

/**
 * @brief Inserts a node at the back of a list (before head sentinel)
 *
 * @param list
 * @param node Node to insert
 *
 * @pre node must not be linked to another list
 */
static inline void list_insert_back(struct list *list, struct list_node *node) {
  list_insert_before(&list->head, node);
}

/**
 * @brief Removes and returns the first node from a list
 *
 * @param list
 * @return First node, or NULL if the list is empty
 */
static inline struct list_node *list_pop_front(struct list *list) {
  if (list_empty(list))
    return NULL;

  struct list_node *node = list->head.next;

  list_remove(node);

  return node;
}

/**
 * @brief Removes and returns the last node from a list
 *
 * @param list
 * @return Last node, or NULL if the list is empty
 */
static inline struct list_node *list_pop_back(struct list *list) {
  if (list_empty(list))
    return NULL;

  struct list_node *node = list->head.prev;

  list_remove(node);

  return node;
}

/**
 * @brief Replaces a node in a list with another node
 *
 * @param old_node Node currently linked to the list
 * @param new_node Node to insert in its place
 *
 * @pre old_node must be linked to a list
 * @pre new_node must not be linked to a list
 */
static inline void list_replace(struct list_node *old_node,
                                struct list_node *new_node) {
  new_node->prev = old_node->prev;
  new_node->next = old_node->next;

  old_node->prev->next = new_node;
  old_node->next->prev = new_node;

  old_node->next = NULL;
  old_node->prev = NULL;
}

/**
 * @brief Iterate over each node in the list from front to back
 */
#define list_for_each(node, list)                                              \
  for ((node) = (list)->head.next; node != &(list)->head; (node) = (node)->next)

#endif
