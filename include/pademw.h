#ifndef  _PADEMW_H
#define _PADEMW_H

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


class padeBoard { 
  unsigned int boardid_; 
  unsigned int ptemp_; 
  unsigned int stemp_; 
  unsigned int triggerInMem_; 
  unsigned int lastTrigger_; 
  unsigned int statusReg_; 
  unsigned int errorReg_; 
  bool armed_;   
  bool master_; 

 public: 
  padeBoard(const std::string &msg) {

    std::vector <std::string> split; 
    boost::algorithm::split(split, msg, boost::algorithm::is_any_of(" ")); 


    std::vector<std::string>::iterator it = split.begin(); 
    if (boost::algorithm::contains(msg, "Master")) {
      //Master has 2 extra entries
      it += 2; 
      master_ = true; 
    }
    // Layout should be as follows 
    // # BoardId statusRegister Armed #Triggers Error Register LastTrigger PadeTemp SipmTemp
      
    boardid_ = strtol((*it).data(), NULL, 16); 
      ++it; 
      statusReg_ = strtol((*it).data(), NULL, 16); 
      ++it; 
      long int tmp = strtol((*it).data(), NULL, 16); 
      if (tmp == 1)
	armed_ = true; 
      ++it; 
      triggerInMem_ = strtol((*it).data(), NULL, 16); 
      ++it; 
      errorReg_ = strtol((*it).data(), NULL, 16); 
      ++it; 
      lastTrigger_ = strtol((*it).data(), NULL, 16); 
      ++it; 
      ptemp_ = strtol((*it).data(), NULL, 16); 
      ++it; 
      stemp_ = strtol((*it).data(), NULL, 16); 

  }



  
  unsigned int id() { return boardid_; }; 
  unsigned int padeTemperature() { return ptemp_; }; 
  unsigned int sipmTemperature() { return stemp_; }; 
  unsigned int availableTriggers() { return triggerInMem_; }; 
  unsigned int lastTrigger() { return lastTrigger_; }; 
  unsigned int statusRegister() { return statusReg_; };
  unsigned int errorRegister() { return errorReg_; }; 
  bool armed() { return armed_; };
  bool isMaster() { return master_; }; 



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

class padeControlClient {

  bool armed_; 
  bool active_; 
  tcp::socket sock_; 
  tcp::resolver resolver_; 
  tcp::resolver::query query_; 
  std::array<char, 512> recv_; 
  std::map<std::string, std::function< void() > > callbacks; 

  std::map<unsigned int, shared_ptr<padeBoard> > padeBoards; 

  
  void armCB() { 
    std::cout << "Arming" << std::endl; 
    armed_ = true; 
  }

  void disarmCB() { 
    std::cout << "Disarming" << std::endl; 
    armed_ = false; 
  }
   
  void statusCB() { 
    std::cout << "Getting Status" << std::endl; 
  }

  void trigCB() { 
    std::cout << "Triggering" << std::endl; 
  }

  void clearCB() { 
    active_ = true; 
  }

  void readCB() { 
    std::cout << "UDP Packets" << std::endl; 
  }

  void statusLoop() { 
    bool finished = false; 
    std::stringstream statusMsg; 
    while (!finished) { 
      boost::system::error_code ec; 
      size_t len = sock_.read_some(boost::asio::buffer(recv_), ec); 
      if (ec == boost::asio::error::eof) { 
	std::cout << "Lost connection." << std::endl;
	return; 
      }
      else if(ec) { 
	throw boost::system::system_error(ec); 
      }
      std::cout << "Received:" << len << " Bytes." << std::endl; 
      std::string statusPart(recv_.data()); 
      recv_.fill(0); 
      finished = boost::algorithm::contains(statusPart, "\r\n"); 
      boost::algorithm::trim(statusPart); 
      statusMsg << statusPart; 

      //Note you could get the hex val: 0 added  to the statusMsg if the status msg is greater than the buffer
      //this will get fixed eventually. 
      std::cout << "Status Message:" << statusPart << std::endl; 
    }

    std::vector <std::string> split;
    std::string toSplit(statusMsg.str()); 
    boost::algorithm::split_regex(split, toSplit, boost::regex("Slave")); 
    std::cout << TString(split[0][0]).Atoi() << " " << split.size() << std::endl; 
    if (TString(split[0][0]).Atoi() == split.size()) { 

      for (auto part : split) { 
	boost::algorithm::trim(part); 

	shared_ptr<padeBoard> board(new padeBoard(part)); 
	padeBoards[board->id()] = board; 
	std::vector <std::string> statusParts; 
	boost::algorithm::split(statusParts, part, boost::algorithm::is_any_of(" ")); 
	std::cout << part << std::endl; 
	for (auto piece : statusParts) { 
	  TString conversion(piece); 
	  if (conversion.IsHex()) {
	    long int c = strtol(conversion.Data(), NULL, 16); 
	    std::cout << std::dec << c << " " << std::hex << c << std::dec << std::endl; 
	  }
	}
	std::cout << std::endl; 
      }
    }
  }

  void recvLoop() { 
    boost::system::error_code ec; 
    size_t len = sock_.read_some(boost::asio::buffer(recv_), ec); 
    if (ec == boost::asio::error::eof) { 
      std::cout << "Lost connection." << std::endl;
      return; 
    }
    else if(ec) { 
      throw boost::system::system_error(ec); 
    }
    std::cout << "Received:" << len << " Bytes." << std::endl; 
    std::string key(recv_.data()); 
    boost::algorithm::trim(key); 
    recv_.fill(0); 
    if (callbacks.count(key) > 0) {
      //      std::function< void() > fn = callbacks[std::string(data.Strip().Data())]; 

      auto fn = callbacks[key]; 
      fn(); 
    }
    else 
      std::cout << "Couldn't find action:" << key << std::endl; 



  }

 public:

 padeControlClient(boost::asio::io_service &io_service, const std::string &address, const std::string &service) : sock_(io_service), resolver_(io_service), query_(address, service) { 
    callbacks[std::string("arm")] = std::bind(&padeControlClient::armCB, this); 
    callbacks[std::string("disarm")] = std::bind(&padeControlClient::disarmCB, this); 
    callbacks[std::string("clear")] = std::bind(&padeControlClient::clearCB, this); 
    callbacks[std::string("trig")] = std::bind(&padeControlClient::trigCB, this); 
    callbacks[std::string("read")] = std::bind(&padeControlClient::readCB, this); 
    active_ = false; 
  }

  bool armed() { return armed_; };

  bool active() { return active_; }; 

  bool connect() { 
    tcp::resolver::iterator endpoint_it  = resolver_.resolve(query_); 
    boost::asio::connect(sock_, endpoint_it); 
    return true; 
  }

  void disconnect() { 
    sock_.close(); 
    active_ = false; 
  }

  void arm() { 
    boost::system::error_code ec; 
    boost::asio::write(sock_, boost::asio::buffer(std::string("arm\r\n")), ec); 
    recvLoop(); 

  }

  void disarm() { 
    boost::system::error_code ec; 
    boost::asio::write(sock_, boost::asio::buffer(std::string("disarm\r\n")), ec); 

  }

  void clear() { 
    boost::system::error_code ec; 
    boost::asio::write(sock_, boost::asio::buffer(std::string("clear\r\n")), ec); 
    recvLoop(); 
  }
    
  void trig() { 

  }

  void read() { 
    boost::system::error_code ec; 
    boost::asio::write(sock_, boost::asio::buffer(std::string("read 1\r\n")), ec); 
  }

  void status() { 
    boost::system::error_code ec; 
    boost::asio::write(sock_, boost::asio::buffer(std::string("status\r\n")), ec); 
    statusLoop(); 
  }

  shared_ptr<padeBoard> getBoard( unsigned int id ) { 
    if  (padeBoards.count(id) > 0)
      return padeBoards[id]; 
    return shared_ptr<padeBoard>(); 
  }

  std::vector<unsigned int> getIDs() { 
    std::vector<unsigned int> ids; 
    for ( auto board : padeBoards ) { 
      ids.push_back(board.first); 
    }
    return ids; 
  }

}; 
  
  


#endif 
