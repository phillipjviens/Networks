#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

using namespace std; 
int sockfd, newsockfd, portno, clilen, n,
sock_count, sock_total, buf_size;
const int BUFFER_SIZE = 131072;
char buffer[BUFFER_SIZE];
ifstream inFile;

void error(const char *msg){
    perror(msg);
    exit(1);
}


int message(string func, int sockfd, char*buffer, size_t buf_size, int flag){
    size_t bytes = buf_size;
    ssize_t n;

    int total;


    while(bytes > 0){
        if(func == "receive"){
            n = recv(sockfd, buffer, bytes, flag);
        }
        else if(func == "send"){
            n = send(sockfd, buffer, bytes, flag);
        }

        if( n <= 0){
            return n;
        }

        bytes -= n;
        buffer += n;
        total += bytes;
    }
    return total;
}

int main(int argc, char *argv[]){
    char *ptr = NULL;
    struct sockaddr_in serv_addr, cli_addr;
    string req;
    struct sockaddr_in{
        short sin_family;
        u_short sin_port;
        struct in_addr sin_addr;
        char sin_zero[8];

    };


//WEB ROOT Function

//index.html function

    if (argc < 2){
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if(sockfd < 0){
        error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = strtol(argv[1],&ptr,10);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR on binding");
    }

    if(listen(sockfd,5) != 0){
        perror("ERROR at Listen");
    };

    clilen = sizeof(cli_addr);

    while(1){
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);

        if(newsockfd < 0){
            error("ERROR on accept");
        }

            string con_type = "Connection: close";
            string bad_path = "/../";
            string paths[] = {"path1"};
            
            
            //Path requested by GET request. 
            string path;
            bzero(buffer,131072);
            //n = message("receive",newsockfd, buffer, 131072, 0);
            n = recv(newsockfd, buffer, sizeof(buffer), 0);
            req = buffer;

            size_t delim = req.find_first_of("/");
            string req_type = req.substr(0,delim-1);
            cout <<"REQUEST: " << req_type << endl;

            if (req_type != "GET"){
                //bzero(buffer, 131072);
                string response = "HTTP/1.1 501 NOT IMPLEMENTED\r\n"
                                  "Host: cs1.seattleu.edu\r\n"
                                  "Content Length: 0\r\n"
                                  "Connection: close\r\n"
                                  "\r\n";
                int length = response.length()+1;
                char resp[length];
                bzero(resp,length);
                strncpy(resp, response.c_str(), length);
                n = send(newsockfd, resp,length,0);
                if (n < 0){
                    error("ERROR Writing to socket");
                }
            }
            else if (strstr(req.c_str(),bad_path.c_str())){
                string response = "HTTP/1.1 400 BAD REQUEST\r\n"
                                  "Host: cs1.seattleu.edu\r\n"
                                  "Connection: close\r\n"
                                  "\r\n";
                int length = response.length()+1;
                char resp[length];
                bzero(resp,length);
                strncpy(resp, response.c_str(), length);
                n = send(newsockfd, resp,length,0);
                if (n < 0){
                    error("ERROR Writing to socket");
                }
            }
            else{
                size_t path_length = req.find("HTTP");
                string path = req.substr(delim+1,path_length-6  );
                cout << path.length() << endl;
                if(FILE *file = fopen(path.c_str(), "r")){
                    cout << path << endl;
                    
                }
                else{
                    cout << "NO FILE LOCATED: " << path << endl;
                    string response = "HTTP/1.1 404 NOT FOUND\r\n"
                                      "Host: cs1.seattleu.edu\r\n"
                                      "Connection: close\r\n"
                                      "\r\n";
                    int length = response.length()+1;
                    char resp[length];
                    bzero(resp,length);
                    strncpy(resp,response.c_str(),length);
                    n = send(newsockfd, resp, length,0);
                    if(n < 0){
                        error("ERROR Writing to socket");
                    }
                }
                
            }

 

    //cout << "SIZE OF BUFFER: " << buffer << endl;
   // printf("GET REQUEST is: %s\n", buffer);
    if (n < 0){
        error("ERROR reading from socket");
    }

        
    }

    return 0;
}