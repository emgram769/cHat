/* cHat.c
 * The main file for the cHat chat client.
 */

#include <stdio.h> /* printf */
#include <stdlib.h> /* atoi */
#include <getopt.h>
#include <curses.h>
#include <pthread.h>

#include "display.h"
#include "network.h"

#define DEFAULT_PORT 1337

/* global buffers. */

typedef struct _chat_buffer {
    unsigned int length;
    char *text;
} chat_buffer;

/* initialize_video:
 * Initializes the video portion of the chat client.
 */
void initialize_video(void) {
    printf("Not implemented\n");
    return;
}

/* print_usage:
 * Prints the usage of cHat. */
void print_usage(void) {
    printf("Usage:  cHat -i <ip> [-p <port> [-v [-h]]]\n");
    return;
}

/* main:
 * The main function parses command line arguments and creates new threads to
 * run all the necessary routines.
 */
int main(int argc, char *argv[]) {


    struct network_data *settings = malloc(sizeof(struct network_data));
    char opt;

    /* Default port. */
    settings->port = DEFAULT_PORT;
    settings->ip_address = NULL;

    while(-1 != (opt = getopt(argc, argv, "hvp:i:"))){
        switch(opt){
            case 'h':
                print_usage();
                return 0;
            case 'p':
                settings->port = atoi(optarg);
                break;
            case 'i':
                settings->ip_address = optarg;
                break;
            case 'v':
                initialize_video();
                break;
            default:
                printf("You done goofed\n");
                break;
        }
    }
 
    /* spawn network thread. */
    pthread_t network_thread;
    if((pthread_create(&network_thread, NULL,
        (void *)initialize_network, (void *)settings))==-1)
        printf("error initializing network thread.\n");
    
    if((pthread_detach(network_thread))==-1)
        printf("error detatching thread.\n");

    initialize_display();

    while(1){ /* ...and 100% CPU usage */
        int c = getch();  /* refresh, accept single keystroke of input. */
        if (c == 13){
            /* enter key */
            draw_xy(COLS / 2, LINES - 1, 's', 1);
        } else if (c == 127){
            /* backspace */
        } else if (c == 27){
            /* esc key */
            quit_dialogue();
        } else if (c > 0) {
            /* any other key */
        }
    }

    cleanup_display();
    return 0;
}
