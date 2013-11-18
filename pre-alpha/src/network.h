/* network.h
 */

#ifndef _CHAT_NETWORK_H_
#define _CHAT_NETWORK_H_

struct network_data {
    int port;
    char *ip_address;
};

void initialize_network(void *network_settings);

#endif /* _CHAT_NETWORK_H_ */
