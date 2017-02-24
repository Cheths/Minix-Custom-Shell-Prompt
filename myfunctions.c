#include "common.h"

void ctrlC_Handler() {
	char choice;
	printf("\nDo you want to exit? Enter y/n \n");
	scanf("%c", &choice);
	log_info("Cmd entered:%c",choice);
	{
		if (choice == 'Y' || choice == 'y') {
			log_info("successful exit");
			exit(0);
		}
	}
}

void alarm_Handler() {
	char choice;
	printf("\nDo you want to exit? Enter y/n \n");
	scanf("%c", &choice);
	log_info("Cmd entered:%c",choice);
	{
		if (choice == 'Y' || choice == 'y') {
			log_info("successful exit");
			exit(0);
		}
		else
		{
			longjmp(sjbuf,1);
		}
	}
}

void signalhandler(int signo)
{
    switch(signo)
    {
        case SIGINT:
        {
            char str;
            printf("\nAre you sure?[Y/N]");
            str = getchar();
            if(str=='y' || str=='Y')
                exit(0);
            else
                longjmp(sjbuf,1);
            break;
        }
        case SIGCHLD:
        {
            int pid;
            pid=wait(NULL);
            printf("PID:%d completed job.\n",pid);
            longjmp(sjbuf,1);
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

void initializeCommandArray(COMMAND_ARRAY *cArray)
{
    int i=0;

    if (cArray==NULL)
    {
        return;
    }

    for(i=0;i<CARRAY_SIZE;i++)
    {
        memset(&cArray[i].input,0,sizeof(SIZE));
        cArray[i].level = -1;
    }
}

int Execute(char *buf)
{

    char *arg_list[30] = {0};
    int status;
    int counter = 0;
    int counter2 = 0;
    int isrun =0;
    int pid    =-1;
    int retval  = 1;

    do
    {
        if(buf)
        {
            char first_argument[64] = {0};

            if( strchr(buf,'&') || strchr(buf,'<') || strchr(buf,'>'))
            {
                printf("Command with &,< and > is not supported\n");
                break;
            }

            arg_list[counter] = strtok(buf, " \n");

            if( arg_list[counter] != NULL )
            {
                strcpy( first_argument, arg_list[0]);

                while(arg_list[counter] != NULL)
                {
                    debug("%s***",arg_list[counter]);
                    counter++;
                    arg_list[counter] = strtok(NULL, " \n");
                }

                /* Handle cd command */
                if( strcmp( first_argument, "cd" ) == 0 )
                {
                    char cwd[255];
                    debug("Cd command ");

                    if( arg_list[1] != NULL )
                    {
                        /* If command is cd -; go back to old directory if
                         * exists */
                        if( strcmp(arg_list[1], "-") == 0 )
                        {
                            if (valid_oldpwd) 
                            {
                                char cwd[255];
                                getcwd(cwd, sizeof(cwd));
                                chdir( oldpwd );
                                stpncpy(oldpwd, cwd, sizeof(oldpwd));
                                debug("Oldpwd %s.. ", oldpwd);
                            }
                        }
                        else
                        {
                            /* cache current directory value before changing the
                             * directory */
                            valid_oldpwd = 1;
                            stpncpy(oldpwd, getcwd(NULL, 0), sizeof(oldpwd));
                            debug("Oldpwd %s.. %s", oldpwd, getcwd(NULL, 0));
                            chdir( arg_list[1] );
                        }
                    }
                    return retval;
                }
                /* Handle 'exit' or 'quit' */
                else if( (strcmp( first_argument, "exit" ) == 0) || 
                         (strcmp( first_argument, "quit" ) == 0) )
                {
                    log_info("Shell Exit: pid %d", getpid());
                    exit(0);
                }


                /* Handle normal command.. No pipe */
                if (arg_list[0] !=NULL && isrun==0)
                {
                    debug("first_argument %s", first_argument);
                    pid = fork();
                    if(pid <0) return 1;
                    else if(pid ==0)
                    {
                        execvp( first_argument, arg_list );
                        exit(0);
                    }
                    else
                    {
                        waitpid( pid, &status, 0 );
                    }
                }
                alarm(5);
            }
        }
        
        
    }while(0);

    return retval;
}

int parseToken(char *buf,COMMAND_ARRAY *cArray, int clear)
{
    int retval=1;
    int length=0;
    int index=0,count=0;
    static int slevel=0;
    static int array_count;

    if(clear)
    {
        slevel = 0;
        array_count = 0;
    }

    if(buf==NULL)
    {
        return 0;
    }

    do
    {
        length = strlen(buf);
        debug("length is %d",length);

        while(length)
        {
            char *c =  (char *)&buf[index];

            /* raise our level if/when '(' is encountered in input */
            if(*c=='(')
            {
                ++slevel;
            }

            /* lower our level if/when '(' is encountered in input */
            if(*c==')')
            {
                --slevel;
            }

            if(isalpha(buf[index]) || isspace(buf[index]) || (*c=='-'))
            {
                cArray[array_count].level = slevel;
                cArray[array_count].input[count] =  buf[index];
                count++;
            }
            index++;
            length--;
        }
        cArray[array_count].input[count] = '\0';
        array_count++;
    }while(0);

    debug("EXIT");
    return retval;
}
