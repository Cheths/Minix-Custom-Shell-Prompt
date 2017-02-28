
/**
 * This file supports all the basic functions for Minix shell prompt.
 */
#include "common.h"

/*
 * Function to check whether source string starts with a particular character.
 */
bool startsWith(const char *prefixString, const char *srcString)
{
    size_t lenpre = strlen(prefixString), lenstr = strlen(srcString);
    return lenstr < lenpre ? false : strncmp(prefixString, srcString, lenpre) == 0;
}

/*
 * Function to handle 'Ctrl+C' keystroke.
 */
void ctrl_C_Handler() {
	char choice;
	printf("\nDo you want to exit? Enter Y/n \n");
	scanf("%c", &choice);
	log_info("Cmd entered:%c", choice);
	if (choice == 'Y' || choice == 'y') {
		log_info("successful exit");
		exit(0);
	}
}

/*
 * Function to show the alarm after 5 seconds of user command input.
 */
void alarm_Handler() {
	char choice;
	printf("\nDo you want to exit? Enter Y/n \n");
	scanf("%c", &choice);
	log_info("Cmd entered:%c", choice);
	if (choice == 'Y' || choice == 'y') {
		log_info("successful exit");
		exit(0);
	} else {
		longjmp(jmpBuf, 1);
	}
}

/*
 * Function to handle signal interrupts.
 */
void signalhandler(int signalCode) {
	switch (signalCode) {
		case SIGINT: {
			char str;
			printf("\nAre you sure?[Y/n]");
			str = getchar();
			if (str == 'y' || str == 'Y')
				exit(0);
			else
				longjmp(jmpBuf, 1);
			break;
		}
		case SIGCHLD: {
			int pid;
			pid = wait(NULL);
			printf("PID:%d completed job.\n", pid);
			longjmp(jmpBuf, 1);
		}
	}
}

/*
 * Function to get a clone of the input command.
 */
char* duplicate(char *command) {
	int length;
	char *dupString;
	if (command == NULL) {
		return NULL;
	}

	length = strlen(command) + 1; //+1 for the null character
	dupString = (char *) malloc(length * sizeof(char));

	if (dupString == NULL) {
		log_info("Malloc failed");
		return NULL;
	} else {
		strcpy(dupString, command);
		return dupString;
	}
}

/*
 * Function to read profile from the "Shell_Profile" config file.
 */
char* readProfile(char *type) {
	FILE *fp;
	char *home, *promptsign, *temp, *temp1, *position;
	char command[20];
	int length;
	struct stat stat_var;
	static char check[100] = "Shell_Profile";

	do {
		if (stat(check, &stat_var) == -1){
			printf("OOOOOps..");
			printf("Profile file could not be opened. Creating a new profile file called profile_custom in /home\n");
			fp = fopen("Shell_Profile", "wb+");

			if (fp == NULL) {
				printf("failed to open file /usr/src/Minix-Custom-Shell-Prompt/Shell_Profile\n");
				return NULL;
			} else {
				fprintf(fp, "#profile file\n");
				fprintf(fp, "prompt=$\n");
				fprintf(fp, "path=/bin:/usr/bin\n");
				fprintf(fp, "home=/usr/src/Minix-Custom-Shell-Prompt/\n");
				fprintf(fp, "alarmEnabled=true");
				fclose(fp);
				log_info("Profile file  is custom loaded\n");
			}
		} else {
			fp = fopen(PROFILE, "r");
			if (fp == NULL) {
				log_info("Cannot open PROFILE file");
				return NULL;
			}
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
				length = strlen(promptsign);
				position = promptsign + length - 1;
				*position = '\0';
			} else if (!strcmp(temp, "path")) {
				char *path;
				temp1 = strtok(NULL, "=");
				position = temp1 + strlen(temp1) - 1;
				*position = '\0';
				setenv("PATH", temp1, 1);
			} else if (!strcmp(temp, "home")) {
				temp1 = strtok(NULL, "=");
				home = duplicate(temp1);
				if (!home) {
					return NULL;
				}
				length = strlen(home);
				position = home + length - 1;
				*position = '\0';
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
void initializeCommandArray(COMMAND_ARRAY *cmdArray) {
	int i = 0;

	if (cmdArray == NULL) {
		return;
	}

	for (i = 0; i < ARR_SIZE; i++) {
		memset(&cmdArray[i].input, 0, sizeof(SIZE));
		cmdArray[i].level = -1;
	}
}

/*
 * Function to handle braces and return the modified string.
 */
void handleBraceAndReturnString(int retval, char* finalString[1024], char* temp) {
	int finalcount = 0;
	char *token;

	token = strtok(temp, ",\n");
	retval = parseToken(token, cmdArray, 1);

	while (token != NULL) {
		token = strtok(NULL, ",\n");
		if (token != NULL)
			retval = parseToken(token, cmdArray, 0);
	}
	
	retval = 0;
	while (cmdArray[finalcount].level != -1) {
		strcat(finalString, cmdArray[finalcount].input);
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

			if (*delimiter == '0') {
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

					} else if(strchr(prevCommand, '&') && !startsWith("&",prevCommand)){
						strtok(prevCommand, "&");
						arg_list[counter] = strtok(NULL, "&");
					}else if(strchr(prevCommand, ';') && !startsWith(";",prevCommand)){
						strtok(prevCommand, ";");						
						arg_list[counter] = strtok(NULL, ";");
						
					}else {
						if (*delimiter != '0') {
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
                            }
                        }
                        else {
                            /* cache current directory value before changing the
                             * directory */
                            valid_oldpwd = 1;
                            stpncpy(oldpwd, getcwd(NULL, 0), sizeof(oldpwd));
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
					counter = 0;

					while (arg_list[counter] != NULL) {
						if(strchr(arg_list[counter],'&') || strchr(arg_list[counter],';')){
							continue;
						}
						char *arg_ind[30] = { 0 };//reinitialize to avoid previous option retained for current command.
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
						} else if(strchr(arg_ind[0], ' ')){
							opt[optCounter] = strtok(arg_ind[0], " ");

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
	char delim = '0';
	char *delimiter = &delim;
	int length = 0;
	int index = 0;
	char cmd;
	char *temp = &cmd;
	length = strlen(buf);

	while (length) {
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
 * Function to build command array.
 */
int parseToken(char *buf, COMMAND_ARRAY *cmdArray, int clear) {
	int retval = 1;
	int length = 0;
	int index = 0, count = 0;
	static int strLevel = 0;
	static int array_count;

	if (clear) {
		strLevel = 0;
		array_count = 0;
	}

	if (buf == NULL) {
		return 0;
	}

	do {
		length = strlen(buf);

		while (length) {
			char *c = (char *) &buf[index];

			if (*c == '(') {
				++strLevel;
			}

			if (*c == ')') {
				--strLevel;
			}

			if (isalpha(buf[index]) || isspace(buf[index]) || (*c == '-') || (*c == ';') || (*c == '&')) {
				cmdArray[array_count].level = strLevel;
				cmdArray[array_count].input[count] = buf[index];
				count++;
			}
			index++;
			length--;
		}
		cmdArray[array_count].input[count] = '\0';
		array_count++;
	} while (0);

	return retval;
}

/*
 * Function to check whether the entered expression is valid or not.
 */
int isValidExpression(char *inputExpression){
	int isValid = 0;
	int index = 0;
	int paranthesisCount = 0;
	int length = strlen(inputExpression);

	while(length-1 > 0) {
		char *c = (char *)&inputExpression[index];
		if(*c == '(') {
			paranthesisCount++;
		} else if(*c == ')') {
			paranthesisCount--;
		}
		index++;
		length--;
	}
	if(paranthesisCount == 0) {
		isValid = 1;
	}
	return isValid;
}

