/* network.c
 */
#include "network.h"
#include "display.h"
#include "util.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h> /* free */
#include <string.h> /* memset */
#include <unistd.h>

#define BUFSIZE 1024

/* initialize_network:
 * This initializes all network systems.  It's called to start the network
 * thread.
 */
void initialize_network(void *network_settings){
    int port = ((struct network_data *)network_settings)->port;
    char *ip_address = ((struct network_data *)network_settings)->ip_address;
    if(!ip_address)
        error_handler("no ip address");
    int listenfd, connfd; /* listen for new connections and current connection. */
    socklen_t clientlen; /* for accept */

    /* to recieve and send. */
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;

    char buf[BUFSIZE]; /* may want to change this. */

    /* now to open the listener. */
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0))<0)
        error_handler("listener socket error");
    
    serveraddr.sin_family = AF_INET; /* protocol */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* open acceptance of connections */
    serveraddr.sin_port = htons((unsigned short)port); /* big endian the port */

    if (bind(listenfd, (struct sockaddr *) &serveraddr, 
               sizeof(serveraddr)) < 0) 
        error_handler("listener binding error");

    if (listen(listenfd, 5) < 0) /* allow 5 requests to queue up */ 
        error_handler("listener listen error");

    clientlen = sizeof(clientaddr);

    while(1){
        int n;
        /* we got a connection! */
        if((connfd = accept(listenfd,
                (struct sockaddr *) &clientaddr, &clientlen)) < 0)
            error_handler("listener accept error");

        /* start up a connection to our friend's ip. */
        struct sockaddr_in forwardaddr;
        int forwardfd;
        socklen_t forwardlen;
        forwardaddr.sin_family = AF_INET; /* protocol */
        forwardaddr.sin_port = htons(port);
        forwardaddr.sin_addr.s_addr = inet_addr(ip_address);
        forwardlen = sizeof(forwardaddr);
    
        /* now to open the forwarder. */
        if((forwardfd = socket(AF_INET, SOCK_STREAM, 0))<0)
            error_handler("forward socket error");

        if(connect(forwardfd,
                (struct sockaddr *) &forwardaddr, forwardlen) < 0)
            error_handler("forward connect error");

        /* initialize recieve buffer. */
        memset(buf,0,BUFSIZE);
        if((n = read(connfd, buf, BUFSIZE)) < 0) /* read bufsize into buf. */
            error_handler("connection read error");

        if((n = write(forwardfd, buf, BUFSIZE)) < 0) /* write buf into forwarded peer. */
            error_handler("forward write error");

        /* wrap up connection. */
        close(forwardfd);
        close(connfd);
    }

    free(network_settings);
}


