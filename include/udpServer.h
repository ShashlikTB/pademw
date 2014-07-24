#ifndef _UDPSERVER_H
#define _UDPSERVER_H


#include <memory> 
#include <cstdlib>
#include <sys/time.h>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>
#include <queue>
#include "TString.h"
#include "TBEvent.h"
#include "TTree.h"
#include "TFile.h"
#include <string>
#include <functional>
using boost::asio::ip::udp; 
using boost::asio::ip::tcp; 
using std::shared_ptr; 
#include "padeBoard.h"



class udpListener { 
  unsigned int anticipatedPackets_; 
  bool running_; 
  bool desynced_;
  udp::socket sock_; 
  udp::endpoint endpoint_; 
  std::function<void ()> tcpCB_; 
  std::array<unsigned char, 270> recv_; 
  unsigned int packetCount_; 
  std::vector<struct padePacket> packets; 
  std::queue< std::array<unsigned char, 270> > unprocessedPackets; 
  std::map<unsigned int, unsigned int> boardPacketCount; 

  void padedataPacketHandler(); 
  void endPacketHandler(); 
  void unknownPacketHandler(); 


  struct padePacket parsePadePacket(const std::array<unsigned char, 270> &array);


 public:

 udpListener(boost::asio::io_service& io_service, unsigned short port) : 
  sock_(io_service, udp::endpoint(udp::v4(), port)), tcpCB_(std::nullptr_t())
    { 
      running_ = false; 
      packetCount_ = 0; 
      anticipatedPackets_ = 0; 
      recv_.fill(0); 
    }

  std::vector<struct padePacket> & Packets() { return packets; }; 
  void clearBoardCount(unsigned int boardID) { 
    boardPacketCount[boardID] = 0; 
  }

  void setBoardCount(unsigned int boardID, unsigned int count) { 
    boardPacketCount[boardID] = count; 
  }

  unsigned int boardCount(unsigned int boardID) { 
    return boardPacketCount[boardID]; 
  }

  void packetHandler(const::boost::system::error_code &ec, std::size_t bytes);


  void startListener(unsigned int anticipatedPackets); 


  void stopListener();
  bool desynced() { return desynced_; }; 
  void resetSync() { desynced_ = false; }; 
  bool running() { return running_; }; 
  void setCallback(std::function<void ()> fn) { tcpCB_ = fn; }; 

}; 







#endif 

