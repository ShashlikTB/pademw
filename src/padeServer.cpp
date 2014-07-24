#include "padeServer.h"
#include "udpServer.h"


void padeServer::addPacketsToBoards(std::vector<struct padePacket> &packets) { 
    for (auto packet : packets) { 

      unsigned int boardKey = packet.boardID; 
      unsigned int eventKey = packet.event; 
      if (padeBoards.count(boardKey) > 0)  {
	padeBoards[boardKey]->addEvent(eventKey, packet); 

      }
      

    }


}


shared_ptr<padeBoard> padeServer::findMaster() { 
  for (auto pair : padeBoards) { 
    if (std::get<1>(pair)->isMaster())
      return std::get<1>(pair); 
  }
  return std::nullptr_t(); 
}

void padeServer::padePacketProcessing() { 
    std::cout << "Processing packets" << std::endl; 
    if (!padeListener_.desynced()) { 
      
      auto master = findMaster(); 
      if (master != std::nullptr_t()) 
	readEvents_.insert(triggerCount_); 
      //      std::cout << "Inserting last trigger" << triggerCount_ << std::endl; 

    }
    padeListener_.resetSync(); 
    std::vector<struct padePacket> &packets = padeListener_.Packets(); 
    addPacketsToBoards(packets); 
    //Wait for server to get ready 
    //boost::asio::deadline_timer shortWait(service_, boost::posix_time::microseconds(100)); 
    //    shortWait.wait(); 

				    
    /* std::cout << "Board List" << std::endl; 
    for (auto pair : padeBoards) { 
      std::cout << "ID:" << std::get<0>(pair) << " Packets in Events "; 
      std::get<1>(pair)->printEvents(); 
      std::cout << std::endl; 
      } */

    
    /*
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
    */
    std::cout << "Clearing boards" << std::endl; 

    clearEvents(); 


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
	  if (padeBoards.count(brd->id()) == 0) { 
	    padeBoards[brd->id()] = brd; 
	  }
	  else { 
	    padeBoards[brd->id()]->updatePadeBoard(part); 
	  }
	  anticipatedPackets_ += 33; 
	  std::cout << std::dec << "Current Anticipated Packets: " << anticipatedPackets_ << std::endl; 
	  std::cout << std::dec << "Found a board: " << brd->id() << std::endl; 

	}
      }   
      if (!padeBoards.empty()) { 
	std::cout << std::dec; 
	boost::format fmter("read %1%\r\n"); 
	std::string msg = "read all\r\n"; 
	for (auto board : padeBoards) { 
	  shared_ptr<padeBoard> brd = std::get<1>( board); 
	  if (brd->isMaster()) { 

	    if (readEvents_.count(brd->lastTrigger()) != 0) {
	      //Get next trigger
	      triggerCount_ = brd->lastTrigger()+1; 
	    }
	    else {
	      triggerCount_ = brd->lastTrigger(); 
	    }
	    fmter % triggerCount_;
	  	  
	    msg = fmter.str();
	    break; 
	  }
	}
	std::cout << msg; 
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
  if (ec) { 
    std::cout << "Error in receive" << std::endl; 
  }
    if (!ec) { 
      //      std::cout << "Clear:" << bytes << " bytes. " << std::endl; 
      std::string line(recv_.data()); 
      //      std::cout << "clear line:" << line << std::endl; 
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
