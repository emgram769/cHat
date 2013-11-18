/* cHat.c
 * The main file for the cHat chat client.
 */

#include <stdio.h> /* printf */
#include <stdlib.h> /* atoi */
#include <getopt.h>

#include "ui.h"
#include "network.h"
#include "video.h"
#include "audio.h"
#include "crypto.h"

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

#define DEFAULT_PORT 1337

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

    WINDOW *main_window;
    if ((main_window = initscr()) == NULL){
        fprintf(stderr, "ncurses failed.\n");
        exit(EXIT_FAILURE);
    }

    keypad(main_window, TRUE);  /* keyboard mapping. */
    //nodelay(main_window, TRUE);  /* refresh. */
    (void) nonl();  /* tell curses not to do NL->CR/NL on output. */
    (void) cbreak();  /* don't wait for line break. */
    (void) noecho();  /* don't print getch to stdout. */

    while(1){ /* ...and 100% CPU usage */
        int c = getch();  /* refresh, accept single keystroke of input. */
        if (c==13){
            /* enter key */
            draw_xy(main_window,COLS/2,LINES-1,'s',1);
        } else if (c==127){
            /* backspace */
        } else if (c>0) {
            /* any other key */
        }
    }

    return 0;
}







