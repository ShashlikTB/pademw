import zmq

port = "5001"
context = zmq.Context()
print "Connecting to server..."
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:%s" % port)

reqs = ["time", "rd 8", "rd b", "rd 0", "time"]

for req in range(1,5):
  print "Sending request: ", req
  socket.send(reqs[req])
  message = socket.recv()
  print "Received reply: ", message
