#ifndef _PADEMW_H
#define _PADEMW_H

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

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

#include <TString.h>
#include <tuple>

//include boost asio library for socket handling and boost array 

#include <boost/asio.hpp>



using boost::asio::ip::udp; 

static const char *menu = "You have the following options:\n (A)rm the trigger\n (D)isarm the Trigger\n (T)rigger Controller\n (R)ead Data\n (S)tatus\n (C)lear\n (Q)uit\n";   


struct manager_data { 
  struct event_base *base; 
  struct event *arm; 
  struct event *disarm; 
  struct event *trigger; 
  struct event *read; 
  struct event *clear; 
  struct event *status; 
  int tcpsock; 
  int udpsock; 
}; 




class padeListener { 
 boost::asio::io_service& _io_service; 
 udp::socket _listener;   
 
 public:
  //  padeListener(); 
 padeListener(boost::asio::io_service &ioservice) : _io_service(ioservice), _listener(ioservice) {}; 
 padeListener(boost::asio::io_service &ioservice, unsigned int port) : _io_service(ioservice), _listener(ioservice, udp::endpoint(udp::v4(), port)) {}; 
 padeListener(const padeListener &listener); 
  bool sendMessage(const TString &msg); 
  
  void Stop(); 
  std::array<char, 1024> _recv; 
  udp::endpoint _remote; 
};

std::tuple<bool, padeListener *> setupListener(const TString &addr, unsigned int port);
  



class padeEventLoop { 
  
  padeListener *_listener; 

 public:
  padeEventLoop() { _listener = NULL; }; 
    
 padeEventLoop(padeListener *listen) : _listener(listen) {}; 
  padeEventLoop(const padeEventLoop &pade); 

}; 




#endif
