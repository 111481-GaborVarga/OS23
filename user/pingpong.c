#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
	int pipe_[2];
	pipe(pipe_);

	int pid = fork();
	char message;

	if(pid == 0){	//child
		read(pipe_[0], &message, 1);
		printf("%d: received ping\n", getpid());
		write(pipe_[1], "b", 1);

	} else {	//parent
		write(pipe_[1], "a", 1);
		sleep(1);
		read(pipe_[0], &message, 1);
		printf("%d: received pong\n", getpid());
	}

	exit(0);
}
