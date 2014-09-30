import zmq
import time

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
             'time':genTime, }

def wcServer(socket):
  while 1:
    
    data = socket.recv()
    print "Received data: ", data
    time.sleep(1)

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
 
        time.sleep(1)
        socket.send(response)


    except KeyError as err: 
        response = "Command %s not found \r\n" % split[0]
        print 'command not found'
    except Exception as e:
        print e 


port = "5001"
context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:%s" % port)

wcServer(socket)

