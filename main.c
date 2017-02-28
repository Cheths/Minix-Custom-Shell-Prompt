/**
 * To Build a Custom shell prompt.
 * This file contains the main function for the program.
 */
#define _POSIX_SOURCE

#include "common.h"
/*
 * main() function to display Custom Shell Prompt and process user inputs in the prompt.
 */
int main(){

	signal(SIGALRM, alarm_Handler);

	char *home;
    char *promptsign;
	char buf[SIZE];
	int retval = 0;

	struct sigaction newAction, oldAction;
	do{
		//log_info("Shell start PID:%d", getpid());

		if(readProfile("prompt") == NULL){
			promptsign = "$";
            home = getcwd(NULL,0);
		} else {
			promptsign = readProfile("prompt");
			home = readProfile("home");
		}

		newAction.sa_handler = SIG_IGN;
		retval = sigaction(SIGINT, &newAction, &oldAction);
		if(retval < 0){
			break;
		}

		setjmp(jmpBuf);
		if (oldAction.sa_handler != SIG_IGN) {
            newAction.sa_handler = signalhandler;
            retval = sigaction(SIGINT, &newAction, &oldAction);
            if(retval < 0) {
                break;
            }
        }

		initializeCommandArray(cmdArray);
		chdir(home);
		while(TRUE){
			char cwd[255];

			getcwd(cwd,sizeof(cwd));
			printf("%s$:",cwd);

			if(!fgets(buf,100,stdin)){
				return 0;
			}
			if(buf != NULL && (strlen(buf) > 1 || !strchr(buf, '\n'))){ // if command is not just enter key and is not null
				char *temp =buf;
				char finalString[1024];
				char *normal;
				normal = buf;
				memset(finalString,0,1024);

				if(isValidExpression(temp) == 0){
					printf("Invalid Input - Paranthesis is not proper\n");
				} else {
					if (buf[0] == '(') {
						handleBraceAndReturnString(retval, &finalString, temp);
						preProcessCommand(finalString);
					} else {
						normal = strtok(normal, "\n");
						retval = preProcessCommand(normal);
					}
				}
			}
		}

	}while(0);

	//log_info("Function:%s Exit", __FUNCTION__);
	return 0;
}
