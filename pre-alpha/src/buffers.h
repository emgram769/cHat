/* buffers.h
 */

#ifndef _CHAT_BUFFERS_H_
#define _CHAT_BUFFERS_H_

/* global buffers. */

typedef struct _line_buffer {
    unsigned int length;
    unsigned int max_length;
    char *text;
} line_buffer;

typedef struct _line_buffer_node {
    line_buffer line;
    struct _line_buffer_node *prev;
} line_buffer_node;

typedef struct _line_buffer_list {
    line_buffer_node *head;
    line_buffer_node *curr;
    line_buffer_node *tail;
} line_buffer_list;

#endif /* _CHAT_BUFFERS_H_ */
