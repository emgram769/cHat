#include "ui.h"

/* draws a char to the window passed in at point (x,y)
 * range is (0,0) -> (COLS-1,LINES-1)
 * if the last parameter is 1, do a screen update on call
 */
void draw_xy(WINDOW* window, int x, int y, char c, int update){
    clear();
    
    move(y,x);
    addch(c);
    
    touchwin(window);
    wnoutrefresh(window);
    
    if(update)
        doupdate();
    
}