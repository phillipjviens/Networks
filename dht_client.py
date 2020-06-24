import socket
import sys
import json


def connect(host,port):

    for res in socket.getaddrinfo(host,port,0,socket.SOCK_DGRAM):
        af, socktype,proto,canonname,sa = res

        try:
            sockfd = socket.socket(af,socktype,proto)
            sockfd.connect(sa)
        except socket.error:
            if sockfd:
                sockfd.close()
            sockfd = None
            continue
        break

    return sockfd,sa

def request(vars, sockfd, sa):
    if len(vars[4]) <= 1:
        sys.exit("ERROR: Invalid Key") 
    if vars[3] == "put":
        if len(sys.argv) == 5:
            msg = (0,vars[3],vars[4], "")
        else:
            msg = (0,vars[3],vars[4], vars[5])
        buffer = bytes(json.dumps(msg),'utf-8')
        sent = sockfd.sendto(buffer,sa)
        data,address = sockfd.recvfrom(4000)
        return(json.loads(data))
    elif vars[3] == "get":
        if len(sys.argv) > 5:
            sys.exit('ERROR: Too many arguments given')
        msg = (0, vars[3], vars[4])
        buffer = bytes(json.dumps(msg), 'utf-8')
        sent = sockfd.sendto(buffer,sa)
        data, address = sockfd.recvfrom(4000)
        return(json.loads(data))
    else:
        sys.exit("COMMAND LINE ERROR: Crud operation not supported")

def main():


    if len(sys.argv) < 3:
        sys.exit('ERROR: Destination Address or Port not provided')
    elif len(sys.argv) < 4:
        sys.exit('ERROR: crud operation not provided')
    elif len(sys.argv) < 5:
        sys.exit('ERROR: Key for KVP not provided')
    elif len(sys.argv) < 6:
        host = sys.argv[1]
        port = int(sys.argv[2])

    else:
        host = sys.argv[1]
        port = int(sys.argv[2])


    try:
        sockfd,sa = connect(host,port)
        results = request(sys.argv,sockfd,sa)

        for result in results:
            print(result)
    finally:
        sockfd.close()

if __name__ == "__main__":
    main()
