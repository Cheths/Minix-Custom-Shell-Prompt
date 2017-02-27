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

	struct sigaction new_act,old_act;
	do{
		//log_info("Function:%s:  Entry", __FUNCTION__);

		if(readProfile("prompt") == NULL){
			promptsign = "$";
            home = getcwd(NULL,0);
		} else {
			promptsign = readProfile("prompt");
			home = readProfile("home");
		}

		new_act.sa_handler = SIG_IGN;
		retval = sigaction(SIGINT, &new_act, &old_act);
		if(retval < 0){
			log_info("retval[%d]",retval);
			break;
		}

		setjmp(sjbuf);
		if (old_act.sa_handler != SIG_IGN)
        {
            new_act.sa_handler = signalhandler;
            retval=sigaction(SIGINT, &new_act, &old_act);
            if(retval<0)
            {
                log_info("retval[%d]",retval);
                break;
            }
        }

		initializeCommandArray(cArray);
		chdir(home);
		while(TRUE){
			char cwd[255];

			getcwd(cwd,sizeof(cwd));
			printf("%s$:",cwd);

			if(!fgets(buf,100,stdin)){
				return 0;
			}
			if(buf != NULL){
				char *temp =buf;

                char finalString[1024];
                char *normal;
                normal = buf;
                memset(finalString,0,1024);

                if(IsValidExpression(temp) == 0){
                	printf("Invalid Input - Paranthesis is not proper\n");

                }
                else{

				if(buf[0] == '('){
					handleBraceAndReturnString(retval, &finalString, temp);
					debug("Final string %s", finalString) debug("Final string %s", finalString) /* Execute command */
                    preProcessCommand(finalString);
				} else {
					normal = strtok(normal,"\n");
					retval = preProcessCommand(normal);
				}
                }
			}

		}

	}while(0);

	//log_info("Function:%s Exit", __FUNCTION__);
	return 0;
}
