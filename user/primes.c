#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define ROOF 35

void
iterate_sieve(int fd)
{
    // read first number, it is prime
    int number;
    read(fd, &number, sizeof(int));
    fprintf(1, "prime %d\n", number);

    // write non divisible to new pipe
    int p[2];
    pipe(p);
    int new_number = -1;
    while(read(fd, &new_number, sizeof(int)) != 0){
        if(new_number % number != 0){
            write(p[1], &new_number, sizeof(int));
        }
    }
    close(p[1]);
    close(fd);

    // iterate if more numbers
    if(new_number != -1){
        iterate_sieve(p[0]);
    }
    close(p[0]);

    exit(0);
}

int
main(int argc, char argv[])
{
    if(argc > 1){
        fprintf(2, "Argument error\n");
        exit(1);
    }

    // write all numbers and pass read fd
    int p[2];
    pipe(p);

    for(int i = 2; i <= ROOF; i++){
        write(p[1], &i, sizeof(int));
    }
    close(p[1]);
    iterate_sieve(p[0]);
    close(p[0]);
    exit(0);
}
