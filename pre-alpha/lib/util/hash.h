/* hash.h
 */

#ifndef _CHAT_LIB_HASH_H_
#define _CHAT_LIB_HASH_H_

typedef struct _list_node {
    char *val;
    char *key;
    struct _list_node *prev;
    struct _list_node *next;
} list_node;


#endif /* _CHAT_LIB_HASH_H_
