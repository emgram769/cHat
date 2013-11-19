/* util.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "util.h"
#include "display.h"


void error_handler(char *err){
    perror(err);
    cleanup_display();
    exit(1);
}

int is_valid_fd(int fd){
    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}
