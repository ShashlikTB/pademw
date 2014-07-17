#ifndef  _PADEUDPSERVER_H
#define _PADEUDPSERVER_H

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


struct padePacket { 
  unsigned int pktCount; 
  unsigned int channel; 
  unsigned int hitCount; 
  std::vector<int> waveform; 
};



class padeUDPServer { 

  udp::socket sock_; 
  udp::endpoint remote_endpoint_; 
  std::array<unsigned char, 270> recv_; 
  bool finishedFlag_; 
  unsigned int packetCount_; 
  std::vector<struct padePacket> packets; 


 public: 
 padeUDPServer(boost::asio::io_service& io_service, unsigned short port) : sock_(io_service, udp::endpoint(udp::v4(), port)) { 
    finishedFlag_ = false; 
    packetCount_ = 0; 
    recv_.fill(0); 
    receive_loop(); 
  }

  struct padePacket parsePadePacket(const std::array<unsigned char, 270> &array); 

  void receive_loop(); 
  void handle_receive(const boost::system::error_code &ec, std::size_t bytes); 
  bool finished() { return finishedFlag_; }; 
  bool send_bytes(const TString &msg); 
  void setpacketCount(unsigned int count) { packetCount_ = count; }; 
  unsigned int packetCount() { return packetCount_; }; 
  std::vector < struct padePacket > *getPackets() { return &packets; }; 

}; 


#endif 
