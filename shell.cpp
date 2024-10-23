#include "shell.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>

#define RED "\e[0;31m"
#define CYN "\e[0;36m"
#define GRN "\e[0;32m"
#define MAG "\e[0;35m"
#define RST "\e[0m"
#define GRA "\e[38;5;250m"

/*
 * Constructor that initializes instances and start shell.
 */
Shell::Shell() {
	exitFlag = false;
	isBackgroundTask = false;
	isPiped = false;
	user = getlogin();
	shell_init();
}

/*
 * Function that starts shell and call function to update prompt.
 */
void Shell::shell_init() {
	system("clear");
	update_prompt();

	printf(CYN "\n\n--------------------------------------\n");
	printf("\n--------Schnoz Shell Initiated--------\n");
	printf("\n--------------------------------------\n\n" RST);
	fflush(stdout);

	sleep(2);

	motd();
}

/*
 * Function that prints the message of the day.
 */
void Shell::motd() {
	system("clear");
	printf(CYN "\n\n----------Message of the Day----------\n");
	printf("\n''Funny how? Funny like a clown? I amuse you?''\n");
	printf("\n--------------------------------------\n\n" RST);

	sleep(3);
	system("clear");
}

/*
 * Funtion that displays prompt with "user:directory >>>"
 */
void Shell::display_prompt() {
	char commandBuff[1024];

	printf(MAG "\n %s" RST, std::string(user).c_str());
	printf(GRN ":%s " RST, prompt.c_str());
	printf(CYN ">>> " RST);

}

/*
 * Function that read prompt input. It returns true if successfull
 * If empty input, return false for new prompt.
 */
bool Shell::get_command(char* commandBuff) {
	char buff[1024];

	fgets(buff, sizeof(buff), stdin);
	buff[strcspn(buff, "\n")] = '\0';

	if(strlen(buff) == 0) {
		return false;
	} else {
		strcpy(commandBuff, buff);
	}

	return true;

}

/*
 * Function that parses the input to words/tokens.
 * It checks for "|" for pipes and "&" for background process.
 * It will then copy the parsed input to the class instances.
 */
void Shell::parse_command(char* commandBuff) {
	char* args[64];
	char* argsPiped[64];
	char* word = strtok(commandBuff, " ");
	int i = 0;
	int k = 0;

	while (word != NULL && i < 63 && k < 63) {
		if(strcmp(word, "|") != 0) {
			if(isPiped) {
				argsPiped[k++] = word;
			} else {
				args[i++] = word;
			}
		} else {
			isPiped = true;
		}
		word = strtok(NULL, " ");
	}

	//Check if it is a background task
	if(strcmp(args[i-1], "&") == 0) {
		isBackgroundTask = true;
		args[i - 1] = NULL;
	} else {
		isBackgroundTask = false;
	}

	args[i] = NULL;
	argsPiped[k] = NULL;

	for (int j = 0; j <= i; ++j) {
		parsed[j] = args[j];
	}

	if(isPiped) {
		for (int j = 0; j <= k; ++j) {
			parsedAfterPipe[j] = argsPiped[j];
		}
	}

}

/*
 * Function that updates the prompt with the current directory.
 */
void Shell::update_prompt() {
	char dir[1024];
	if(getcwd(dir, sizeof(dir)) != NULL) {
		prompt = std::string(dir);
	}
}

/*
 * Function that changes the directory using chdir().
 */
void Shell::shell_change_dir() {
	chdir(parsed[1]);
	update_prompt();
}

/*
 * Function that handles defined commands like exit, message of the day,
 * and cd. If passed command is not defined, then it will return false.
 * to do a general command.
 */
int Shell::shell_builtin_command() {
	int numberOfCommands = 3;
	const char* listOfCommands[numberOfCommands];
	int switchIndex = 0;

	listOfCommands[0] = "exit";
	listOfCommands[1] = "motd";
	listOfCommands[2] = "cd";

	for (int i = 0; i < numberOfCommands; i++) {
		if (strcmp(parsed[0], listOfCommands[i]) == 0) {
			switchIndex = i + 1;
			break;
		}
	}

	switch(switchIndex) {
	case 1:
		exitFlag = true;
		printf("\nExiting.");
		fflush(stdout);
		usleep(500000);
		printf(".");
		fflush(stdout);
		usleep(500000);
		printf(".");	
		fflush(stdout);
		usleep(500000);
		system("clear");

		printf(CYN "\n\n--------------------------------------\n");
		printf("\n---------------Goodbye!---------------\n");
		printf("\n--------------------------------------\n" RST);
		fflush(stdout);

		sleep(2);
		system("clear");
		return 1;
	case 2:
		motd();
		return 1;
	case 3:
		shell_change_dir();
		return 1;
	default:
		return 0;
	}
}

/*
 * Function that handles general commands that will be executed using execvp().
 * It creates a child processto execute the command. If the background flag is true
 * The parent process will not wait on the child to complete and will continue the prompt.
 */
void Shell::shell_general_command() {
	int pid = fork();

	if(pid < 0) {
		printf("\nFailed to Fork\n ");
		return;
	} else if(pid == 0) {
		// child process
		printf("\n");

		if(execvp(parsed[0], parsed) < 0) {
			printf(RED "Failed to run command / Command not found\n" RST);
			exit(0);
		}
	} else {
		if(!isBackgroundTask) {
			waitpid(pid, NULL, 0);
			printf(GRA "\n-------------Program ended-------------\n" RST);
		}
	}
}

/*
 * Function that handles piped commands. It creates two children processes
 * one for the first half of the command (left of the "|") and the other for the second half.
 * Pipes are used to pass the output of the first half to the second half.
 * For example a "cat file.txt | sort" command.
 */
void Shell::shell_general_command_piped() {
	int pid1;
	int pid2;
	int p[2];

	if(pipe(p) < 0) {
		printf("\nFail to Pipe\n");
		return;
	}

	pid1 = fork();

	if(pid1 < 0) {
		printf("\nFailed to Fork\n");
		return;
	} else if(pid1 == 0) {
		// child 1 process
		close(p[0]);
		dup2(p[1], STDOUT_FILENO);
		close(p[1]);

		if(execvp(parsed[0], parsed) < 0) {
			printf(RED "Failed to run command / Command not found\n" RST);
			exit(0);
		}
		printf("\n");

	} else {
		pid2 = fork();

		if(pid2 < 0) {
			printf("\nFailed to Fork\n");
			return;
		} else if(pid2 == 0) {
		    // child 2 process
			close(p[1]);
			dup2(p[0], STDIN_FILENO);
			close(p[0]);

			if(execvp(parsedAfterPipe[0], parsedAfterPipe) < 0) {
				printf(RED "Failed to run command / Command not found\n" RST);
				exit(0);
			}
		} else {
			close(p[0]);
			close(p[1]);
			waitpid(pid2, NULL, 0);
		}
		waitpid(pid1, NULL, 0);
	}

}

/*
 * Function that resets the parsed string.
 */
void Shell::reset_command_buff() {
	for (int i = 0; i < 64; ++i) {
		parsed[i] = NULL;
	}
}

/*
 * Main loop that applies the run logic of displaying prompt,
 * gettint input, processing the char array and executing the command.
 */
void Shell::shell_run() {
	while(!exitFlag) {
		char commandBuff[1024];

		display_prompt();

		if(get_command(commandBuff)) {
			parse_command(commandBuff);
		} else {
			continue;
		}

		if(!shell_builtin_command()) {
			if(isPiped) {
				shell_general_command_piped();
				isPiped = false;
			} else {
				if(!isBackgroundTask) {
					printf(GRA "------------Starting Program------------\n" RST);
				}
				shell_general_command();
			}
		}
	}
}
