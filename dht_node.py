import sys
import linecache
import hashlib
import json

node_list = dict()
lines = dict()
finger = list()
node_index = []
current = None
m = 160
class Node:
    def __init__(self, id, hex, port, host):
        self.id = id
        self.hex = hex
        self.port = port
        self.host = host
        self.prev = None
        self.data = dict()


#Connects to other nodes
def connectNode(host,port):
    for res in socket.getaddrinfo(host,port,0, socket.SOCK_DGRAM):
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
    return sockfd, sa

#compiles the list of active nodes from the file given
def get_active_nodes(file):
    node_lines = linecache.getlines(file)
    i = 0
    for line in node_lines:
        host = line.split()[0]
        port = line.split()[1]
       

        addrinfo = getIp(host,port)

        pton_ip = socket.inet_pton(socket.AF_INET, addrinfo[0])
        htons_port = socket.htons(addrinfo[1])


        id, hex = getID(bytes(pton_ip) + bytes(htons_port))
        node_list[id] = Node(id, hex, port, host)
        lines[i] = Node(id, hex, port, host)

        node_index.insert(i, id) 
        i+=1
    node_index.sort()


#aquires the connection information for other nodes
def getIp(host, port):
     for res in socket.getaddrinfo(host,port,0,socket.SOCK_DGRAM):
        af, socktype,proto,canonname, sa = res
        try:
            sockfd = socket.socket(af,socktype,proto)
            sockfd.connect(sa)
        except socket.error:
            #print(sa)
            if sockfd:
                sockfd.close()
            sockfd = None
            continue
        return sa

#returns the hash IDs and hexidecimal representations
def getID(key):
    key_hash = hashlib.sha1(str(key).encode())
    key_id = int(key_hash.hexdigest(),16) % (2**m)
    key_hex = key_hash.hexdigest()
    return key_id, key_hex


#Returns the information of the current node 
def getCurrentNode(start):
    current = lines[int(start)-1]

    current_index = node_index.index(current.id)
    print("The current index: ", current_index)
    if current_index == 0:
        prev = node_index[len(lines)-1]
    else:
        prev = node_index[current_index-1]

    print("previous index: ", prev)
    return current, prev


#compiles a finger table for the current node
def getFingers(current):
    l = 0
    for i in range(160):
        next_index = (node_index.index(current.id) + 1) % len(node_index)
        finger_key = (current.id + (2**i) % (2**m))
        if finger_key < node_index[next_index]:
            if node_index[next_index] not in finger and node_index[next_index] != current.id:
                finger.insert(l, node_index[next_index])
                l+=1
        else:
            for k in range(1, len(node_index)-1,1):
                next_index = (node_index.index(current.id) + k ) % len(node_index)
                if finger_key < node_index[next_index]:
                    if node_index[next_index] not in finger and node_index[next_index] != current.id:
                        finger.insert(l, node_index[next_index])
                        l+=1
                    break
                else:
                    if node_index[0] not in finger and node_index[0] != current.id:
                        finger.insert(l, node_index[0])
                        l+=1


#finds key position in DHT and then stores, updates, or deletes
def storeVal(current, hop, key, value):
    int_hop = int(hop)
    int_hop +=1
    key_id, key_hex = getID(key)
    suc = current.id

    if key_id < current.id and key_id > current.prev:

        suc = current.id 
    elif key_id > current.id and key_id < finger[0]:

        suc = finger[0]
    else:
        if key_id < current.prev:
            suc = current.prev
        for i in range(len(finger)):
            if key_id < finger[i] and key_id > suc:
                if(finger[i] - key_id) < (key_id - suc):
                    suc = finger[i]
            elif key_id < finger[i] and key_id < suc:
                if (finger[i] - key_id) < (suc - key_id):
                    suc = finger[i]
            elif key_id > finger[i] and key_id > suc:
                if (key_id - finger[i]) < (key_id - suc):
                    suc = finger[i]
            elif key_id > finger[i] and key_id < suc:
                if (key_id - finger[i]) < (suc - key_id):
                    suc = finger[i]
    

    if suc == current.id:
        current.data[key] = value
 
        if value == "":
            return (key, "has been deleted") 
        else:
            return ("Number of Hops: " + str(int_hop), 
            "Node Hex Location: " + str(current.hex), 
            "Key ID: " + str(key_hex), 
            "Key: " + str(key), 
            "Value: " + str(value))      
    else:
        sockfd, sa = connectNode(node_list[suc].host, node_list[suc].port)
        msg = (int_hop, "put", key, value)
        buffer = bytes(json.dumps(msg), 'utf-8')
        sockfd.sendto(buffer,sa)
        data = sockfd.recvfrom(4000)
        datas = json.loads(data[0])

        if value == "":
            return (key, "has been deleted")
        return (datas)       


#Locates Node and value returns error message if not present. 
def getVal(current, hop, key):
    int_hop = int(hop)
    int_hop +=1
    key_id, key_hex = getID(key)
    suc = current.id


    if key_id < current.id and key_id > current.prev:

        suc = current.id 
    elif key_id > current.id and key_id < finger[0]:

        suc = finger[0]
    else:
        if key_id < current.prev:
            suc = current.prev
        for i in range(len(finger)):
            if key_id < finger[i] and key_id > suc:
                if(finger[i] - key_id) < (key_id - suc):
                    suc = finger[i]
            elif key_id < finger[i] and key_id < suc:
                if (finger[i] - key_id) < (suc - key_id):
                    suc = finger[i]
            elif key_id > finger[i] and key_id > suc:
                if (key_id - finger[i]) < (key_id - suc):
                    suc = finger[i]
            elif key_id > finger[i] and key_id < suc:
                if (key_id - finger[i]) < (suc - key_id):
                    suc = finger[i]

    if suc == current.id:

        if key not in current.data.keys() or current.data[key] == "":
            return (key, "not present in DHT")
        else:
            return ("Number of Hops: " + str(int_hop), 
            "Node Hex Location: " + str(current.hex), 
            "Key ID: " + str(key_hex), 
            "Key: " + str(key), 
            "Value: " + str(current.data[key]))      
    else:
        sockfd, sa = connectNode(node_list[suc].host, node_list[suc].port)
        msg = (int_hop, "get", key)
        buffer = bytes(json.dumps(msg), 'utf-8')
        sockfd.sendto(buffer,sa)
        data = sockfd.recvfrom(4000)
        datas = json.loads(data[0])
        return datas       
    


def clientReq(current, reqs):
    if reqs[1] == "get":
        results = getVal(current, reqs[0],reqs[2])
        return results
    elif reqs[1] == "put":
        results = storeVal(current, reqs[0],reqs[2],reqs[3])
        return results




def main():
    if (len(sys.argv) < 3):
        sys.exit("ERROR: Node file or line number missing")
    else:
        get_active_nodes(sys.argv[1])
        current, prev = getCurrentNode(sys.argv[2])
        current.prev = prev
        sockfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sockfd.bind((current.host, int(current.port)))
        getFingers(current)
    while True:
        print("DHT Node_ID: ", current.id)
        req, address = sockfd.recvfrom(4000)
        reqs = json.loads(req)
        results = clientReq(current, reqs)
        buffer = bytes(json.dumps(results), 'utf-8')
        sockfd.sendto(buffer,address)

if __name__=="__main__":
    main()
