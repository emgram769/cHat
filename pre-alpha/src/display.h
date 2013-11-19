/* display.h
 */

#ifndef _CHAT_DISPLAY_H_
#define _CHAT_DISPLAY_H_

void initialize_display(void);

void display(void);

void write_xy(int x, int y, char* msg, int update);

void draw_xy(int x, int y, char c, int update);

void quit_dialogue(void);

void cleanup_display(void);

#endif /* _CHAT_DISPLAY_H_ */

