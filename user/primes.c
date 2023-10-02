#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


void primes(int p) {

    int prime;
    int num = -1;
    int flag = 0;
    int pipe_[2];

    pipe(pipe_);

    read(p, &prime, sizeof(int));
    printf("prime %d\n", prime);

    while(1) {
        flag = read(p, &num, sizeof(int));

        //if there is nothing left to read, the loop ends
        if(flag <= 0) {
            break;
        }

        if (num % prime != 0) {
            write(pipe_[1], &num, sizeof(int));
        }
    }

    // there is no number left in the pipe, program ends here
    if (num == -1) {
        close(p);
        close(pipe_[0]);
        close(pipe_[1]);
        return;
    }

    int pid = fork();

    if(pid == 0) {
        //child
        close(p);
        close(pipe_[1]);
        primes(pipe_[0]);
        close(pipe_[0]);
    } else {
        //parent
        close(p);
        close(pipe_[0]);
        close(pipe_[1]);
        wait(0);
    }
}

int main(int argc, char *argv[]){

    if (argc > 1) {
        printf("Primes takes no arguments!\n");
        exit(-1);
    }

    int max_number = 35;
    int pipe_[2];
    pipe(pipe_);

    // fill the pipe with all the numbers (until max_number - 1)
    for(int i = 2; i < max_number; i++) {
        write(pipe_[1], &i, sizeof(int));
    }

    close(pipe_[1]);
    primes(pipe_[0]);
    close(pipe_[0]);
    exit(0);
}
