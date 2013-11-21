/* cHat.c
 * The main file for the cHat chat client.
 */

#include <stdio.h> /* printf */
#include <stdlib.h> /* atoi */
#include <string.h> /* memcpy */
#include <getopt.h>
#include <curses.h>
#include <pthread.h>

#include "display.h"
#include "network.h"

#define DEFAULT_PORT 1337

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

line_buffer curr_line;
line_buffer_list *line_list;


/* initialize_buffers:
 * Initializes the text buffers used by the client.
 */
void initialize_buffers(void) {
    curr_line.text = NULL;
    curr_line.length = 0;
    curr_line.max_length = 0;
}
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

/* process_input:
 * Processes the keyboard input and displays it. */
void process_input(char c) {
    if (curr_line.max_length == 0) {
        curr_line.text = calloc(8,sizeof(char));
        curr_line.length = 0;
        curr_line.max_length = 8;
    }
    
    unsigned int length = curr_line.length;
    unsigned int max_length = curr_line.max_length;

    /* check if the buffer is big enough and expand if not.*/

    if (length < max_length-1) { /* null terminated string! */
        curr_line.text[length] = c;
    } else {
        char* new_text = calloc(2*max_length,sizeof(char));
        memcpy(new_text, curr_line.text, length);
        free(curr_line.text);
        curr_line.text = new_text;
        curr_line.max_length = 2*max_length;
        curr_line.text[length] = c;
    }

    curr_line.length++;

    write_xy(0,LINES-1,curr_line.text,1);
    return;
};

/* main:
 * The main function parses command line arguments and creates new threads to
 * run all the necessary routines.
 */
int main(int argc, char *argv[]) {

    /* initialization */
    
    initialize_buffers();

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
    
    if((pthread_detach(network_thread))==-1) /* let it clean itself up. */
        printf("error detatching thread.\n");

    initialize_display();

    while(1){ /* ...and 100% CPU usage */
        int c = getch();  /* refresh, accept single keystroke of input. */
        if (c == 13){
            /* enter key */
        } else if (c == 127){
            /* backspace */
        } else if (c == 27){
            /* esc key */
            if(quit_dialogue())
                break;
        } else if (c > 0) {
            /* any other key */
            process_input(c);
        }
    }
    
    cleanup_display();
    return 0;
}

