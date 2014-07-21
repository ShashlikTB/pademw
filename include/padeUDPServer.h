#ifndef  _PADEUDPSERVER_H
#define _PADEUDPSERVER_H

#include <memory> 
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/pending/queue.hpp>


#include "TString.h"
#include <string>
#include <functional>
using boost::asio::ip::udp; 
using boost::asio::ip::tcp; 
using boost::pending::queue;
using std::shared_ptr; 




struct padePacket { 
  unsigned int pktCount; 
  unsigned int channel; 
  unsigned int hitCount; 
  unsigned int boardID; 
  std::vector<int> waveform; 
};


struct event { 
  std::vector<padePacket> eventPackets; 
  boost::posix_time::ptime ts; 

event(std::vector<struct padePacket> packets) : eventPackets(packets), ts(boost::date_time::microsec_clock<boost::posix_time::ptime>::universal_time()) { }; 

}; 



class padeUDPServer { 

  queue<struct padePacket> padePackets_; 
  udp::socket sock_; 
  udp::endpoint remote_endpoint_; 
  std::array<unsigned char, 270> recv_; 
  bool finishedFlag_; 
  unsigned int packetCount_; 
  std::vector<struct padePacket> packets; 
  std::vector<struct event> events; 

  void padedataPacketHandler(); 
  void endPacketHandler(); 
  void unknownPacketHandler(); 



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
  std::vector < struct event > &Events() { return events; }; 
}; 


#endif 
