/* display.h
 */

#ifndef _CHAT_DISPLAY_H_
#define _CHAT_DISPLAY_H_

void initialize_display(void);

void display(void);

void clear_display(void);

void write_xy(int x, int y, char* msg, int update);

void draw_xy(int x, int y, char c, int update);

int quit_dialogue(void);

void popup_dialogue(char *msg);

void cleanup_display(void);

#endif /* _CHAT_DISPLAY_H_ */

