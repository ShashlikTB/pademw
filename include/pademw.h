#ifndef _PADEMW_H
#define _PADEMW_H


#include <TString.h>
#include <tuple>

//include boost asio library for socket handling and boost array 

#include <boost/asio.hpp>
#include <boost/thread.hpp>

using std::shared_ptr; 
using boost::asio::ip::udp; 

static const char *menu = "You have the following options:\n (A)rm the trigger\n (D)isarm the Trigger\n (T)rigger Controller\n (R)ead Data\n (S)tatus\n (C)lear\n (Q)uit\n";   



class padeListener { 
 boost::asio::io_service& _io_service; 
 shared_ptr<udp::socket> _listener;   
 std::array<char, 1024> recvBuffer; 
 std::array<char, 128> sndBuffer; 
 
 public:
  //  padeListener(); 
 padeListener(boost::asio::io_service &ioservice) : _io_service(ioservice), _listener(shared_ptr<udp::socket>(new udp::socket(ioservice))) { 
   boost::asio::socket_base::reuse_address option(true); 
   _listener->set_option(option); 
 }
 padeListener(boost::asio::io_service &ioservice, unsigned int port) : _io_service(ioservice), _listener(shared_ptr<udp::socket>(new udp::socket(ioservice, udp::endpoint(udp::v4(), port)))) {
   boost::asio::socket_base::reuse_address option(true); 
   _listener->set_option(option); 
 }
 padeListener(const padeListener &listener) : _io_service(listener._io_service), _listener(listener._listener) {}; 

 ~padeListener(); 
 void operator()(); 
 bool simplemanagementLoop(); 
  bool sendMessage(const TString &msg); 
  bool Running(); 
  void Stop(); 
  std::array<char, 1024> _recv; 
  udp::endpoint _remote; 
};

std::tuple<bool, padeListener *> setupListener(const TString &addr, unsigned int port, boost::asio::io_service &io_service);
  







#endif
