#define _POSIX_SOURCE

#include "common.h"

int main(){
	
	//signal(SIGINT, ctrlC_Handler);
	
	char *home;
    char *promptsign;
	char buf[SIZE];
	
	do{
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
			printf("%s%s",cwd,promptsign);
			
			if(!fgets(buf,100,stdin)){
				return 0;
			}
		}
	}while(0);
	
	return 0;
}