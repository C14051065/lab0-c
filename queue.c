#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new_queue = malloc(sizeof(struct list_head));

    if (new_queue)
        INIT_LIST_HEAD(new_queue);

    return new_queue;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *safe, *entry;
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);

    free(head);

    return;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *node = malloc(sizeof(element_t));
    if (!node) {
        return false;
    }
    char *str = malloc(sizeof(char) * strlen(s) + 1);
    if (!str) {
        free(node);
        return false;
    }
    strncpy(str, s, strlen(s) + 1);
    node->value = str;
    list_add(&node->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *node = malloc(sizeof(element_t));
    if (!node) {
        return false;
    }
    char *str = malloc(sizeof(char) * strlen(s) + 1);
    if (!str) {
        free(node);
        return false;
    }
    strncpy(str, s, strlen(s) + 1);
    node->value = str;
    list_add_tail(&node->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *rm_element = list_first_entry(head, element_t, list);
    list_del_init(&rm_element->list);

    if (!sp || !bufsize)
        return rm_element;

    strncpy(sp, rm_element->value, bufsize);
    sp[bufsize - 1] = '\0';

    return rm_element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *rm_element = list_last_entry(head, element_t, list);
    list_del_init(&rm_element->list);

    if (!sp || !bufsize)
        return rm_element;

    strncpy(sp, rm_element->value, bufsize);
    sp[bufsize - 1] = '\0';

    return rm_element;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *fast = head->next, *slow = head->next;

    for (; fast != head && fast->next != head; fast = fast->next->next) {
        slow = slow->next;
    }

    list_del_init(slow);

    q_release_element(list_entry(slow, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;

    q_sort(head, 0);

    element_t *cur, *next;
    bool del = false;
    list_for_each_entry_safe (cur, next, head, list) {
        if (&next->list != head && !strcmp(cur->value, next->value)) {
            list_del(&cur->list);
            q_release_element(cur);
            del = true;
        } else if (del) {
            del = false;
            list_del(&cur->list);
            q_release_element(cur);
        }
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node = head;
    struct list_head *prev = NULL, *next = NULL;

    do {
        next = node->next;
        prev = node->prev;
        node->prev = next;
        node->next = prev;
        node = next;
    } while (node != head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;

    struct list_head *it, *safe, *cut;
    int cnt = k;
    cut = head;
    list_for_each_safe (it, safe, head) {
        if (--cnt)
            continue;
        LIST_HEAD(tmp);
        cnt = k;
        list_cut_position(&tmp, cut, it);
        q_reverse(&tmp);
        list_splice(&tmp, cut);  // add tmp at cut
        cut = safe->prev;
    }
}

static void merge_sort(struct list_head *first,
                       struct list_head *second,
                       bool descend)
{
    if (!first || !second)
        return;

    LIST_HEAD(tmp);
    while (!list_empty(first) && !list_empty(second)) {
        element_t *first_entry = list_first_entry(first, element_t, list);
        element_t *second_entry = list_first_entry(second, element_t, list);
        int cmp = strcmp(first_entry->value, second_entry->value);
        if (descend)
            cmp = -cmp;
        if (cmp <= 0)
            list_move_tail(&first_entry->list, &tmp);
        else
            list_move_tail(&second_entry->list, &tmp);
    }

    list_splice(&tmp, first);
    list_splice_tail_init(second, first);

    return;
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    // sort by merge sort
    // divide to one last
    struct list_head *slow = head, *fast = head;
    for (; fast->next != head && fast->next->next != head;
         fast = fast->next->next) {
        slow = slow->next;
    }

    LIST_HEAD(second);
    list_cut_position(&second, slow, head->prev);

    q_sort(head, descend);
    q_sort(&second, descend);

    merge_sort(head, &second, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    struct list_head *li = head->next;
    struct list_head *next = li->next;
    int count = 1;

    while (next != head) {
        element_t *li_entry = list_entry(li, element_t, list),
                  *next_entry = list_entry(next, element_t, list);
        if (strcmp(li_entry->value, next_entry->value) > 0) {
            list_del(next);
            q_release_element(next_entry);
            next_entry = list_entry(li->next, element_t, list);
        } else {
            count++;
            li = li->next;
        }
    }

    return count;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    struct list_head *pos = head->prev;
    element_t *entry = list_entry(pos, element_t, list);
    char *max = entry->value;
    int count = 1;
    pos = pos->prev;

    while (pos != head) {
        entry = list_entry(pos, element_t, list);
        if (strcmp(entry->value, max) > 0) {
            max = entry->value;
            count++;
        } else {
            struct list_head *tmp = pos->prev;
            list_del(pos);
            q_release_element(entry);
            pos = tmp;
            continue;
        }
        pos = pos->prev;
    }

    return count;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;

    queue_contex_t *qhead = list_first_entry(head, queue_contex_t, chain);
    list_del_init(&qhead->chain);
    queue_contex_t *cur = NULL;

    list_for_each_entry (cur, head, chain) {
        list_splice_init(cur->q, qhead->q);
        qhead->size += cur->size;
    }

    list_add(&qhead->chain, head);
    q_sort(qhead->q, descend);

    return qhead->size;
}
