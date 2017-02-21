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
			getcwd(cwd,sizeof(cwd));
			printf("%s$:",cwd);
			
			if(!fgets(buf,100,stdin)){
				return 0;
			}
			if(buf != NULL){
				char *temp = buf;
				char *normal;
				normal = buf;

				if(buf[0] == '('){

				} else {
					retval = Execute(normal);
				}
			}

		}

	}while(0);
	
	//log_info("Function:%s Exit", __FUNCTION__);
	return 0;
}
