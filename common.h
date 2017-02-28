/**
 * This file contains all the header files for the program.
 */
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
#define ARR_SIZE 20
#define TRUE 1
#define FALSE 0
#define PROFILE "Shell_Profile"

#define log_err(M, ...) fprintf(stderr, " [ERROR] [%s]: [%s]: (%d): errno: %s " M "\n", __FILE__, __func__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_info(M, ...) fprintf(stderr,M "\n",##__VA_ARGS__)

#define PIPE_READ 0
#define PIPE_WRITE 1


typedef struct _command_
{
    char input[SIZE];
    int level;
}COMMAND_ARRAY;


jmp_buf jmpBuf;
COMMAND_ARRAY cmdArray[ARR_SIZE];
int valid_oldpwd;
char oldpwd[255];
int alarmEnabled;

void ctrl_C_Handler();
void alarm_Handler();
char* duplicate(char *command);
char* readProfile(char *type);
void signalhandler(int signo);
void initializeCommandArray(COMMAND_ARRAY *cmdArray);
int parseToken(char *buf,COMMAND_ARRAY *cmdArray, int clear);
int Execute(char *buf, char *delimiter);
void handleBraceAndReturnString(int retval, char* finalString[1024], char* temp);
int isValidExpression(char *input);
