#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
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
    int total;
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
        total+=bits;

    }
    return total;

}



//Checks to see if a given string is an integer. I utilized this
//for both the port number and Content Length. 
bool isNumber(string num_str){
    if(num_str.size() == 0) return false;
    for(unsigned int i = 0;i<num_str.size();i++){
        if((num_str[i]>='0' && num_str[i] <='9')==false){
            return false;
        }
    }
    return true;

}



int main (int argc, char *argv[]){
    int sockfd, s, n, length, con_length, size;
    int buffer_size = 256;
    char* buffer = (char*) malloc(buffer_size);
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    
    size_t col,next_col, sla, 
    eoh_l, delim;
    string protocol,host,port,path, resp, url, 
    url_sub, con_url, addr, con_l, con_sub, 
    content_length, request, head;
    
    string prot = "http://";
    string header_con = "Content-Length: ";
    string colon = ":";
    string slash = "/";
    string eoh = "\r\n\r\n";

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
 

    // Checks to ensure that a least 2 arguments are given
    if(argc < 2){

        error("URL NOT SPECIFIED");
    }

    //If 2 only 2 are given checks for URL formatting and then parses the URL
    else if(argc < 3) {

        url = argv[1];

        //Checks to enusre that http:// protocol is specified. returns error if not.
        if(!strstr(url.c_str(),prot.c_str())){
            error("HTTP PROTOCOL NOT SPECIFIED OR IMPROPERLY FORMATTED");
        }
        else{

            //If protocol is specified we parse the URL checking for portnumbers
            //and path.
            col= url.find_first_of(":");
            protocol =url.substr(0,col);
            url_sub=url.substr(col+3);
            if (strstr(url_sub.c_str(),colon.c_str())){
                next_col = url_sub.find_first_of(":");
                host = url_sub.substr(0,next_col);
                if(strstr(url_sub.c_str(),slash.c_str())){
                    sla = url_sub.find_first_of("/");
                    port = url_sub.substr(next_col+1, sla-next_col-1);
                    if(!isNumber(port)){
                        error("INVALID PORT NUMBER");
                    }
                    path = url_sub.substr(sla);
                }
                else{
                    port = url_sub.substr(next_col+1);
                    path = "/";
                    if(!isNumber(port)){
                        error("INVALID PORT NUMBER");
                    }  
                }
            }
            else if (strstr(url_sub.c_str(),slash.c_str())){
                sla = url_sub.find_first_of("/");
                host = url_sub.substr(0,sla);
                port = "80";
                path = url_sub.substr(sla);
            }
            else{
                
                host = url.substr(col+3);
                port = "80";
                path = "/";

            }

            int psize = port.length();
            char portnum[psize+1];
            strcpy(portnum,port.c_str());

            url = argv[1];
            col = url.find_first_of(":");
            addr = url.substr(col+3);
            int l = host.length();
            char char_addr[l+1];
            strcpy(char_addr,host.c_str());
	    s = getaddrinfo(char_addr,portnum,&hints,&result);

        }

       
    }
    else{
        url = argv[1];
        port = argv[2];
        if(!strstr(url.c_str(),prot.c_str())){
            error("HTTP PROTOCOL NOT SPECIFIED OR IMPROPERLY FORMATTED");
        }
        else {
            col = url.find_first_of(":");
            protocol = url.substr(0,col);
            url_sub = url.substr(col+3);
            if(strstr(url_sub.c_str(),slash.c_str())){
                sla = url_sub.find_first_of("/");
                host = url_sub.substr(0,sla);
                path = url_sub.substr(sla);
            }
            else{
                host = url_sub;
                path = slash;
            }

        }

        url = argv[1];
        col = url.find_first_of(":");
        addr = url.substr(col+3);
        length = host.length();
        char char_addr[length+1];
        strcpy(char_addr,host.c_str());
	    s = getaddrinfo(char_addr,argv[2],&hints, &result);
    }



    length= host.length();
    con_url = protocol+host;
    char char_host[length+1];
    strcpy(char_host,con_url.c_str());


    if(s != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }




    //iterate linked list to find connection
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


    //concat a request string 
    request = "GET " + path + " HTTP/1.1\r\n"
              "Host: " +host+ "\r\n"
              "Connection: close\r\n"
              "\r\n";

    //convert request string to char array.
    length = request.length();
    char char_req[length + 1];
    strcpy(char_req,request.c_str());


    //write the request to stderr
    cerr << char_req << endl;

    n = message("send", sockfd, char_req, sizeof(char_req),0);
    if ( n < 0){
        error("ERROR WRITING TO SOCKET");
    }


    int i = 0;
    n = 0;
    size= buffer_size;


    //while loop that continues to receive until all bytes
    //are received. 
    while((n = recv(sockfd, buffer + i, 256,0)) > 0){
        i+= n;
        size = buffer_size;
        buffer_size += 256;
        char *new_buffer= (char *)malloc(buffer_size);
        memcpy(new_buffer, buffer, size);
        free(buffer);
        buffer = new_buffer;
        resp = buffer;

        //Parse the header to pull out necessary information
        if (strstr(resp.c_str(),eoh.c_str())){
            eoh_l = resp.find(eoh);
            head = resp.substr(0, eoh_l);
            if(strstr(resp.c_str(),header_con.c_str())){
                delim = resp.find(header_con);
                con_l = resp.substr(delim);
                delim = con_l.find("\r\n");
                con_sub = con_l.substr(0,delim);
                delim = con_sub.find_first_of(":");
                content_length = con_sub.substr(delim+2);
                if(isNumber(content_length)){
                    stringstream con_conv(content_length);
                    con_length = 0;
                    con_conv >> con_length;
                }
            }
        }
    }

    char header[eoh_l];
    char body[con_length+1];
    
    //Copy the header
    for(unsigned int i = 0; i < eoh_l; i++){
        header[i] = buffer[i];
    }


    //Print the header to stderr
    cerr << header << endl;

    for(unsigned int i = 0; i < eoh_l+con_length; i++){
        body[i] = buffer[i+4+eoh_l];
    }    
    char ret[4] = "\r\n";

    //Write the response body and then new line 
    write(STDOUT_FILENO,body, con_length);
    write(STDERR_FILENO, ret, sizeof(ret));
    freeaddrinfo(result);
    close(sockfd); 
    

    return 0;
}
