#ifndef  _PADEMW_H
#define _PADEMW_H

#include <memory> 
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "TString.h"
#include <string>
using boost::asio::ip::udp; 
using std::shared_ptr; 


struct padePacket { 
  unsigned int pktCount; 
  unsigned int channel; 
  unsigned int hitCount; 
  std::vector<unsigned char> waveform; 
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

}; 




#endif 
