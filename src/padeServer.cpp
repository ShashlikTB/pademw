#include "padeServer.h"
#include "udpServer.h"


void padeServer::addPacketsToBoards(std::vector<struct padePacket> &packets) { 
    for (auto packet : packets) { 

      unsigned int boardKey = packet.boardID; 
      unsigned int eventKey = packet.event; 
      if (padeBoards.count(boardKey) > 0) 
	padeBoards[boardKey]->addEvent(eventKey, packet); 

    }


}


struct padePacket udpListener::parsePadePacket(const std::array<unsigned char, 270> &array) { 
  struct padePacket pkt; 
  pkt.boardID = array[2]; 
  pkt.pktCount = array[4] << 8 | array[5]; 
  pkt.channel = array[6]; 
  pkt.event = array[7] << 8 | array[8]; 
  if (array.size() > 70) { 
    pkt.waveform.reserve(60); 
    int adc = 0; 
    for (unsigned int i = 0; i < 60; i++ ) { 
      //ADC position calculation pulled from Paul's C# Code 
      adc = array[17+4*i] * 256 + array[16+4*i]; 
      pkt.waveform.push_back(adc); 
      adc = array[15+4*i]*256 + array[14+4*i]; 
      pkt.waveform.push_back(adc); 
      adc = 0; 
    }
    pkt.waveform.shrink_to_fit(); 

  }
  return pkt; 
};


void padeServer::padePacketProcessing() { 
    std::cout << "Processing packets" << std::endl; 
    std::vector<struct padePacket> &packets = padeListener_.Packets(); 
    addPacketsToBoards(packets); 

    /*std::cout << "Board List" << std::endl; 
    for (auto pair : padeBoards) { 
      std::cout << "ID:" << std::get<0>(pair) << " Packets in Events "; 
      std::get<1>(pair)->printEvents(); 
      std::cout << std::endl; 
      } */

    

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
      }

    }
    tbevt->Fill(); 	
    tree->Write(); 
    f_.Write(); 



    padeBoards.clear(); 


  }


void padeServer::readHandler(const::boost::system::error_code &ec, std::size_t bytes) {

    if (!ec) { 
      //      std::cout << "Read:" << bytes << " bytes. " << std::endl; 
      std::string line(recv_.data()); 
      //      std::cout << "Data: " << line << std::endl; 
      boost::algorithm::trim(line); 
      recv_.fill(0); 
      if (boost::algorithm::contains(line, "read")) { 
	timer_.expires_from_now(timeoutLen_); 
	//enable pade udp server 
	auto fn = [this](const boost::system::error_code &ec) { 
	  if (!ec) { 
	    std::cout << "Timeout Waiting for Packets" << std::endl; 
	    timeout_ = true; 
	    padeListener_.stopListener(); 
	    padePacketProcessing(); 
	  }
	}; 
	timer_.async_wait(fn); 
	padeListener_.startListener(anticipatedPackets_); 
      }
	
    }
  }


void padeServer::statusHandler(const boost::system::error_code &ec, std::size_t bytes) { 
    if (!ec) { 
      
      //      std::cout << "Read:" << bytes << " bytes. " << std::endl; 
      std::string line(recv_.data()); 
      //      std::cout << "Data: " << line << std::endl; 
      recv_.fill(0); 

      std::vector <std::string> split;
      boost::algorithm::split_regex(split, line, boost::regex("Slave")); 
      if (TString(split[0][0]).Atoi() == split.size()) { 
	anticipatedPackets_ = 0; 
	for (auto part : split) { 
	  boost::algorithm::trim(part); 
	  //	  std::cout << part << std::endl; 
	  shared_ptr<padeBoard> brd(new padeBoard(part)); 
	  padeBoards[brd->id()] = brd; 
	  anticipatedPackets_ += brd->lastTrigger()*32+1; 
	  std::cout << std::dec << "Current Anticipated Packets: " << anticipatedPackets_ << std::endl; 
	  std::cout << std::dec << "Found a board: " << brd->id() << std::endl; 
	}
      }   
      if (!padeBoards.empty()) { 
	std::string msg = "read\r\n"; 
	auto fn = [this, msg](const::boost::system::error_code &ec, std::size_t bytes) { 
	  if (!ec) { 
		     sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::readHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
	  }
	}; 
	
	sock_.async_write_some(boost::asio::buffer(msg), fn); 	


      }
    }
  }


void padeServer::clearHandler(const::boost::system::error_code &ec, std::size_t bytes) { 
    if (!ec) { 
      //      std::cout << "Read:" << bytes << " bytes. " << std::endl; 
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
	  if (!ec) { 
		     sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::statusHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
	  }
	}; 
	sock_.async_write_some(boost::asio::buffer(std::string("status\r\n")), fn); 

      }
    }
  }

void padeServer::writeHandler(const::boost::system::error_code &ec, std::size_t bytes) { 
    if (!ec) { 
      std::cout << "Wrote: " << bytes << " bytes. " << std::endl; 
      
    }
  }

void padeServer::connectHandler(const boost::system::error_code &ec) { 
    if (!ec) { 
      std::cout << "Connecting" << std::endl; 
      sock_.async_write_some(boost::asio::buffer(std::string("clear\r\n")), boost::bind(&padeServer::writeHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 

      sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::clearHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
    }

  }

void padeServer::connect() { 
    sock_.async_connect(endpoint_, boost::bind(&padeServer::connectHandler, this, boost::asio::placeholders::error)); 
  }
