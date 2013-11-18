/* network.c
 */
#include "network.h"
#include "display.h"
#include "util.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h> /* free */
#include <unistd.h>

#define BUFSIZE 1024

/* initialize_network:
 * This initializes all network systems.  It's called to start the network
 * thread.
 */
void initialize_network(void *network_settings){
    int port = ((struct network_data *)network_settings)->port;
    int listenfd;//, connfd; /* listen for new connections and current connection. */
    (void) port; /* for now. */

    /* to recieve and send. */
    //struct sockaddr_in serveraddr;
    //struct sockaddr_in clientaddr;

    //char buf[BUFSIZE]; /* may want to change this. */

    /* now to open the listener. */
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0))<0)
        error_handler("listener socket error");


    free(network_settings);
}


