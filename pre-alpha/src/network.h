/* network.h
 */

#ifndef _CHAT_NETWORK_H_
#define _CHAT_NETWORK_H_

#define MAXUSERS    128

#include "buffers.h"

struct network_data {
    int port;
    char *ip_address;
};

struct peer_data {
    int port;
    int user_len;
    char *user_hash[MAXUSERS]; /* array of users associated with peer. */
    char *ip_address; /* ip of peer */
};

struct peers {
    struct peer_data peer_array[MAXUSERS]; /* array of peers */
    int peer_len;
};

void initialize_network(void *network_settings);

int send_msg(char *msg);

void push_to_line_list(line_buffer line);

#endif /* _CHAT_NETWORK_H_ */
