#include "padeServer.h"
#include "udpServer.h"



void padeServer::startSpill() { 

  std::cout << "Starting spill" << std::endl; 

  if (!connected_)  {
    std::cout << "Not Connected, attempting to connect" << std::endl; 
    sock_.async_connect(endpoint_, boost::bind(&padeServer::connectControl, this, boost::asio::placeholders::error)); 
  }
  else { 
    //Check for Life, clear buffers
    sock_.async_write_some(boost::asio::buffer(std::string("clear\r\n")), whFn);
    sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::livingControl, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
  }
  
} 

void padeServer::connectControl(const boost::system::error_code &ec)  {
  if (ec) { 
    std::cout << "Failed to connect: " << ec << std::endl; 
    sock_.close(); 
    boost::asio::deadline_timer retryTimeout(service_, boost::posix_time::seconds(4)); 
    std::cout << "Retrying in 4 seconds: " << ec << std::endl; 
    std::function<void (const boost::system::error_code &ec)> retryFn = [this](const boost::system::error_code &ec) { startSpill(); }; 
    retryTimeout.async_wait(retryFn); 
  }
  else { 
    connected_ = true; 
    startSpill(); 
  }

}



void padeServer::livingControl(const::boost::system::error_code &ec, std::size_t bytes) { 
    if (!ec) { 
      std::string clearMsg(recv_.data()); 
      recv_.fill(0); 
      if (!(boost::algorithm::contains(clearMsg, "clear"))) { 
	  std::cout << "Bad response from Control Server...trying again" << std::endl; 
	  startSpill(); 
      }
      std::cout << "Pade Control Server is alive!" << std::endl; 
      std::cout << "Getting # and Status of Pade Boards!" << std::endl;
      sock_.async_write_some(boost::asio::buffer(std::string("status\r\n")), whFn); 
      sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::statusControl, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
    }

}


void padeServer::parseStatusMessage(const std::string &status) { 

  std::vector <std::string> split;
  boost::algorithm::split_regex(split, status, boost::regex("Slave")); 
  if ((unsigned int) TString(split[0][0]).Atoi() == split.size()) { 
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

}

std::string padeServer::buildReadString() { 
  std::string msg = "read all\r\n"; 
  boost::format fmter("read %1%\r\n"); 
  if (!padeBoards.empty()) { 
	  
    auto master = findMaster(); 

    if (master == std::nullptr_t()) { 
      std::cout << "No master pade found!" << std::endl; 
      return msg; 
    }

    if (readEvents_.count(master->lastTrigger()) != 0)
      triggerCount_ = master->lastTrigger()+1; 
    else 
      triggerCount_ = master->lastTrigger(); 
    
    fmter % triggerCount_; 

    msg = fmter.str(); 

  }
  return msg; 




}

void padeServer::clearPackets() {  

    if (sock_.available() > 0) { 
      //need to clear recv buffer
	sock_.read_some(boost::asio::buffer(recv_)); 
	std::cout << "Excess stuff " << std::string(recv_.data()) << std::endl; 
	recv_.fill(0); 
      }


}



void padeServer::statusControl(const::boost::system::error_code &ec, std::size_t bytes) {
    if (!ec) { 

      std::string line(recv_.data()); 
      recv_.fill(0); 

      if (boost::algorithm::contains(line, "Master") && line.size() > 10) { 
	//Status message 
	parseStatusMessage(line); 
	auto armfn = [this](const::boost::system::error_code &ec, std::size_t bytes) { 
	  if (!ec) {
	    std::string armMsg(recv_.data()); 
	    recv_.fill(0); 
	    if (!(boost::algorithm::contains(armMsg, "arm"))) { 
	      //Failed to arm the Pade Boards, will try again from status
	      sock_.async_write_some(boost::asio::buffer(std::string("status\r\n")), whFn); 
	    }
	    //Armed the Pade Boards, wait for spill to complete, will eventually come from the wire chamber data 

	    sock_.async_write_some(boost::asio::buffer(std::string("trig\r\n")), whFn); 
	    sock_.async_write_some(boost::asio::buffer(std::string("trig\r\n")), whFn); 
	    sock_.async_write_some(boost::asio::buffer(std::string("trig\r\n")), whFn); 
	    sock_.async_write_some(boost::asio::buffer(std::string("trig\r\n")), whFn); 
	    sock_.async_write_some(boost::asio::buffer(std::string("trig\r\n")), whFn); 
	    sock_.async_receive(boost::asio::buffer(recv_), [this](const boost::system::error_code &ec, std::size_t bytes) { recv_.fill(0); }); 
	    boost::asio::deadline_timer waitForSpillCompletion(service_, boost::posix_time::seconds(5)); 
	    waitForSpillCompletion.async_wait(boost::bind(&padeServer::endSpill, this, boost::asio::placeholders::error)); 
	    
	  }
	}; 

	sock_.async_write_some(boost::asio::buffer(std::string("arm\r\n")), whFn); 
	sock_.async_receive(boost::asio::buffer(recv_), armfn); 
      }
      else {
	//Try again
	clearPackets(); 
	sock_.async_write_some(boost::asio::buffer(std::string("status\r\n")), whFn); 
	sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::statusControl, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
      }
    
    }

}

void padeServer::endSpill(const boost::system::error_code &ec) { 
  //Increment spill
  currentSpill_++; 

  //Write to status and read
  sock_.async_write_some(boost::asio::buffer(std::string("disarm\r\n")), whFn); 
  sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::disarmControl, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 

}

void padeServer::disarmControl(const boost::system::error_code &ec, std::size_t bytes) { 
  if (ec) { 
    std::cout << "May have lost connection, or other socket problem:: Disarm";
  }
  std::string disarmMsg(recv_.data()); 
  recv_.fill(0); 
  if (!(boost::algorithm::contains(disarmMsg, "disarm"))) { 
    //Didn't disarm properly or heard something funky
    //Retrying
    clearPackets(); 
    sock_.async_write_some(boost::asio::buffer(std::string("disarm\r\n")), whFn); 
    sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::disarmControl, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
  }

  //Disarmed
  sock_.async_write_some(boost::asio::buffer(std::string("status\r\n")), whFn); 
  sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::dataPacketControl, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 


}

void padeServer::packetLoop(const boost::system::error_code &ec, std::size_t bytes) { 

  std::cout << "Packet Loop" << std::endl; 
  if (ec) { 
    std::cout << "May have lost connection or other socket problem: packet loop" << std::endl; 
    finished(); 
    connected_ = false; 
    sock_.close(); 
    return; 
  }

  std::string statusMsg(recv_.data()); 
  recv_.fill(0); 

  if (boost::algorithm::contains(statusMsg, "Master")) {
    sock_.async_write_some(boost::asio::buffer(std::string("status\r\n")), whFn); 
    sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::dataPacketControl, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 

  }
}


void padeServer::dataPacketControl(const boost::system::error_code &ec, std::size_t bytes) { 

  if (ec) { 
    std::cout << "May have lost connection or other socket problem: data packet control" << std::endl; 
    connected_ = false; 
    finished(); 
  }

  std::string statusMsg(recv_.data()); 
  recv_.fill(0); 

  if (boost::algorithm::contains(statusMsg, "Master") && statusMsg.size() > 10) { 
    parseStatusMessage(statusMsg); 
    
    //Ready to read 
    std::string readMsg = buildReadString(); 
    
    auto readFn = [this, readMsg](const::boost::system::error_code &ec, std::size_t bytes) { 
      if (!ec) { 
	sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::udpControl, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
      }
    }; 
    sock_.async_write_some(boost::asio::buffer(readMsg), whFn); 
    sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::udpControl, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 


  }
}

  void padeServer::udpControl(const::boost::system::error_code &ec, std::size_t bytes) {
    if (ec) { 
      std::cout << "Socket problem at udpControl" << std::endl; 
    }

    std::string read(recv_.data()); 
    boost::algorithm::trim(read); 
    recv_.fill(0); 
    if (boost::algorithm::contains(read, "read")) { 
      timer_.expires_from_now(timeoutLen_); 
    
      //Setup listener callback 
      auto tcpCB = [this]() {
	std::cout << "Got all packets" << std::endl;       
	if (sock_.available() > 0) { 
	  //need to clear recv buffer
	  sock_.read_some(boost::asio::buffer(recv_)); 
	  std::cout << "Excess stuff " << std::string(recv_.data()) << std::endl; 
	  recv_.fill(0); 
	}
	processPackets(); 
	//	padeListener_.stopListener(); 
	timer_.cancel(); 
      };
      padeListener_.setCallback(tcpCB); 
      

      auto timeoutfn = [this](const boost::system::error_code &ec) { 
	if (!ec) { 
	  std::cout << "Timeout Waiting for Packets" << std::endl; 
	  timeout_ = true; 
	  //	  padeListener_.stopListener(); 
	  processPackets(); 
	}
      }; 
      timer_.async_wait(timeoutfn); 
      padeListener_.startListener(anticipatedPackets_); 

    }
  
}


void padeServer::processPackets() { 

    std::cout << "Processing packets" << std::endl; 
    auto master = findMaster(); 
    if (!padeListener_.desynced()) { 
      if (master != std::nullptr_t()) 
	readEvents_.insert(triggerCount_); 
      
    }

    padeListener_.resetSync(); 
    std::vector<struct padePacket> &packets = padeListener_.Packets(); 
    addPacketsToBoards(packets); 
    packets.clear(); 

    if (triggerCount_ < master->availableTriggers()) { 
      std::cout << master->availableTriggers()-triggerCount_ << " triggers available" << std::endl; 
      std::cout << "Master Avalible:" << master->availableTriggers() << " Trigger Count " << triggerCount_ << std::endl; 
      sock_.async_write_some(boost::asio::buffer(std::string("status\r\n")), whFn); 
      sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::dataPacketControl, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
    }
    else {
      std::cout << "We've gotten all available triggers!" << std::endl; 
      sock_.async_write_some(boost::asio::buffer(std::string("clear\r\n")), whFn); 
      auto finishedFn = [this](const::boost::system::error_code &ec, std::size_t bytes) { finished(); }; 
      sock_.async_receive(boost::asio::buffer(recv_), finishedFn); 

    }


}

void padeServer::finished() { 

  std::cout << "Filling TTree" << std::endl; 

  TTree *tree = new TTree("t1041", "T1041"); 
  TBEvent event; 
  TBSpill spill; 
  TBranch *tbevt = tree->Branch("tbevent", "TBEvent", &event, 64000, 0); 
  TBranch *tbspill = tree->Branch("tbspill", "TBSpill", &spill, 64000, 0); 
  timeval tim; 
  gettimeofday(&tim, NULL); 
  std::cout << "Current Spill " << currentSpill_ << std::endl; 
  spill.SetSpillData(currentSpill_, tim.tv_usec, 0, 0, 0, 0, -1, -1, 50.0, 50.0); 
  tbspill->Fill(); 
  
  shared_ptr<padeBoard> master = findMaster(); 
  unsigned int eventCount = master->nEvents(); 
  for (unsigned int i = 0; i < eventCount; i++) { 
    for (auto pair : padeBoards) { 
      shared_ptr<padeBoard> board (std::get<1>(pair) ); 
      std::vector<padePacket> &padeEvent = board->getEvent(i); 
      for (auto pkt : padeEvent) { 
	event.FillPadeChannel(tim.tv_usec, pkt.ts, pkt.boardID, pkt.pktCount, pkt.channel, pkt.event, pkt.waveform.data()); 
      }
    }
    tbevt->Fill(); 	
    event.Reset(); 
  }
    //    tree->Write(); 
  f_.Write(); 

    std::cout << "Clearing boards" << std::endl; 

    clearEvents(); 
    timer_.expires_from_now(roughDeltaNSpill_); 
    auto timeoutfn = [this](const boost::system::error_code &ec) { 
      if (!ec) { 
	  //	  padeListener_.stopListener(); 
	  startSpill(); 
	}
      }; 
    std::cout << "Waiting for next spill...." << std::endl; 
    timer_.async_wait(timeoutfn); 

}


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






void padeServer::writeHandler(const::boost::system::error_code &ec, std::size_t bytes) { 
    if (!ec) { 
      std::cout << "Wrote: " << bytes << " bytes. " << std::endl; 
      
    }
    else { 
      std::cout << "Error writing to socket, Boost Error:" << ec << std::endl; 

    }
  }
