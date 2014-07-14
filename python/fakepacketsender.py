import socket 
import time
## quick tool to send packet data spilt by '+++' to a local server for testing 
## Will get modified in the near future into something more useful 


f = open('newpacketdata.bin', 'rb')

packets = f.read().split('+++')
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
host = '127.0.0.1'
port = 21331
s.connect((host,port))
time.sleep(2)
count = 1
for packet in packets: 
    arr = bytearray(packet)
    if len(arr) < 5:
        break

    arr[5] = count
    count += 1
    arr[4] = 0
    print arr
    s.send(arr)
    time.sleep(0.005)
    



