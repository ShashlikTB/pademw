import socket
import time 
import random 
import math


#Fake Wire Chamber Controller

def genRdRead(val): 
    if val.find('b') != -1: 
        return str(8)
    elif val.find('5') != -1:
        return str(0x10)
    else:
        return 0x0
    


def genTime(): 
    return str('1-1-1900')


commands = { 'rd':genRdRead,
             'time':genTime,}


             


def wcServer(conn, addr): 

    while 1: 

        data = conn.recv(25)

        if not data: 
            print "Lost connection" 
            break
    

        try: 
            if (len(data.strip()) > 0): 
                print "%s" % data.strip()
            split = data.strip().split(' ')
            fn = commands[split[0]]
            if len(split) > 1: 
                response = "%s\r\n" % fn(split[1])
            else:
                response = "%s\r\n" % fn()
            
            conn.sendall(response)


        except KeyError as err: 
            response = "Command %s not found \r\n" % split[0]
            print 'command not found'
            conn.sendall(response)
        except Exception as e:
            print e 
        


host = '127.0.0.1'
fakeServer = 5001




s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((host, fakeServer))
s.listen(1)
conn, addr = s.accept()
print 'Connected by:', addr
wcServer(conn, addr)

conn.close()
