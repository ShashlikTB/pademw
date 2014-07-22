#ifndef _PADESERVER_H
#define _PADESERVER_H


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
  bool running_; 
  udp::socket sock_; 
  udp::endpoint endpoint_; 
  std::array<unsigned char, 270> recv_; 
  unsigned int packetCount_; 
  std::vector<struct padePacket> packets; 
  std::queue< std::array<unsigned char, 270> > unprocessedPackets; 
  void padedataPacketHandler(); 
  void endPacketHandler(); 
  void unknownPacketHandler(); 
  struct padePacket parsePadePacket(const std::array<unsigned char, 270> &array);


 public:

   udpListener(boost::asio::io_service& io_service, unsigned short port) : sock_(io_service, udp::endpoint(udp::v4(), port)) { 
    running_ = false; 
    packetCount_ = 0; 
    recv_.fill(0); 
    }

  std::vector<struct padePacket> & Packets() { return packets; }; 

  void packetHandler(const::boost::system::error_code &ec, std::size_t bytes) {
    unprocessedPackets.push(recv_); 
    struct padePacket pkt = parsePadePacket(recv_); 
    pkt.ts = bytes; 
    packets.push_back(pkt); 
    if (packets.back().pktCount != packetCount_) { 
      std::cout << std::dec << packets.back().pktCount << " Ext Count " << packetCount_ << " Internal, Desynced!" << std::endl; 
      std::cout << "Attempting resync on next event boundary" << std::endl; 
    }
    std::cout << std::dec << "Packet Count: " << packets.back().pktCount << " Channel: " 
	      << packets.back().channel << " Board ID:" << packets.back().boardID << std::endl; 
    packetCount_++; 
  
    recv_.fill(0); 
    if (running_)
      sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&udpListener::packetHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 


  }

  void startListener() { 
    std::cout << "Starting UDP Listener" << std::endl; 
    recv_.fill(0); 
    running_ = true; 
    sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&udpListener::packetHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
  }


  void stopListener() { 
    std::cout << "Stopping Listener" << std::endl; 
    running_ = false; 
    std::cout << unprocessedPackets.size() << " packets" << std::endl; 
    /*    while (!unprocessedPackets.empty()) { 
      for (auto ch : unprocessedPackets.front()) { 
	std::cout << std::hex << ch << " ";
      }
      std::cout << std::endl << std::endl; */
      
  }

}; 


class padeServer { 

  std::array<char, 512> recv_; 
  std::map<std::string, std::function< void() > > callbacks; 
  std::map<unsigned int, shared_ptr<padeBoard> > padeBoards; 

  bool timeout_; 



 public: 
 padeServer(boost::asio::io_service &service, const std::string &address, unsigned short tcpport) : service_(service), sock_(service), 
    endpoint_(tcp::endpoint(boost::asio::ip::address::from_string(address), tcpport)), timer_(service, boost::posix_time::seconds(5)), padeListener_(service, 21331), timeoutLen_(boost::posix_time::seconds (5)) {
    connected_ = false; 
    timeout_ = false; 

  }

  void addPacketsToBoards(std::vector<struct padePacket> &packets) { 
    for (auto packet : packets) { 

      unsigned int boardKey = packet.boardID; 
      unsigned int eventKey = packet.event; 
      if (padeBoards.count(boardKey) > 0) 
	padeBoards[boardKey]->addEvent(eventKey, packet); 

    }


  }


  void padePacketProcessing() { 
    std::cout << "Processing packets" << std::endl; 
    std::vector<struct padePacket> &packets = padeListener_.Packets(); 
    addPacketsToBoards(packets); 

    std::cout << "Board List" << std::endl; 
    for (auto pair : padeBoards) { 
      std::cout << "ID:" << std::get<0>(pair) << " Packets in Events "; 
      std::get<1>(pair)->printEvents(); 
      std::cout << std::endl; 

    }

    
    TFile *f = new TFile("test.root", "RECREATE"); 
    TTree *tree = new TTree("t1041", "T1041"); 
    TBEvent event; 
    TBSpill spill; 
    TBranch *tbevt = tree->Branch("tbevent", "TBEvent", &event, 64000, 0); 
    TBranch *tbspill = tree->Branch("tbspill", "TBSpill", &spill, 64000, 0); 
    timeval tim; 
    gettimeofday(&tim, NULL); 
    spill.SetSpillData(1, tim.tv_usec, 0, 0, 0, 0, -1, -1, 50.0, 50.0); 
    tbspill->Fill(); 
    for (auto pair : padeBoards) { 
      shared_ptr<padeBoard> board (std::get<1>(pair) ); 
      for (std::map<unsigned int, std::vector<struct padePacket> >::iterator it = board->begin(); it != board->end(); ++it) { 
	for (auto pkt : it->second) { 
	  event.FillPadeChannel(tim.tv_usec, pkt.ts, pkt.boardID, pkt.pktCount, pkt.channel, pkt.event, pkt.waveform.data()); 
	}
	std::cout << "Iterator...transfer size:" << (it->second)[0].ts << std::endl;
      }

    }
    tbevt->Fill(); 	
    tree->Write(); 
    f->Write(); 
    f->Close(); 
      

    

    service_.stop(); 

  }

  void readHandler(const::boost::system::error_code &ec, std::size_t bytes) {

    if (!ec) { 
      std::cout << "Read:" << bytes << " bytes. " << std::endl; 
      std::string line(recv_.data()); 
      std::cout << "Data: " << line << std::endl; 
      boost::algorithm::trim(line); 
      recv_.fill(0); 
      if (boost::algorithm::contains(line, "read")) { 
	timer_.expires_from_now(timeoutLen_); 
	//enable pade udp server 
	auto fn = [this](const boost::system::error_code &ec) { timeout_ = true; 
								padeListener_.stopListener(); 
								padePacketProcessing(); 
	}; 
	timer_.async_wait(fn); 
	padeListener_.startListener(); 
      }
	
    }
  }

  void statusHandler(const boost::system::error_code &ec, std::size_t bytes) { 
    if (!ec) { 

      std::cout << "Read:" << bytes << " bytes. " << std::endl; 
      std::string line(recv_.data()); 
      std::cout << "Data: " << line << std::endl; 
      recv_.fill(0); 

      std::vector <std::string> split;
      boost::algorithm::split_regex(split, line, boost::regex("Slave")); 
      if (TString(split[0][0]).Atoi() == split.size()) { 

	for (auto part : split) { 
	  boost::algorithm::trim(part); 
	  std::cout << part << std::endl; 
	  shared_ptr<padeBoard> brd(new padeBoard(part)); 
	  padeBoards[brd->id()] = brd; 
	  std::cout << std::dec << "Found a board: " << brd->id() << std::endl; 
	}
      }   
      if (!padeBoards.empty()) { 
	std::string msg = "read\r\n"; 
	auto fn = [this, msg](const::boost::system::error_code &ec, std::size_t bytes) { 
	  if (!ec) { std::cout << "Read: " << bytes << " bytes" << std::endl; 
		     sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::readHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
	  }
	}; 
	
	sock_.async_write_some(boost::asio::buffer(msg), fn); 	


      }
    }
  }

  void clearHandler(const::boost::system::error_code &ec, std::size_t bytes) { 
    if (!ec) { 
      std::cout << "Read:" << bytes << " bytes. " << std::endl; 
      std::string line(recv_.data()); 
      recv_.fill(0); 
      boost::algorithm::trim(line); 
      if (boost::algorithm::contains(line, "clear")) { 
	connected_ = true; 
	std::string msg("status\r\n"); 

	//If you haven't used c++11's lambda form this may be a bit confusing 
	//This binds an anonymous function to auto fn, capturing the this variable and the status msg
	//into a callback used by boost after data has been written to a socket
	auto fn = [this, msg](const::boost::system::error_code &ec, std::size_t bytes) { 
	  if (!ec) { std::cout << "Read: " << bytes << " bytes" << std::endl; 
		     sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::statusHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
	  }
	}; 
	sock_.async_write_some(boost::asio::buffer(std::string("status\r\n")), fn); 

      }
    }
  }


  void writeHandler(const::boost::system::error_code &ec, std::size_t bytes) { 
    if (!ec) { 
      std::cout << "Wrote: " << bytes << " bytes. " << std::endl; 
      
    }
  }

  void connectHandler(const boost::system::error_code &ec) { 
    if (!ec) { 
      std::cout << "Connecting" << std::endl; 
      sock_.async_write_some(boost::asio::buffer(std::string("clear\r\n")), boost::bind(&padeServer::writeHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 

      sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::clearHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
    }

  }


  void connect() { 
    sock_.async_connect(endpoint_, boost::bind(&padeServer::connectHandler, this, boost::asio::placeholders::error)); 
  }

 private:
  boost::asio::io_service &service_; 
  tcp::socket sock_; 
  tcp::endpoint endpoint_; 
  boost::asio::deadline_timer timer_; 
  udpListener padeListener_; 
  boost::posix_time::seconds timeoutLen_; 
  bool connected_; 

}; 

#endif 
