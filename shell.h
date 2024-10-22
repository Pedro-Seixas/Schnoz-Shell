#ifndef SHELL
#define SHELL
#include <cstring>
#include <string>

class Shell {
private:
	char* parsed[64];
	char* parsedAfterPipe[64];
	char* user;
	std::string prompt;
	bool exitFlag;
	bool isBackgroundTask;
	bool isPiped;

public:
	Shell();
	void shell_init();
	void shell_change_dir();
	void update_prompt();
	void motd();
	void parse_command(char* commandBuff);
	void display_prompt();
	void shell_general_command();
	void shell_general_command_piped();
	void reset_command_buff();
	void shell_run();
	bool get_command(char* commandBuff);
	int shell_builtin_command();


};
#endif
