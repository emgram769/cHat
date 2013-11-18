/* network.h*
 */

#ifndef _CHAT_NETWORK_H_
#define _CHAT_NETWORK_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h> /* free */

struct network_data {
    int port;
    char *ip_address;
};

void initialize_network(void *network_settings);

#endif /* _CHAT_NETWORK_H_ */
