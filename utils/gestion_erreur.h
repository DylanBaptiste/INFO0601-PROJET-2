#include "config.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

void gerer_erreur(int cond, const char* custom_message, const char* file, const unsigned long line);