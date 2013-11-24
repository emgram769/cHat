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
#include "buffers.h"
#include "util.h"

#define DEFAULT_PORT    1337
#define BUF_MAX         1024

line_buffer curr_line;
line_buffer_list line_list;

/* private functions */
void send_input(void);
void process_input(char c);
void initialize_buffers(void);
void initialize_video(void);
void print_usage(void);
void delete_char(void);

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
                printf("\n");
                break;
        }
    }

    /* if they didn't specify a peer, ask for it. */
    if (settings->ip_address == NULL) {
        /* max 20 char ip address. */
        char *ip = calloc(20,sizeof(char));
        
        /* prompt user and get their input. */
        printf("Please enter a peer IP address: ");
        fgets(ip, sizeof(ip), stdin);

        /* set the settings with that input */
        settings->ip_address=ip; 
    }
    
    /* spawn network thread. */
    pthread_t network_thread;

    if((pthread_create(&network_thread, NULL,
        (void *)initialize_network, (void *)settings))==-1)
        error_handler("error initializing network thread.\n");
    
    if((pthread_detach(network_thread))==-1) /* let it clean itself up. */
        error_handler("error detatching thread.\n");

    initialize_display();

    while(1){ /* ...and 100% CPU usage */
        int c = getch();  /* refresh, accept single keystroke of input. */
        if (c == 13){
            /* enter key */
            send_input();
        } else if (c == 127){
            /* backspace */
            delete_char();
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

/* initialize_buffers:
 * Initializes the text buffers used by the client.
 */
void initialize_buffers(void) {
    //curr_line = calloc(1,sizeof(line_buffer));
    curr_line.text = NULL;
    curr_line.length = 0;
    curr_line.max_length = 0;

    line_list.head = NULL;
    line_list.curr = NULL;
    line_list.tail = NULL;
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

/* send_input:
 * Sends the current line buffer
 */
void send_input(void){
    if (curr_line.length == 0) /* null input, can't mess with it */
        return;

    //send_msg(curr_line.text,curr_line.length); 
    push_to_line_list(curr_line);

    free(curr_line.text);
    curr_line.text = calloc(8,sizeof(char));
    curr_line.length = 0;
    curr_line.max_length = 8;
 
    //process_input('\0'); /* hacky? resets the string */

    /* refresh display. */ 
    display();
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
    
    if (c == '\0') /* this doesn't add to the string. */
        return;

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

    /* refresh window. */
    display();
    return;
};

/* delete_char:
 * Deletes the most recent character typed (called on backspace)
 */
void delete_char(void) {
    if (curr_line.length == 0)
        return;

    curr_line.text[--curr_line.length] = 0; /* deletion process. */

    /* maybe we should shrink the line! */
    if (curr_line.length < curr_line.max_length/4) {
        char *new_text = calloc(curr_line.max_length/4, sizeof(char));
        memcpy(new_text, curr_line.text, curr_line.length);
        free(curr_line.text);
        curr_line.text = new_text;
    }

    /* refresh window. */
    display();
    return;
}


