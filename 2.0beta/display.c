/* display.c
 * The file containing all the display logic.
 */

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
    nodelay(main_window, TRUE);  /* refresh. */
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

