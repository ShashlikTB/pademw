//socket related stuff 
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


#include <iostream>
#include <list> 
#include <vector>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <pademw.h>
#include <controller.h>

#include <gperftools/profiler.h>
#include <assert.h>
#include "TString.h"
//#include "shashlik.h"
//#include "TBEvent.h"

using namespace std; 

//Goals for this version, start an event loop and then bind to udp port 21331 and print received messages 

bool finished = false; 
bool armed = false; 
unsigned int trigger_count = 0; 





void *user_thread(void *argument) { 
  //change over to ncurses when possible
  struct manager_data *manager = (struct manager_data *) argument; 
  
  for (;;) { 
    
    std::cout << menu; 
    std::string command; 
    std::cin >> command; 
    char cmd = command[0]; 
    switch(cmd) { 
    case 'A':
      {
	if (armed) 
	  printf("Already armed\n"); 
	else { 
	  event_add(manager->arm, 0); 
	}
	break; 
      }
    case 'D': {
	if (armed) { 
	  event_add(manager->disarm,0); 
	}
	break; 
    }
    case 'T': {
      if (armed)
	event_add(manager->trigger, 0); 
      else
	printf("Arm the trigger first. \n"); 
      break; 
    }
    case 'R': {
      printf("Reading Data\n"); 
      if (armed)
	printf("Disable arming first\n"); 
      else
	event_add(manager->read, 0); 
      break; 
    }
    case 'S': { 
      printf("Acquiring Status \n"); 
      event_add(manager->status, 0); 
      break; 
    }
    case 'C': {
      printf("Clearing memory\n"); 
      if (armed)
	printf("Disable arming first\n"); 
      else
	event_add(manager->clear, 0); 
      break; 
    }
    case 'Q': { 
      finished = true; 
      event_base_loopexit(manager->base,0); 
      return NULL; 
      break; 
    }
    default: 
      printf("I don't understand\n"); 
      break; 
    }

  }

  return NULL; 
  
}

void tcparm_cb(int x, short y, void *pargs) { 
  const int recbuf = 64; 
  char arming_msg[] = "arm\n"; 
  int sent = send(x, arming_msg, sizeof(arming_msg)-1, 0); 

  //sleep for a millisecond
  //  usleep(1000); 

  unsigned char recvbuf[recbuf]; 
  struct sockaddr_in from; 
  unsigned int addrLen = sizeof(from); 
  int nbytes = 0; 
  bzero(recvbuf, 64); 
  if ((nbytes = recvfrom(x, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&from, &addrLen)) == -1) { 
    printf("Error during receive.\n"); 
  }
  //  printf("Sent: %d bytes, Heard back: %s\n", sent, recvbuf); 
  TString s((char *) recvbuf); 
  s = s.Strip(); 
  if (s.Contains("arm")) { 
    armed = true; 
    printf("Arming successful\n"); 
  }
  else
    printf("Arming failed\n"); 
  
}

void tcpdisarm_cb(int x, short y, void *pargs) { 
  const int recbuf = 64; 

  //  printf("Disarming\n"); 
  char disarming_msg[] = "disarm\n"; 
  int sent = send(x, disarming_msg, sizeof(disarming_msg)-1, 0); 

  unsigned char recvbuf[recbuf]; 
  struct sockaddr_in from; 
  unsigned int addrLen = sizeof(from); 
  int nbytes = 0; 
  bzero(recvbuf, 64); 
  if ((nbytes = recvfrom(x, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&from, &addrLen)) == -1) { 
    printf("Error during receive.\n"); 
  }
  //  printf("Sent: %d bytes, Heard back: %s\n", sent, recvbuf); 
  TString s((char *) recvbuf); 
  s = s.Strip(); 
  if (s.Contains("disarm")) { 
    armed = false; 
    printf("Disarming successful\n"); 
  }
  else
    printf("Disarming failed\n"); 

  
}

void tcptrigger_cb(int x, short y, void *pargs) { 
  const int recbuf = 64; 

  char trigger_msg[] = "trig\n"; 
  int sent = send(x, trigger_msg, sizeof(trigger_msg)-1, 0); 

  unsigned char recvbuf[recbuf]; 
  struct sockaddr_in from; 
  unsigned int addrLen = sizeof(from); 
  int nbytes = 0; 
  bzero(recvbuf, 64); 
  if ((nbytes = recvfrom(x, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&from, &addrLen)) == -1) { 
    printf("Error during receive.\n"); 
  }
  //printf("Sent: %d bytes, Heard back: %s\n", sent, recvbuf); 

  TString s((char *) recvbuf); 
  s = s.Strip(); 
  if (s.Contains("trig")) { 
    printf("Triggering successful\n"); 
  }
  else
    printf("Triggering failed\n"); 

}

void tcpread_cb(int x, short y, void *pargs) { 
  const int recbuf = 64; 

  char read_msg[] = "read all\n"; 
  int sent = send(x, read_msg, sizeof(read_msg)-1, 0); 

  unsigned char recvbuf[recbuf]; 
  bzero(recvbuf, 64); 
  struct sockaddr_in from; 
  unsigned int addrLen = sizeof(from); 
  int nbytes = 0; 

  if ((nbytes = recvfrom(x, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&from, &addrLen)) == -1) { 
    printf("Error during receive.\n"); 
  }
  //printf("Sent: %d bytes, Heard back: %s\n", sent, recvbuf); 
  TString s((char *) recvbuf); 
  s = s.Strip(); 
  if (s.Contains("read")) { 
    printf("Read successful\n"); 
  }
  else
    printf("Read failed\n"); 
  

}


void tcpclear_cb(int x, short y, void *pargs) { 
  const int recbuf = 64; 

  char read_msg[] = "clear\n"; 
  int sent = send(x, read_msg, sizeof(read_msg)-1, 0); 

  unsigned char recvbuf[recbuf]; 
  bzero(recvbuf, 64); 
  struct sockaddr_in from; 
  unsigned int addrLen = sizeof(from); 
  int nbytes = 0; 

  if ((nbytes = recvfrom(x, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&from, &addrLen)) == -1) { 
    printf("Error during receive.\n"); 
  }
  //printf("Sent: %d bytes, Heard back: %s\n", sent, recvbuf); 

  TString s((char *) recvbuf); 
  s = s.Strip(); 
  if (s.Contains("clear")) { 
    printf("Clear successful\n"); 
  }
  else
    printf("Clear failed\n"); 
  

}

void tcpstatus_cb(int x, short y, void *pargs) { 
  const int recbuf = 512; 

  char read_msg[] = "status\n"; 
  int sent = send(x, read_msg, sizeof(read_msg)-1, 0); 

  unsigned char recvbuf[recbuf]; 
  bzero(recvbuf, 512); 
  struct sockaddr_in from; 
  unsigned int addrLen = sizeof(from); 
  int nbytes = 0; 

  if ((nbytes = recvfrom(x, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&from, &addrLen)) == -1) { 
    printf("Error during receive.\n"); 
  }
  //printf("Sent: %d bytes, Heard back: %s\n", sent, recvbuf); 

  
  TString s((char *) recvbuf); 
  printf("Status packet: %s", recvbuf); 
  s = s.Strip(); 
  if (s.Contains("status")) { 
    printf("status check successful\n"); 
  }
  else
    printf("status check failed\n"); 
  

}


void socket_loop(struct event_base *base, struct event *udp,  int sock); 

struct pade_data_packet {
  
  unsigned int pkt_count; 
  unsigned int channel; 
  unsigned int hitCount; 
  std::vector<unsigned char> waveform; 
}; 


struct pade_data_packet *parsePadePacket(unsigned char *packet, int nbytes) { 
  static unsigned int counter = 0; 
  static unsigned int synced = 0; 
  if (!(nbytes == 266 || nbytes == 70)) { 
    std::cout << "Not a pade packet, skipping" << std::endl; 
    return NULL; 
  }

  struct pade_data_packet *pade = new struct pade_data_packet(); 
  pade->pkt_count = packet[4] << 8 | packet[5]; 
  if (counter == 0)
    counter = pade->pkt_count; 

  pade->channel = packet[6]; 
  //  std::cout << "Received a pade packet, packet count:" << pade.pkt_count << " channel:" << pade.channel << std::endl; 

  pade->hitCount = packet[7] << 8 | packet[8]; 
  pade->waveform.reserve(260); 
  if (nbytes > 70) { 
    //Data packet, not checksum packet
    //Nbytes should always be 70 or 266
    for (int i = 10; (i < nbytes); i++) { 
      pade->waveform.push_back(packet[i]); 
    }
  }


  if (counter != pade->pkt_count) {
    std::cout << "Got desynced! Synced for:" << synced << " Rounds.  Resetting for next round" << std::endl; 
    std::cout << "Received a pade packet, packet count:" << pade->pkt_count << " channel:" << pade->channel << std::endl; 
    counter = pade->pkt_count; 
    synced = 0; 
  }
  counter++; 
  synced++; 
  return pade;
 }
  

void printPacket(unsigned char *packet, int nbytes) { 

  for (int i = 0; i < nbytes; i++) { 
    printf("%X\t", packet[i]); 
    if (((i+1) % 10 == 0) && i > 0)
      printf("\n"); 
  }
  printf("\n"); 
}

void tcpmanager_cb(int x, short y, void *pargs) { 
  if ((y & EV_TIMEOUT) != 0)
    return; 
  struct sockaddr_in from; 
  unsigned int addrLen = sizeof(from); 
  int nbytes = 0; 
  unsigned char recvbuf[512]; 
  bzero(recvbuf, 512); 
  if ((nbytes = recvfrom(x, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&from, &addrLen)) == -1) { 
    printf("Error during receive.\n"); 
  }
  printf("Buffer: %s \n", recvbuf); 


}

void udp_cb(int x, short y, void *pargs) { 
  if ((y & EV_TIMEOUT) != 0)
    return; 

  list<pade_data_packet *> *packets = (list<pade_data_packet *> *) pargs; 
  //  printf("UDP Packet Received!\n"); 
  struct sockaddr_in from; 
  unsigned int addrLen = sizeof(from); 
  int nbytes = 0; 
  unsigned char recvbuf[512]; 
  bzero(recvbuf, 512); 
  if ((nbytes = recvfrom(x, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&from, &addrLen)) == -1) { 
    printf("Error during receive.\n"); 
  }
  printf("We got %d Bytes\n", nbytes); 
  printPacket(recvbuf, nbytes); 
  struct pade_data_packet *packet = parsePadePacket(recvbuf, nbytes);
  packets->push_back(packet); 
  if ((packets->size() > 0) && ((packets->size() % 100) == 0)) { 
    printf("Captured %ld packets.\n", packets->size()); 
  }

    
}


int main(int argc, char *argv[]) {
  int sock; 
  

  if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) { 
    std::cout << "Couldn't get socket" << std::endl; 
    exit(EXIT_FAILURE); 
  }

  // Attempt to bind localhost, 21331 
  struct sockaddr_in socketInfo; 
  int port = 21331; 
  bzero(&socketInfo, sizeof(struct sockaddr_in)); 
  socketInfo.sin_family = AF_INET; 
  socketInfo.sin_addr.s_addr=INADDR_ANY; //htonl(INADDR_ANY); //INADDR_ANY; 
  socketInfo.sin_port = htons(port); 

  int optval = 1; 
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(optval)) == -1) { 
    printf("error at setsockopt \n"); 
  }

  //set receive buffer size to max
  int n = 1024 * 1024; 
  if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)) == -1) { 
    printf("error at setsockopt \n"); 
  }


  if ( bind(sock, (struct sockaddr *) &socketInfo, sizeof(struct sockaddr_in)) < 0) { 
    close(sock); 
    std::cout << "Couldn't get socket" << std::endl; 
    exit(EXIT_FAILURE); 
  }


  // Setup tcp socket to connect to control software 
  int tcpsock = 0; 

  bzero(&socketInfo, sizeof(struct sockaddr_in)); 
  if (argc == 1) {
    std::cout << "Please supply an ipaddress or hostname." << std::endl; 
    exit(-1); 
  }
  
  struct addrinfo tcpaddr; 
  struct addrinfo *tcpaddr_ptr, *tcpit; 
  bzero(&tcpaddr, sizeof(struct addrinfo)); 
  tcpaddr.ai_family= AF_INET; 
  tcpaddr.ai_socktype= SOCK_STREAM; 
  tcpaddr.ai_flags = AI_PASSIVE; 
  tcpaddr.ai_protocol = 0; 
  int err = 0; 
  if ((err = getaddrinfo(argv[1], argv[2], &tcpaddr, &tcpaddr_ptr)) != 0) { 
   std::cout << "Bad ipaddress or hostname." << std::endl; 
   printf("Error %d.\n", err); 
    exit(-1); 
  } 
  
  for (tcpit = tcpaddr_ptr; tcpit != NULL; tcpit = tcpit->ai_next) { 
    tcpsock = socket(tcpit->ai_family, tcpit->ai_socktype, tcpit->ai_protocol); 
    if (tcpsock == -1)
      continue; 
  
    if (connect(tcpsock, tcpit->ai_addr, tcpit->ai_addrlen) != -1)
      break; 
    close (tcpsock); 
  }

  freeaddrinfo(tcpaddr_ptr); 

  bool managementMode = true; 
  if (tcpit == NULL) { 
    printf("Couldn't connect to server, continuing in non-management mode.\n"); 
    managementMode = false; 
  }
    
   
 
  struct event_base *base = event_base_new(); 


  struct event *udp; 
  struct event *tcp_arm, *tcp_disarm, *tcp_trigger, *tcp_read, *tcp_clear, *tcp_status; 
  tcp_arm = tcp_disarm = tcp_trigger = tcp_read = tcp_clear = tcp_status = NULL; 
  
  list<pade_data_packet *> packets; 
  udp = event_new(base, sock, EV_READ|EV_PERSIST,udp_cb, &packets); 

  if (managementMode) { 
    tcp_arm = event_new(base, tcpsock, EV_WRITE, tcparm_cb, NULL); 
    tcp_disarm = event_new(base, tcpsock, EV_WRITE, tcpdisarm_cb, NULL); 
    tcp_trigger = event_new(base, tcpsock, EV_WRITE, tcptrigger_cb, NULL); 
    tcp_read = event_new(base, tcpsock, EV_WRITE, tcpread_cb, NULL); 
    tcp_clear = event_new(base, tcpsock, EV_WRITE, tcpclear_cb, NULL); 
    tcp_status = event_new(base, tcpsock, EV_WRITE, tcpstatus_cb, NULL); 

  }
  struct manager_data manage; 
  manage.base = base; 
  manage.arm = tcp_arm; 
  manage.disarm = tcp_disarm; 
  manage.trigger = tcp_trigger; 
  manage.read = tcp_read; 
  manage.clear = tcp_clear; 
  manage.status = tcp_status; 
  manage.tcpsock = tcpsock; 
  manage.udpsock = sock; 


  

  if (!udp) { 
    std::cout << "Couldn't create listener. " << std::endl; 
    close(sock); 
    exit(EXIT_FAILURE); 
  }
  if(event_add(udp, 0) != 0)  {
    std::cout << "Couldn't add event. " << std::endl; 
    close(sock); 
    exit(EXIT_FAILURE); 
  }
  
  printf("Creating user thread.\n"); 
  pthread_t user; 
  int uthread  = pthread_create(&user, NULL, user_thread, (void *) &manage); 
  assert(0 == uthread); 

  socket_loop(base, udp, sock); 
  uthread = pthread_join(user, NULL); 
  printf("Finished with server \n"); 
  event_base_free(base); 
  event_free(udp); 
  close(sock); 
  
  return 0; 
}

void socket_loop(struct event_base *base, struct event *udp, int sock) { 
  struct timeval one_sec; 
  one_sec.tv_sec = 1 ;
  one_sec.tv_usec = 0; 
  event_add(udp,&one_sec); 


  while (!finished) { 
    event_base_dispatch(base);       
    //event_base_loop(base, EVLOOP_NONBLOCK); 
  }


}
