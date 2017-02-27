
/**
 * This file supports all the basic functions for Minix shell prompt.
 */
#include "common.h"

/*
 * Function to check whether source string starts with a particular character.
 */
bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

/*
 * Function to handle 'Ctrl+C' keystroke.
 */
void ctrlC_Handler() {
	char choice;
	printf("\nDo you want to exit? Enter y/n \n");
	scanf("%c", &choice);
	log_info("Cmd entered:%c", choice);
	{
		if (choice == 'Y' || choice == 'y') {
			log_info("successful exit");
			exit(0);
		}
	}
}

/*
 * Function to show the alarm after 5 seconds of user command input.
 */
void alarm_Handler() {
	char choice;
	printf("\nDo you want to exit? Enter y/n \n");
	scanf("%c", &choice);
	log_info("Cmd entered:%c", choice);
	{
		if (choice == 'Y' || choice == 'y') {
			log_info("successful exit");
			exit(0);
		} else {
			longjmp(sjbuf, 1);
		}
	}
}

/*
 * Function to handle signal interrupts.
 */
void signalhandler(int signo) {
	switch (signo) {
	case SIGINT: {
		char str;
		printf("\nAre you sure?[Y/N]");
		str = getchar();
		if (str == 'y' || str == 'Y')
			exit(0);
		else
			longjmp(sjbuf, 1);
		break;
	}
	case SIGCHLD: {
		int pid;
		pid = wait(NULL);
		printf("PID:%d completed job.\n", pid);
		longjmp(sjbuf, 1);
	}
	}
}

/*
 * Function to get a clone of the input command.
 */
char* duplicate(char *command) {
	int len;
	char *hs;
	if (command == NULL) {
		return NULL;
	}

	len = strlen(command) + 1; //+1 for the null character
	hs = (char *) malloc(len * sizeof(char));

	if (hs == NULL) {
		log_info("Malloc failed");
		return NULL;
	} else {
		strcpy(hs, command);
		return hs;
	}
}

/*
 * Function to read profile from the "Shell_Profile" config file.
 */
char* readProfile(char *type) {
	FILE *fp;
	char *home, *promptsign, *temp, *temp1, *pos;
	char command[20];
	int len;

	do {
		fp = fopen(PROFILE, "r");
		if (fp == NULL) {
			log_info("Cannot open PROFILE file");
			return NULL;
		}

		while (fgets(command, sizeof(command), fp) != NULL) {
			temp = strtok(command, "=");

			if (strchr(temp, '#')) {
				continue;
			}

			if (!strcmp(temp, "prompt")) {
				temp1 = strtok(NULL, "=");
				promptsign = duplicate(temp1);
				if (!promptsign) {
					return NULL;
				}
				len = strlen(promptsign);
				pos = promptsign + len - 1;
				*pos = '\0';
			} else if (!strcmp(temp, "path")) {
				char *path;
				temp1 = strtok(NULL, "=");
				pos = temp1 + strlen(temp1) - 1;
				*pos = '\0';
				setenv("PATH", temp1, 1);
			} else if (!strcmp(temp, "home")) {
				temp1 = strtok(NULL, "=");
				home = duplicate(temp1);
				if (!home) {
					return NULL;
				}
				len = strlen(home);
				pos = home + len - 1;
				*pos = '\0';
			} else if (!strcmp(temp, "alarmEnabled")) {
				temp1 = strtok(NULL, "=");
				if (!strcmp(temp1, "true")) {
					alarmEnabled = 1;
				} else {
					alarmEnabled = 0;
				}
			}
		}

		fclose(fp);

		if (!strcmp(type, "home"))
			return home;
		else if (!strcmp(type, "prompt"))
			return promptsign;
		else
			return NULL;
	} while (0);
}

/*
 * Function to initialize command array.
 */
void initializeCommandArray(COMMAND_ARRAY *cArray) {
	int i = 0;

	if (cArray == NULL) {
		return;
	}

	for (i = 0; i < CARRAY_SIZE; i++) {
		memset(&cArray[i].input, 0, sizeof(SIZE));
		cArray[i].level = -1;
	}
}

/*
 * Function to handle braces and return the modified string.
 */
void handleBraceAndReturnString(int retval, char* finalString[1024], char* temp) {
	int finalcount = 0;
	char *token;
	/* Tokenize user input and parse it */
	token = strtok(temp, ",\n");
	retval = parseToken(token, cArray, 1);
	while (token != NULL) {
		token = strtok(NULL, ",\n");
		if (token != NULL)
			retval = parseToken(token, cArray, 0);
	}
	
	retval = 0;
	while (cArray[finalcount].level != -1) {
		strcat(finalString, cArray[finalcount].input);
		finalcount++;
	};
}

/*
 * Function that processes command execution logic.
 */
int Execute(char *buf, char *delimiter) {

	char *arg_list[30] = { 0 };
	int status;
	int counter = 0;
	int counter2 = 0;
	int isrun = 0;
	int pid = -1;
	int retval = 1;

	do {
		if (buf) {
			char first_argument[64] = { 0 };

			if (strchr(buf, '<') || strchr(buf, '>') || strchr(buf, '|')) {
				printf("Command with < and > is not supported\n");
				break;
			}

			if (delimiter == NULL) {
				arg_list[counter] = buf;
			} else {
				arg_list[counter] = strtok(buf, delimiter);
			}
			if (arg_list[counter] != NULL) {
				strcpy(first_argument, arg_list[0]);
				while (arg_list[counter] != NULL) {
					counter++;
					char *prevCommand = arg_list[counter - 1];
					if (startsWith("(", prevCommand)) {
						
						arg_list[counter - 1] = '\0';
						char stringBraces[1024];
						memset(stringBraces, 0, 1024);
						handleBraceAndReturnString(retval, &stringBraces, prevCommand);
						preProcessCommand(stringBraces);

					} else if(strchr(prevCommand, '&')){						
						strtok(prevCommand, "&");
						arg_list[counter] = strtok(NULL, "&");
					}else if(strchr(prevCommand, ';')){
						strtok(prevCommand, ";");						
						arg_list[counter] = strtok(NULL, ";");
						
					}else {
						if (delimiter != NULL) {
							arg_list[counter] = strtok(NULL, "\0");
						}
					}
				}

				if( strcmp( first_argument, "cd .." ) == 0 ) {
                    char cwd[255];
                    int iterator = 0;

                    arg_list[iterator] = strtok(first_argument, " ");
					
                    if( arg_list[0] != NULL ) {
                    	 while(arg_list[iterator]!=NULL) {
                    		iterator++;
                        	arg_list[iterator]= strtok(NULL,"\n");
                        }
                        /* If command is cd -; go back to old directory if
                         * exists */
                        if( strcmp(arg_list[1], "-") == 0 ) {
                        	log_info("inside if ");
                            if (valid_oldpwd) {
                                char cwd[255];
                                getcwd(cwd, sizeof(cwd));
                                chdir( oldpwd );
                                stpncpy(oldpwd, cwd, sizeof(oldpwd));
                                debug("Oldpwd %s.. ", oldpwd);
                            }
                        }
                        else {
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
				//Handle exit and quit
				else if ((strcmp(first_argument, "exit") == 0)
						|| (strcmp(first_argument, "quit") == 0)) {
					log_info("Shell Exit: pid %d", getpid());
					exit(0);
				}

				// Handle normal command
				if (arg_list[0] != NULL && isrun == 0) {
					//pid = fork();
					counter = 0;

					while (arg_list[counter] != NULL) {
						if(strchr(arg_list[counter],'&') || strchr(arg_list[counter],';')){
							continue;
						}
						char *arg_ind[30] = { 0 };//reinitialize to avoid previous option retained for current command.
						debug("%s***",arg_list[counter]);
						arg_ind[0] = arg_list[counter];
						char *opt[30] = { 0 };
						int optCounter = 0;

						//Handle options
						if (strchr(arg_ind[0], '-')) {
							opt[optCounter] = strtok(arg_ind[0], " -");

							while (arg_ind[optCounter] != NULL) {
								arg_ind[optCounter + 1] = strtok(NULL, " ");
								optCounter++;
							}
						}

						pid = fork();
						if (pid < 0) {
							return 1;
							log_info("Error forking the process, PID: %d", pid);
						} else if (pid == 0) {
							execvp(arg_list[counter], arg_ind);
							exit(errno);
						} else {
							waitpid(pid, &status, 0);
							if(WIFEXITED(status) && WEXITSTATUS(status) != 0){
								printf("Command not found in Custom shell.\n");
							}
						}
						counter++;
					}
				}
				if (alarmEnabled) {
					alarm(5);
				}
			}
		}

	} while (0);

	return retval;
}

/*
 * Function to pre-process input commands checking for delimiters.
 */
int preProcessCommand(char *buf) {
	char delim;
	char *delimiter = &delim;
	int length = 0;
	int index = 0;
	char cmd;
	char *temp = &cmd;
	//strcpy(temp, buf);
	//*temp = *buf;
	length = strlen(buf);
	while (length) {
		//char *c = (char *) &temp[index];
		if (buf[index] == '&') {
			*delimiter = '&';
			break;
		} else if (buf[index] == ';') {
			*delimiter = ';';
			break;
		}
		index++;
		length--;
	}
	return Execute(buf, delimiter);
}

/*
 * Function to get index of particular character in source string.
 */
int getIndex(char *str, char *delim)
{
	char *c;
	int index;
	c = strchr(str,delim);
	index = (int)(c-str);
	return index;
}

/*
 * Function to build command array.
 */
int parseToken(char *buf, COMMAND_ARRAY *cArray, int clear) {
	int retval = 1;
	int length = 0;
	int index = 0, count = 0;
	static int slevel = 0;
	static int array_count;

	if (clear) {
		slevel = 0;
		array_count = 0;
	}

	if (buf == NULL) {
		return 0;
	}

	do {
		length = strlen(buf);

		while (length) {
			char *c = (char *) &buf[index];

			/* raise our level if/when '(' is encountered in input */
			if (*c == '(') {
				++slevel;
			}

			/* lower our level if/when '(' is encountered in input */
			if (*c == ')') {
				--slevel;
			}

			if (isalpha(buf[index]) || isspace(buf[index]) || (*c == '-')
					|| (*c == ';') || (*c == '&')) {
				cArray[array_count].level = slevel;
				cArray[array_count].input[count] = buf[index];
				count++;
			}
			index++;
			length--;
		}
		cArray[array_count].input[count] = '\0';
		array_count++;
	} while (0);

	debug("EXIT");
	return retval;
}

/*
 * Function to check whether the entered expression is valid or not.
 */
int IsValidExpression(char *input){
	int valid = 0;
	int index = 0;
	int paranthesisCounter = 0;
	int length = strlen(input);

	while(length-1 > 0)
	{
		char *c = (char *)&input[index];
		if(*c == '(')
		{
			paranthesisCounter++;
		}
		else if(*c == ')')
		{
			paranthesisCounter--;
		}
		index++;
		length--;
	}
	if(paranthesisCounter == 0)
	{
		valid = 1;
	}
	return valid;
}

