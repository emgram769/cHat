/* display.c
 * The file containing all the display logic.
 */
#include "display.h"

#include <stdlib.h>  /* exit */ 
#include <curses.h>

/* The ncurses window we will be displaying to. */
static WINDOW *main_window;

/* initalize_display:
 * This will setup the display for writing to.
 */
void initialize_display(void) {
    if ((main_window = initscr()) == NULL){
        fprintf(stderr, "ncurses failed.\n");
        exit(EXIT_FAILURE);
    }

    keypad(main_window, TRUE);  /* keyboard mapping. */
    //nodelay(main_window, TRUE);  /* BAD NEWS for CPU, may want to handle this differently */
    (void) nonl();  /* tell curses not to do NL->CR/NL on output. */
    (void) cbreak();  /* don't wait for line break. */
    (void) noecho();  /* don't print getch to stdout. */

    return;
}

/* display:
 * Refresh the display.
 */
void display(void) {
    return;
}

/* draw_xy:
 * draws a char to the window passed in at point (x,y).
 * range is (0,0) -> (COLS-1,LINES-1).
 * if the last parameter is 1, do a screen update on call.
 */
void draw_xy(int x, int y, char c, int update){
    clear();
    
    move(y,x);
    addch(c);
    
    touchwin(main_window);
    wnoutrefresh(main_window);
    
    if(update)
        doupdate();
}

/* quit_dialogue:
 * Prompts the user to quit the program.
 */
void quit_dialogue(void) {
    clear();
    int height, width;
    height = 6;
    width = 20;

    /* create a local window and display it. */
    WINDOW *quit_window;
    quit_window = subwin(main_window, height, width,
                    (LINES-height)/2, (COLS-width)/2);
    mvwprintw(quit_window, 1, 1, /* padding for border. */ 
                    "are you sure\n you want to quit?");
    mvwprintw(quit_window, height-2, 1,
                    "press q");
    box(quit_window, 0, 0); /* adds a border. */
    
    wrefresh(main_window);
    
    getch(); /* weird that I need this here */
    int c = getch();
    if(c=='q'||c=='Q'){
        cleanup_display();
        exit(0);
    }

}

/* cleanup_display:
 * Cleans up the display.  Should be called before exit.
 */
void cleanup_display(void) {
    /* We ignore the return value of endwin, as there is nothing we can do if
     * it does return an error.
     */
    (void) endwin();
    return;
}

