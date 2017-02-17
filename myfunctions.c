
#include "common.h"

void ctrlC_Handler() {
	char choice;
	printf("\nDo you want to exit? Enter y/n \n");
	scanf("%c", &choice);
	{
		if (choice == 'Y' || choice == 'y') {
			exit(1);
		}
	}
}

char* duplicate(char *command){
    int len;
    char *hs;
	//log_info("Inside duplicate::%s",command);
    if(command == NULL)
    {
        return NULL;
    }

    len=strlen(command)+1;//+1 for the null character
    hs=(char *)malloc(len*sizeof(char));

    if (hs == NULL) {
       log_info("Malloc failed");
        return NULL;
    } else {
        strcpy(hs,command);
		//log_info("Exiting duplicate::%s",command);
        return hs;
    }	
}

char* readProfile(char *type)
{
    FILE *fp;
    char *home,*promptsign, *temp,*temp1,* pos;
    char command[20];
    int len;

    do
    {
        fp=fopen(PROFILE,"r");
        if(fp==NULL)
        {
            log_info("Cannot open PROFILE file");
            return NULL;
        }

        while(fgets(command, sizeof(command), fp)!=NULL)
        {
            temp=strtok(command,"=");

            if(strchr(temp,'#')) {
                continue;
            }

            if(!strcmp(temp,"prompt")){
                temp1=strtok(NULL,"=");
                promptsign=duplicate(temp1);
                if (!promptsign) {
                    return NULL;
                }
                len=strlen(promptsign);
                pos=promptsign+len-1;
                *pos='\0';
            } else if(!strcmp(temp,"path")) {
                char *path;
                temp1=strtok(NULL,"=");
                pos=temp1+strlen(temp1)-1;
                *pos='\0';
                setenv("PATH", temp1, 1);
            } else if(!strcmp(temp,"home")) {
                temp1=strtok(NULL,"=");
                home=duplicate(temp1);
                if (!home) {
                    return NULL;
                }
                len=strlen(home);
                pos=home+len-1;
                *pos='\0';
            }
        }

        fclose(fp);

        if(!strcmp(type,"home"))
            return home;
        else if (!strcmp(type,"prompt"))
            return promptsign;
        else
            return NULL;
    } while(0);
}