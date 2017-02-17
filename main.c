#define _POSIX_SOURCE

#include "common.h"

int main(){
	
	signal(SIGINT, ctrlC_Handler);
	
	char *home;
    char *promptsign;
	char buf[SIZE];
	
	do{
		//log_info("Function:%s:  Entry", __FUNCTION__);

		if(readProfile("prompt") == NULL){
			promptsign = "$";
            home = getcwd(NULL,0);
		} else {
			promptsign = readProfile("prompt");
			home = readProfile("home");
		}
		chdir(home);
		
		while(1){
			char cwd[255];
			getcwd(cwd,sizeof(cwd));
			printf("%s$:",cwd);
			
			if(!fgets(buf,100,stdin)){
				return 0;
			}
		}
	}while(0);
	
	//log_info("Function:%s Exit", __FUNCTION__);
	return 0;
}