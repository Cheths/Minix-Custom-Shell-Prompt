#define _POSIX_SOURCE

#include "common.h"

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

			//log_info("count %d",count);
			getcwd(cwd,sizeof(cwd));

			//log_info("cwd %s",cwd);

			printf("%s$:",cwd);

			if(!fgets(buf,100,stdin)){
				return 0;
			}
			if(buf != NULL){
				char *temp =buf;
                char *token;
                char finalString[1024];
                char *normal;
                normal = buf;
                memset(finalString,0,1024);

				if(buf[0] == '('){
					int finalcount=0;

                    /* Tokenize user input and parse it */
                    token = strtok(temp,",\n");
                    retval = parseToken(token,cArray,1);
                    while(token!=NULL)
                    {
                        token= strtok(NULL,",\n");
                        if(token!=NULL)
                            retval = parseToken(token,cArray,0);
                    }

                    /*
                     * execute the command based on their level, as righmost input in
                     * '()' has higher priority.
                     */
                    retval=0;//sortLevel(cArray);

                    debug("Final string %s", finalString);
                    while(cArray[finalcount].level != -1)
                    {
                        strcat(finalString,cArray[finalcount].input);

                        /* If their are multiple commands then insert pipe in
                         * between */
                        if(finalcount < CARRAY_SIZE && cArray[finalcount+1].level != -1)
                            strcat(finalString," | ");
                        finalcount++;
                    }
                    debug("Final string %s", finalString);
                    /* Execute command */
                    Execute(finalString);

				} else {
					retval = Execute(normal);
				}
			}

		}

	}while(0);
	
	//log_info("Function:%s Exit", __FUNCTION__);
	return 0;
}
