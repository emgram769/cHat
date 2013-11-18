/* cHat.c
 * The main file for the cHat chat client.
 */

#include <stdio.h>  /* printf */
#include <stdlib.h>  /* atoi */
#include <getopt.h>

#include "display.h"

#define DEFAULT_PORT 1337

/* initialize_video:
 * Initializes the video portion of the chat client.
 */
void initialize_video(void) {
    printf("Haha Not implemented\n");
    return;
}

/* print_usage:
 * Prints the usage of cHat.
 */
void print_usage(void) {
    printf("Usage:  cHat ....... to be written later by Bram.\n");
    return;
}

/* main:
 * The main function parses command line arguments and creates new threads to
 * run all the necessary routines.
 */
int main(int argc, char *argv[]) {
    int port;
    char *ip_address;
    char opt;

    /* Default port. */
    port = DEFAULT_PORT;
    ip_address = NULL;

    while(-1 != (opt = getopt(argc, argv, "hvp:i:"))){
        switch(opt){
            case 'h':
                print_usage();
                return 0;
            case 'p':
                port = atoi(optarg);
                break;
            case 'i':
                ip_address = optarg;
                break;
            case 'v':
                initialize_video();
                break;
            default:
                printf("You done goofed\n");
                break;
        }
    }

    if (ip_address)
        printf("ip_address: %s\n", ip_address);
    printf("port: %d\n", port);

    initialize_display();

    cleanup_display();
    return 0;
}







