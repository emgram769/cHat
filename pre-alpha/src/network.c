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


/* parse_buf:
 * returns 1 if the message of the passed in
 * buffer should be forwarded.
 * 0 if it should not.
 * It really should do other stuff in the future.
 * Fuck parsing in C. 
 */
int parse_buf(char *buf) {
    if (buf == NULL)
        return -1;
    int i;
    char *check = "00000";
    for (i=0;i<4;i++) {
        if (check[i] != buf[i])
            return 0;
    }
    return 1;
}

/* send_msg:
 * simply opens a connection and
 * sends the message. Returns 0 on success
 * -1 on failure.
 */
int send_msg(char *msg) {
    (void)msg;
    return 0;
}

/* wait_for_connnection:
 * called by the initializer, this is the main function that is used
 * after set up of the socket structs.
 */
void wait_for_connection(struct sockaddr_in clientaddr, struct sockaddr_in forwardaddr,
                         int listenfd, char buf[BUFSIZE], struct network_data *network_settings) {

    socklen_t clientlen = sizeof(clientaddr); /* for accept, stupid socket interface */
    
    int port = network_settings->port;
    char *ip_address = network_settings->ip_address;

    int connfd, forwardfd; /* initialize file descriptors */
        
    while(1) {
        int n;
        /* we got a connection! */
        if ((connfd = accept(listenfd,
                (struct sockaddr *) &clientaddr, &clientlen)) < 0)
            error_handler("listener accept error");

        /* start up a connection to our friend's ip. */
        socklen_t forwardlen;
        forwardaddr.sin_family = AF_INET; /* protocol */
        forwardaddr.sin_port = htons(port);
        forwardaddr.sin_addr.s_addr = inet_addr(ip_address);
        forwardlen = sizeof(forwardaddr);
    
        /* now to open the forwarder. */
        if ((forwardfd = socket(AF_INET, SOCK_STREAM, 0))<0)
            error_handler("forward socket error");

        if (connect(forwardfd,
                (struct sockaddr *) &forwardaddr, forwardlen) < 0)
            error_handler_display("forward connect error");

        /* initialize recieve buffer. */
        memset(buf,0,BUFSIZE);
        if ((n = read(connfd, buf, BUFSIZE)) < 0) /* read bufsize into buf. */
            error_handler("connection read error");
        
        clear_display();
        write_xy(0,0,buf,1);
        
        if (!parse_buf(buf))
        if ((n = write(forwardfd, buf, BUFSIZE)) < 0) /* write buf into forwarded peer. */
            error_handler_display("forward write error");

        /* wrap up connection. */
        close(forwardfd);
        close(connfd);
    }


}

/* initialize_network:
 * This initializes all network systems.  It's called to start the network
 * thread.
 */
void initialize_network(void *network_settings) {
    int port = ((struct network_data *)network_settings)->port;
    char *ip_address = ((struct network_data *)network_settings)->ip_address;
    if (!ip_address)
        error_handler("no ip address");
    int listenfd; /* socket fds. */

    /* to recieve and send. */
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    struct sockaddr_in forwardaddr;

    char buf[BUFSIZE]; /* may want to change this. */

    /* now to open the listener. */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0))<0)
        error_handler("listener socket error");
    
    serveraddr.sin_family = AF_INET; /* protocol */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* open acceptance of connections */
    serveraddr.sin_port = htons((unsigned short)port); /* big endian the port */

    if (bind(listenfd, (struct sockaddr *) &serveraddr, 
               sizeof(serveraddr)) < 0) 
        error_handler("listener binding error");

    if (listen(listenfd, MAXUSERS) < 0) /* allow requests to queue up */ 
        error_handler("listener listen error");

    /* wait for a new connection now that everything is set up */
    wait_for_connection(clientaddr, forwardaddr, listenfd, buf,
                        (struct network_data *)network_settings); 

    free(network_settings);
}


