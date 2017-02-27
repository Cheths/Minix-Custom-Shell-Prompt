#include "common.h"

/*bool startsWith (char* base, char* str) {
    return (strstr(base, str) - base) == 0;
}*/

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

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

char* duplicate(char *command) {
	int len;
	char *hs;
	//log_info("Inside duplicate::%s",command);
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
		//log_info("Exiting duplicate::%s",command);
		return hs;
	}
}

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
	/*
	 * execute the command based on their level, as rightmost input in
	 * '()' has higher priority.
	 */
	retval = 0;
	;
	while (cArray[finalcount].level != -1) {
		strcat(finalString, cArray[finalcount].input);
		finalcount++;
	};
	//return finalString;
}

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

			if (strchr(buf, '<') || strchr(buf, '>')) {
				printf("Command with &,< and > is not supported\n");
				break;
			}

			arg_list[counter] = strtok(buf, delimiter);

			if (arg_list[counter] != NULL) {
				strcpy(first_argument, arg_list[0]);
				while (arg_list[counter] != NULL) {
					counter++;
					char *prevCommand = arg_list[counter-1];
					if(startsWith("(",prevCommand)){
						arg_list[counter-1] = '\0';
						char stringBraces[1024];
						memset(stringBraces,0,1024);
						handleBraceAndReturnString(retval,&stringBraces,prevCommand);
						preProcessCommand(stringBraces);

					}
					else{
						arg_list[counter] = strtok(NULL, delimiter);
					}
					//counter++;



				}

				//Handle exit and quit
				if ((strcmp(first_argument, "exit") == 0) || (strcmp(first_argument, "quit") == 0)) {
					log_info("Shell Exit: pid %d", getpid());
					exit(0);
				}

				// Handle normal command
				if (arg_list[0] != NULL && isrun == 0) {
					debug("first_argument %s", first_argument);
					//pid = fork();
					//char *arg_ind[30] = { 0 };
					counter = 0;

					while (arg_list[counter] != NULL) {
						char *arg_ind[30] = { 0 };	//reintialize to avoid previous option retained for current command.
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
							log_info("Inside execv loop: PID: %d", pid);
							log_info("Arg is %s", arg_list[counter]);
							log_info("counter-->%d", counter);
							log_info("Before Child exit..");
							execvp(arg_list[counter], arg_ind);
							exit(0);
						} else {
							log_info("Inside waitpid loop: PID: %d", pid);
							log_info("Arg is %s", arg_list[counter]);
							log_info("counter-->%d", counter);
							waitpid(pid, &status, 0);
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

int preProcessCommand(char *buf) {
	char *delimiter;

	/*char *ampersandChar = strchr(buf, '&');
	char *semicolonChar = strchr(buf, ';');*/

	/*int ampersandCharLength = (int) (ampersandChar - buf);
	int semicolonCharLength = (int) (semicolonChar - buf);*/

	int ampersandCharLength = getIndex(buf,'&');
	int semicolonCharLength = getIndex(buf,';');

	if (ampersandCharLength > 0 && ampersandCharLength < semicolonCharLength) {
		delimiter = "&";
	} else if(semicolonCharLength > 0 && ampersandCharLength > semicolonCharLength){
		delimiter = ";";
	}
	return Execute(buf, delimiter);
}

int getIndex(char *str, char *delim)
{
	char *c;
	int index;
	c = strchr(str,delim);
	index = (int)(c-str);
	return index;
}

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
		debug("length is %d",length);

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

int IsValidExpression(char *input){
	int valid = 0;
	int index = 0;
	int paranthesisCounter = 0;
	//char *arg_list[30] = {0};
	int length = strlen(input);
	//*arg_list = input;

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

