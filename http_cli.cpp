#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <ctype.h>

using namespace std;

int portn;

void error(const char *msg){
    perror(msg);
    exit(1);
}


// Sends or recieves in a loop to ensure that message has been fully
//transmitted.
int message(string func, int sockfd, char*buffer, size_t buf_size, int flag){
    size_t bits = buf_size;
    ssize_t n;
    while(bits > 0){
        if (func == "receive"){
            n = recv(sockfd, buffer, bits, flag);
        }
        else if (func == "send"){
            n = send(sockfd, buffer, bits, flag);
        }

        if (n <= 0){
            return n;
        }
        bits -= n;
        buffer += n;

    }
    return buf_size;

}

bool isNumber(string portnum){
    if(portnum.size() == 0) return false;
    for(int i = 0;i<portnum.size();i++){
        if((portnum[i]>='0' && portnum[i] <='9')==false){
            return false;
        }
    }
    return true;

}

int main (int argc, char *argv[]){
    int sockfd, s, n, newsockfd;
    char buffer[131072];
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    
    if(argc < 2){

        error("URL NOT SPECIFIED");
    }
    else if(argc < 3) {
        string url = argv[1];
        size_t found = url.find_first_of(":");
        string protocol =url.substr(0,found);
        string url_new=url.substr(found+3);
        size_t found1 = url_new.find_first_of(":");
        string host = url_new.substr(0,found1);

        size_t found2 = url_new.find_first_of("/");
        string port = url_new.substr(found1+1, found2-found1-1);
        string path = url_new.substr(found2);

        cout << "Protocol: " << protocol <<endl;
        cout <<"host: " << host<< endl;
        //cout << "port: " << port << endl;
        cout << "path: " << path << endl;

        if(isNumber(port)){

            int n = port.length();
            char port_array[n+1];

            strcpy(port_array, port.c_str());
            cout << "PORTNUM: " << port_array << endl;
            s = getaddrinfo(argv[1], port_array, &hints, &result);
            //Grab the port number
        }
    }
    else{

        s = getaddrinfo(argv[1],argv[2],&hints, &result);

        cout << "s: " << s << endl;
    }


    
    //s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if(s != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype,
            rp->ai_protocol);
        if(sockfd == -1){
            continue;
        }
        if(connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1){
            break;
        }
        close(sockfd);
    }

    if(rp == NULL){
        error("COULD NOT CONNECT\n");
    }


    freeaddrinfo(result);
    cout << "we made it here" << endl;

    printf("Please enter the message: ");
    bzero(buffer,131072);
    fgets(buffer,131072,stdin);
    
    n = message("send",sockfd,buffer,131072,0);
    if(n < 0){
        error("Error writing to socker");
    }
    n = message("receive", sockfd, buffer, 131072,0);
    if(n < 0){
        error("Error reading from socket");
    }

    cout << "Message received from the server: " << buffer << endl;

    bzero(buffer, 131072);
    n = message("receive", sockfd, buffer, 131072, 0);
    if(n < 0){
        error("Error reading socket");
    }

    cout << buffer << endl;
    close(sockfd);
    //do the stuff

    return 0;
}