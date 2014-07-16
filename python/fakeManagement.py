import socket
import time 
import random 

### Fake Pade Management Computer 



def generatePadePacket(count, channel): 
    arr = bytearray(266)
    arr[0] = 1
    arr[4] = count & 0x00FF
    arr[5] = (count & 0xFF00)>> 8
    arr[6] = channel
    arr[7] = 1
    arr[8] = 1
    #generate data part of packet
    for i in range(10, 266): 
        arr[i] = random.randint(0, 0xff)
        
    return arr




class serverResponder: 
    
    endPacket = '\x00N%\x00\x04A\x124Vx%\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00 \x08\x00\x00\xa2\x00\x00\x00\x87eC!\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'


    def __init__(self, boards): 
        self.actions = { 
            'arm': self.arm,
            'disarm': self.disarm,
            'trig': self.trig,
            'read': self.read,
            'clear': self.clear,
            'status': self.status,
            }
        self.pades = boards
        self.packetCount = 0

    def arm(self, msg): 
        print 'Arming'
        return 'arm'

    def disarm(self, msg): 
        print 'Disarming'
        return 'disarm'

    def trig(self, msg):
        print 'Triggering'
        return 'trig'

    def read(self, msg): 
        print 'reading'
        return 'read'

    def clear(self, msg):
        print 'clearing'
        return 'clear'

    def status(self, msg): 
        print 'status'
        npades = str(len(self.pades))
        padeStatus = [pade.status() for pade in self.pades]
        return "%s %s" % (npades, ' '.join(padeStatus))
        

    def sendfakePadePackets(self): 
        host = '127.0.0.1'
        port = 21331
        self.udpSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udpSocket.connect((host, port))
        channel = 0
        time.sleep(0.01)
        for i in range(0, len(self.pades)): 
            for i in range(0, 32): 
                pack = generatePadePacket(self.packetCount, channel)
                print self.packetCount
                print pack
                self.packetCount += 1
                self.udpSocket.sendall(pack)
                time.sleep(0.009)
            self.udpSocket.send(self.endPacket)




    def lookupAction(self, msg): 
        return self.actions[msg.strip().split(' ')[0]](msg)





class padeBoard: 

    def __init__(self, padeType): 
        self.stat = hex(random.randint(0,9999))[2:]
        self.bid = random.randint(0,300)
        self.type = padeType
        self.arm = hex(0)[2:]
        self.trigger = hex(random.randint(0,1000))[2:]
        self.errReg = hex(random.randint(0,9999))[2:]
        self.lastTrig = hex(random.randint(0,9999))[2:]
        self.ptemp = hex(random.randint(0,9999))[2:]
        self.stemp = hex(random.randint(0,9999))[2:]

            

    def status(self): 
        return "%s %s %s %s %s %s %s %s %s" % (self.type, self.bid, self.stat, self.arm,
                                            self.trigger, self.errReg, self.lastTrig, 
                                            self.ptemp, self.stemp)





def server(conn, addr): 
    pades = [] 
    pades.append(padeBoard('Master'))
    for i in range (0,2): 
        pades.append(padeBoard('Slave'))

    responder = serverResponder(pades)
    while 1: 
        data = conn.recv(1024)
        if not data: break
        try:
            if (len(data.strip()) > 0): 
                print "We received: %s" % data.strip()
                action = responder.lookupAction(data)+'\r\n'
                conn.sendall(action)
                if action.find('read') != -1: 
                    responder.sendfakePadePackets()

        except Exception as e: 
            print e
            conn.sendall("Bad Command\r\n")





host = '127.0.0.1'
udpPort = 21331
fakeServer = 23

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((host, fakeServer))
s.listen(1)
conn, addr = s.accept()
print 'Connected by:', addr
server(conn, addr)

conn.close()

    



