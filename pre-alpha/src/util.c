/* util.c
 */

#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "display.h"

void error_handler(char *err){
    perror(err);
    cleanup_display();
    exit(1);
}
