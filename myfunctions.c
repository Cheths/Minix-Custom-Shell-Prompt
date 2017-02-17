
#include "common.h"

void ctrlC_Handler() {
	char choice;
	printf("\nDo you want to exit? Enter y/n \n");
	scanf("%c", &choice);
	log_info("Cmd entered:%c",choice);
	{
		if (choice == 'Y' || choice == 'y') {
			log_info("successful exit");
			exit(1);
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

                while(arg_list[counter2] != NULL)
                {
                    if(strchr(arg_list[counter2],'|'))
                        debug("Pipe command entered");
					
                    if( !strcmp( arg_list[counter2], "|"))
                    {
                        int num_pcmds = 0;
                        int tmp = 0;
                        while (arg_list[tmp] != NULL) {
                            debug("arg_list[%d]: %s", tmp, arg_list[tmp]);
                            if (!strcmp( arg_list[tmp], "|")) {
                                num_pcmds++;
                            }
                            tmp++;
                        } 
                       
                        {
                            int fds[num_pcmds][2];
                            int arg[num_pcmds + 1][2];
                            int cmd = 1; /* atleast 1 command */
                            pid_t pids[num_pcmds + 1];

                            tmp = 0;
                            arg[cmd - 1][0] = 0;

                            /* Mark when each command is starting and ending */
                            while (arg_list[tmp] != NULL) {
                                if (!strcmp( arg_list[tmp], "|")) {
                                    arg[cmd - 1][1] = tmp;
                                    pipe(&fds[cmd - 1][0]);
                                    debug("cmd %d.. fds[%d][0] %d fds[%d][1] %d", 
                                           cmd -1, cmd -1, fds[cmd - 1][0],
                                           cmd -1, fds[cmd - 1][1]);
                                    cmd++;
                                    if (cmd <= num_pcmds + 1) {
                                        arg[cmd - 1][0] = tmp + 1;
                                    }
                                }
                                tmp++;
                            }
                            arg[cmd -1][1] = tmp;

                            for (tmp = 0; tmp <= num_pcmds; tmp++) {
                                debug("num_pcmds %d: arg[%d][0].. %d; arg[%d][1].. %d", 
                                       num_pcmds, tmp, arg[tmp][0], tmp, arg[tmp][1]);
                            }

                            for (tmp = 0; tmp <= num_pcmds; tmp++) {

                                int k;
                                pids[tmp] = fork();
    
                                if (pids[tmp] == 0) { // child

                                    if (tmp == 0) {
                                        dup2(fds[tmp][PIPE_WRITE], STDOUT_FILENO);
                                    } else if (tmp == num_pcmds) {
                                        dup2(fds[tmp - 1][PIPE_READ], STDIN_FILENO );
                                    } else {
                                        dup2(fds[tmp - 1][PIPE_READ], STDIN_FILENO );
                                        dup2(fds[tmp][PIPE_WRITE], STDOUT_FILENO);
                                    }

                                    arg_list[arg[tmp][1]] = 0;

                                    for (k = 0; k < num_pcmds; k++) {
                                        debug("k %d: 0 %d, 1 %d", k, fds[k][0],
                                                fds[k][1]);
                                        close(fds[k][0]);
                                        close(fds[k][1]);
                                    }

                                    for (k = arg[tmp][0]; k <= arg[tmp][1]; k++)
                                    {
                                        debug("tmp %d: arg_list[%d] %s", tmp, k,
                                                arg_list[k]);
                                    }

                                    /* execute the command */
                                    execvp(arg_list[arg[tmp][0]], &arg_list[arg[tmp][0]]);
                                    exit(0);
                                }
                            }
                            for (tmp = 0; tmp < num_pcmds; tmp++) {
                                close(fds[tmp][0]);
                                close(fds[tmp][1]);
                            }
                            for (tmp = 0; tmp <= num_pcmds; tmp++) {
                                waitpid(pids[tmp], NULL, 0);
                            }
                        } 
                        /* All is done */
                        isrun=1;
                        break;
                    }
                    counter2++;
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
            }
        }
    }while(0);

    return retval;
}
