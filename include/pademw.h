#ifndef  _PADEMW_H
#define _PADEMW_H

#include <memory> 
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "TString.h"
#include <string>
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
  std::vector<struct padePacket> *getPackets() { return &packets; }; 

}; 

class padeControlClient {
  tcp::socket sock_; 
  tcp::resolver resolver_; 
  tcp::resolver::query query_; 
  std::array<char, 25> recv_; 
  
  void rwloop() { 
    boost::system::error_code ec; 
    size_t len = sock_.read_some(boost::asio::buffer(recv_), ec); 
    if (ec == boost::asio::error::eof) { 
      std::cout << "Lost connection." << std::endl;
      return; 
    }
    else if(ec) { 
      throw boost::system::system_error(ec); 
    }

    std::cout << "Data from server:" << std::string(recv_.data()); 
    recv_.fill(0); 
    std::string msg("Greetings client!\n"); 
    boost::asio::write(sock_, boost::asio::buffer(msg), ec); 
    rwloop(); 


  }

 public:
 padeControlClient(boost::asio::io_service &io_service, const std::string &address, const std::string &service) : sock_(io_service), resolver_(io_service), query_(address, service) { 

  }

  bool connect() { 
    tcp::resolver::iterator endpoint_it  = resolver_.resolve(query_); 
    boost::asio::connect(sock_, endpoint_it); 
    rwloop(); 
    return true; 
  }
  
  
}; 


#endif 
