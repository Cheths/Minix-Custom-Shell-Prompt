#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>

#include <malloc.h>

#define PROFILE "Shell_Profile"
#define SIZE 1024

void ctrlC_Handler();
char* duplicate(char *command);
char* readProfile(char *type);