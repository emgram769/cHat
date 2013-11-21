/* display.h
 */

#ifndef _CHAT_DISPLAY_H_
#define _CHAT_DISPLAY_H_

#include <curses.h> /* weird WINDOW type bullshit. */

void initialize_display(void);

void display(void);

void clear_display(void);

int quit_dialogue(void);

void popup_dialogue(char *msg);

void cleanup_display(void);

#endif /* _CHAT_DISPLAY_H_ */

