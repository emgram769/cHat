/* display.h
 */

#ifndef _CHAT_DISPLAY_H_
#define _CHAT_DISPLAY_H_

#include <pthread.h>

void initialize_display(void);

void display(int i); /* takes a parameter that describes update */

void clear_display(void);

int quit_dialogue(void);

void popup_dialogue(char *msg);

void cleanup_display(void);

pthread_mutex_t display_mutex;

#endif /* _CHAT_DISPLAY_H_ */

