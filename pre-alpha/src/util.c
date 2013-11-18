/* util.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void error_handler(char *err){
    perror(err);
    exit(1);
}
