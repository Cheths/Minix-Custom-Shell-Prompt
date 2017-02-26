#define _POSIX_SOURCE

#include "common.h"

/*void handleBraceAndReturnString(int retval, char* finalString[1024], char* temp) {
	int finalcount = 0;
	char *token;
	 Tokenize user input and parse it
	token = strtok(temp, ",\n");
	retval = parseToken(token, cArray, 1);
	while (token != NULL) {
		token = strtok(NULL, ",\n");
		if (token != NULL)
			retval = parseToken(token, cArray, 0);
	}

	 * execute the command based on their level, as rightmost input in
	 * '()' has higher priority.

	retval = 0;
	;
	while (cArray[finalcount].level != -1) {
		strcat(finalString, cArray[finalcount].input);
		finalcount++;
	};
	//return finalString;
}*/

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
