#include "padeClient.h"

void padeControlClient::armCB() { 
    std::cout << "Arming" << std::endl; 
    armed_ = true; 
  }

  void padeControlClient::disarmCB() { 
    std::cout << "Disarming" << std::endl; 
    armed_ = false; 
  }
   
  void padeControlClient::trigCB() { 
    std::cout << "Triggering" << std::endl; 
  }

  void padeControlClient::clearCB() { 
    active_ = true; 
  }

  void padeControlClient::readCB() { 
    std::cout << "UDP Packets" << std::endl; 
    readReady_ = true; 
    
  }


  void padeControlClient::statusLoop() { 
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

  void padeControlClient::recvLoop() { 
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
      callbacks[key](); 

    }
    else 
      std::cout << "Couldn't find action:" << key << std::endl; 



  }

padeControlClient::padeControlClient(boost::asio::io_service &io_service, const std::string &address, const std::string &service) : sock_(io_service), resolver_(io_service), query_(address, service) { 
    callbacks[std::string("arm")] = std::bind(&padeControlClient::armCB, this); 
    callbacks[std::string("disarm")] = std::bind(&padeControlClient::disarmCB, this); 
    callbacks[std::string("clear")] = std::bind(&padeControlClient::clearCB, this); 
    callbacks[std::string("trig")] = std::bind(&padeControlClient::trigCB, this); 
    callbacks[std::string("read")] = std::bind(&padeControlClient::readCB, this); 
    active_ = false; 
    readReady_ = false; 
  }


  bool padeControlClient::connect() { 
    tcp::resolver::iterator endpoint_it  = resolver_.resolve(query_); 
    boost::asio::connect(sock_, endpoint_it); 
    return true; 
  }

  void padeControlClient::disconnect() { 
    sock_.close(); 
    active_ = false; 
  }

  void padeControlClient::arm() { 
    boost::system::error_code ec; 
    boost::asio::write(sock_, boost::asio::buffer(std::string("arm\r\n")), ec); 
    recvLoop(); 

  }

  void padeControlClient::disarm() { 
    boost::system::error_code ec; 
    boost::asio::write(sock_, boost::asio::buffer(std::string("disarm\r\n")), ec); 

  }

  void padeControlClient::clear() { 
    boost::system::error_code ec; 
    boost::asio::write(sock_, boost::asio::buffer(std::string("clear\r\n")), ec); 
    recvLoop(); 
  }
    
  void padeControlClient::trig() { 

  }

  void padeControlClient::read() { 
    boost::system::error_code ec; 
    boost::asio::write(sock_, boost::asio::buffer(std::string("read 1\r\n")), ec); 
    recvLoop(); 
  }

  void padeControlClient::status() { 
    boost::system::error_code ec; 
    boost::asio::write(sock_, boost::asio::buffer(std::string("status\r\n")), ec); 
    statusLoop(); 
  }

  shared_ptr<padeBoard> padeControlClient::getBoard( unsigned int id ) { 
    if  (padeBoards.count(id) > 0)
      return padeBoards[id]; 
    shared_ptr<padeBoard> nBoard; 
    return nBoard; 
  }

  std::vector<unsigned int> padeControlClient::getIDs() { 
    std::vector<unsigned int> ids; 
    for ( auto board : padeBoards ) { 
      ids.push_back(board.first); 
    }
    return ids; 
  }
