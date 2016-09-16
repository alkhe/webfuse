#include <stdio.h>
#include <stdlib.h>

#include "die.h"

void die(const char* msg) {
	perror(msg);
	exit(0);
}
