pademw
======

Middleware that talks to the control software of the PADE boards and receives data from the PADE boards

to build do something along the lines of 
mkdir build
cd build
cmake ..
make standalone 
I also have a testing make path, but it's not very updated at the moment. 

General overview of the file structure: 

src/
	main.cpp -- Testing Main
	standalone.cpp	 -- Standalone Main
	padeUDPServer.cpp -- UDP Server code, listening and parsing PADE Packets 
	pademworignal.cpp -- original c/c++ server code not used
	padeClient.cpp -- TCP Client that connects to the PADE control software
	padeBoard.cpp -- some misc. class stuff 
	
python/
	fakeManagement.py -- generates and supplies fake PADE status lines and fake PADE UDP packets
	fakepacketsender.py -- send packets from a binary file of packets separated by '+++'
	

include/
	padeBoard.h -- header and class def for padeBoard class
	padeClient.h -- header and class def for the TCP Client
	padeUDPServer.h -- header and class def for the UDP Server 
	shashlik.h -- some currently unused code for the Testbeam, will get used soon
	TBEvent.h -- similar to above 

/rubinov.cs -- c# test file of some of Paul Rubinov's code 

Currently Implemented -- 

Status message parsing 

UDP packet parsing 
TCP Client and UDP Server 
Simple test loop 
Fake PADE control server and fake packet sender 

Next Steps: 
Simple event loop that will write data from events to a ROOT TTree based on status 
messages and UDP packets 



