import socket
import time 
import random 
import math
### Fake Pade Management Computer 



def generatePadePacket(bID, count, channel, event): 
    arr = bytearray(266)
    arr[0] = 1
    arr[2] = int(bID, 16)
    arr[4] = (count & 0xFF00) >> 8
    arr[5] = (count & 0x00FF)
    arr[6] = channel
    arr[7] = (event & 0xFF00) >> 8
    arr[8] = (event & 0x00FF)
    #generate data part of packet
    for i in range(10, 266): 
        arr[i] = 50+(int(round(15*math.sin(channel*5+i/20))))
        
    return arr


def generateEndPacket(boardID, count): 
    endPacket = bytearray('\x00N%\x00\x04A\x124Vx%\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00 \x08\x00\x00\xa2\x00\x00\x00\x87eC!\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff')
    endPacket[2] = int(boardID, 16)
    endPacket[4] = (count & 0xFF00) >> 8
    endPacket[5] = (count & 0x00FF)

    return endPacket

class serverResponder: 
    
    


    def __init__(self, boards, events): 
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
        
        self.events = []
        packets = [] 
        for ev in range(0, events): 
            for i in range(0, len(self.pades)): 
                for j in range(0, 32): 
                    pack = generatePadePacket(self.pades[i].bid, self.pades[i].pktcount, j, ev)
                    packets.append(pack)
                    self.pades[i].pktcount += 1
                packets.append(generateEndPacket(self.pades[i].bid, self.pades[i].pktcount))
                self.pades[i].pktcount += 1
            self.events.append(packets)
            packets = []
                            


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
        

    def sendfakePadePackets(self, ev): 
        host = '127.0.0.1'
        port = 21331
        self.udpSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


        self.udpSocket.connect((host, port))
        try: 
#            print "event %s" % ev
            event = self.events[ev]
            for packet in event: 
                self.udpSocket.send(packet)
#                time.sleep(0.0001)
        except Exception as e:
            print e
                



    def lookupAction(self, msg): 
        print "action: %s" % msg.strip().split(' ')[0]
        return self.actions[msg.strip().split(' ')[0]](msg)





class padeBoard: 

    trigger = 21
    def __init__(self, padeType): 
        self.stat = hex(random.randint(0,9999))[2:]
        self.bid = hex(random.randint(10,100))[2:]
        self.type = padeType
        self.arm = hex(0)[2:]
        self.pktcount = 0
        if (padeType is 'Master'):
            # Want all of the triggers to match
            padeBoard.trigger = hex(random.randint(0,1000))[2:]

        self.errReg = hex(random.randint(0,9999))[2:]
        self.lastTrig = hex(0)[2:]
        self.ptemp = hex(random.randint(0,9999))[2:]
        self.stemp = hex(random.randint(0,9999))[2:]

            

    def status(self): 
        return "%s %s %s %s %s %s %s %s %s" % (self.type, self.bid, self.stat, self.arm,
                                            padeBoard.trigger, self.errReg, self.lastTrig, 
                                            self.ptemp, self.stemp)





def server(conn, addr): 
    pades = [] 
    pades.append(padeBoard('Master'))
    for i in range (0,3): 
        pades.append(padeBoard('Slave'))

    responder = serverResponder(pades, 301)
    i = 0
    while 1: 
        print 'starting send loop, i:%s, packetCount: %s' % (i, responder.packetCount)
        if (i > 300):
            break

        data = conn.recv(256)
        if not data: 
            print "no data"
            break
        try:
            if (len(data.strip()) > 0): 
                print "We received: %s" % data.strip()
                action = responder.lookupAction(data)+'\r\n'
                conn.sendall(action)
                if action.find('read') != -1: 
                    i += 1
                    eventNumber = int(data.strip().split(' ')[1], 10)
                    responder.sendfakePadePackets(eventNumber)
                    for pade in responder.pades:
                        pade.lastTrig = hex(eventNumber)

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

    



