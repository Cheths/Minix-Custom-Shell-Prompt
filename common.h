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

#define SIZE 1024
#define CARRAY_SIZE 20
#define TRUE 1
#define FALSE 0
#define PROFILE "Shell_Profile"

#define log_err(M, ...) fprintf(stderr, " [ERROR] [%s]: [%s]: (%d): errno: %s " M "\n", __FILE__, __func__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_info(M, ...) fprintf(stderr,M "\n",##__VA_ARGS__)
#define debug(M, ...)

#define PIPE_READ 0
#define PIPE_WRITE 1


typedef struct _command_
{
    char input[SIZE];
    int level;
}COMMAND_ARRAY;


jmp_buf sjbuf;
COMMAND_ARRAY cArray[CARRAY_SIZE];
int valid_oldpwd;
char oldpwd[255];
int alarmEnabled;

void ctrlC_Handler();
void alarm_Handler();
char* duplicate(char *command);
char* readProfile(char *type);
void signalhandler(int signo);
void initializeCommandArray(COMMAND_ARRAY *cArray);
int parseToken(char *buf,COMMAND_ARRAY *cArray, int clear);
int Execute(char *buf, char *delimiter);
void handleBraceAndReturnString(int retval, char* finalString[1024], char* temp);
int IsValidExpression(char *input);
