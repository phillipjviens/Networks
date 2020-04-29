#include <sys/types.h>
#include <sys/sockets.h>
#include <netdb.j>
#include <stio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#using namespace std;

void error(const char *msg){
    perror(msg);
    exit(1);
}

int receive_message(int sockfd, char*buffer, size_t buf_size, int flag){
    return buf_size;

}
