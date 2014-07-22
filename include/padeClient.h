#ifndef _PADECLIENT_H
#define _PADECLIENT_H


#include <memory> 
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>
#include "TString.h"
#include <string>
#include <functional>
using boost::asio::ip::udp; 
using boost::asio::ip::tcp; 
using std::shared_ptr; 
#include "padeBoard.h"




class padeControlClient {

  bool armed_; 
  bool active_; 
  bool readReady_; 
  tcp::socket sock_; 
  tcp::resolver resolver_; 
  tcp::resolver::query query_; 
  std::array<char, 512> recv_; 
  std::map<std::string, std::function< void() > > callbacks; 

  std::map<unsigned int, shared_ptr<padeBoard> > padeBoards; 

  
  void armCB(); 
  void disarmCB(); 
  void statusCB(); 
  void trigCB(); 
  void clearCB(); 
  void readCB(); 
  void statusLoop(); 
  void recvLoop();



 public:

  padeControlClient(boost::asio::io_service &io_service, const std::string &address, const std::string &service);

  bool armed() { return armed_; };
  bool active() { return active_; }; 
  bool readReady() { return readReady_; }; 

  bool connect();
  void disconnect();


  //commands
  void arm(); 
  void disarm();

  void clear();
  void trig(); 
  void read();

  void status();
  shared_ptr<padeBoard> getBoard( unsigned int id );

  std::vector<unsigned int> getIDs();

}; 
  
  


#endif 
