/* display.c
 * The file containing all the display logic.
 */

#include <stdlib.h>  /* exit */ 
#include <curses.h>

#include "buffers.h"
#include "display.h"

/* The ncurses window we will be displaying to. */
static WINDOW *main_window;
static WINDOW *chat_window;
static WINDOW *input_window;

/* The structures we display are stored in cHat.c */
extern line_buffer curr_line;
extern line_buffer_list line_list;

/* local functions. */
void write_xy(WINDOW* window, int x, int y, char* msg, int update);
void draw_xy(WINDOW* window, int x, int y, char c, int update);

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

    chat_window = subwin(main_window, LINES-5, COLS, 0, 0);
    //chat_window = subwin(main_window, 10, COLS, 0, 0);
    input_window = subwin(main_window, 2, COLS, LINES-2, 0);
    
    start_color();

    init_pair(1,COLOR_WHITE, COLOR_BLUE);
    
    wbkgd(input_window, COLOR_PAIR(1));

    return;
}

/* display:
 * Refresh the display.
 * parameter describes update
 * 0 is all, 1 is input, 2 is chat lines
 */
void display(int i) {
    if(i==0)
        clear();

    /* draw the screen here. */
    
    /* draw the chat window */
    if(i==0 || i==2) {    
        line_buffer_node *curr_node = line_list.curr;
        int counter = 0;

        while(curr_node!=NULL) {
            write_xy(chat_window, 0, LINES-10-counter, curr_node->line.text,0);
            curr_node = curr_node->prev;
            counter++;
        }
        
    }

    if(i==0||i==1) {
        /* print char count, useful for debugging. */
        char *len = calloc(10,sizeof(char));
        sprintf(len, "%d", curr_line.length);
        write_xy(input_window, COLS-3, 1, len,0);
        free(len);

        write_xy(input_window, 0, 0, curr_line.text,0); /* current typing drawn last. */
    }
   
    wnoutrefresh(input_window);
    wnoutrefresh(chat_window);
    wnoutrefresh(main_window);
    doupdate();

    return;
}

/* clear_display:
 * clears the display.
 */
void clear_display(void) {
    clear();
    return;
}

/* write_xy:
 * writes a msg to the window passed in at point (x,y)
 * if the last parameter is 1, do a screen update on call.
 */
void write_xy(WINDOW* window, int x, int y, char *msg, int update){
    mvwprintw(window, y, x, msg);

    touchwin(window);
    wnoutrefresh(window);
    
    if(update)
        doupdate();
}


/* draw_xy:
 * draws a char to the window passed in at point (x,y).
 * range is (0,0) -> (COLS-1,LINES-1).
 * if the last parameter is 1, do a screen update on call.
 */
void draw_xy(WINDOW* window, int x, int y, char c, int update){
    move(y,x);
    addch(c);
    
    touchwin(window);
    wnoutrefresh(window);
    
    if(update)
        doupdate();
}

/* quit_dialogue:
 * Prompts the user to quit the program.
 * returns 1 if user quits
 */
int quit_dialogue(void) {
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
        return 1;
    } else {
        delwin(quit_window);
        display(0);
        return 0;
    }

}

/* popup_dialogue:
 * Prompts the user with popup. esc to exit
 */
void popup_dialogue(char *msg) {
    clear();
    int height, width;
    height = 6;
    width = 20;

    /* create a local window and display it. */
    WINDOW *popup_window;
    popup_window = subwin(main_window, height, width,
                    (LINES-height)/2, (COLS-width)/2);
    mvwprintw(popup_window, 1, 1, /* padding for border. */ 
                    msg);
    mvwprintw(popup_window, height-2, 1,
                    "(esc to close)");
    box(popup_window, 0, 0); /* adds a border. */
    
    wrefresh(main_window);
    
    getch(); /* weird that I need this here */
    int c = getch();
    if(c==27){
        delwin(popup_window);
        display(0);
    } else {
        delwin(popup_window);
        display(0);
    }

}




/* cleanup_display:
 * Cleans up the display.  Should be called before exit.
 */
void cleanup_display(void) {
    /* We ignore the return value of endwin, as there is nothing we can do if
     * it does return an error.
     */
    if (main_window != NULL)
        (void) endwin();
    return;
}

