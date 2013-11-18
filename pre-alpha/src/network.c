/* network.c
 */
#include "network.h"
#include "display.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h> /* free */
#include <unistd.h>

/* initialize_network:
 * This initializes all network systems.  It's called to start the network
 * thread.
 */
void initialize_network(void *network_settings){
    int port = ((struct network_data *)network_settings)->port;
    (void) port; /* for now */
    
    free(network_settings);
}


