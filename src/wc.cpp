#include "wc.h"




bool sharedWCData::inSpill(bool spill) { 

  boost::mutex::scoped_lock lock(guard); 
  inSpill_ = spill; 
  return inSpill_; 
}


bool sharedWCData::inSpill() { 

  boost::mutex::scoped_lock lock(guard); 
  return inSpill_; 

}

unsigned int sharedWCData::beamTime() { 

 boost::mutex::scoped_lock lock(guard); 
 return time_; 


}


unsigned int sharedWCData::beamTime(unsigned int bT) { 

 boost::mutex::scoped_lock lock(guard); 
 time_ = bT; 
 return time_; 


}


void wireChamberClient::connect() { 

  recv_.fill(0); 
    if (!connected_) { 
      //      std::cout << "Not Connected, attempting to connect to wire chamber " << std::endl; 
      tcp::resolver::iterator it = resolver_.resolve(query_); 
      tcp::resolver::iterator end; 
      tcp::endpoint endpoint_; 
      while (it != end) { 
	endpoint_ = *it++; 
	auto addr = endpoint_.address(); 
	if (addr.is_v4()) { 
	  break;
	}
      }

      sock_.async_connect(endpoint_, [this](const boost::system::error_code &ec) { 
	  if (ec) 
	    {std::cout << "Failed to connect to Wirechamber" << std::endl; }
	  else  {
	    connected_ = true; 
	    checkStatus(); 
	  }
	}); 
	    
	    
    }
    else 
      checkStatus(); 

}

void wireChamberClient::checkStatus()  {

  sock_.async_write_some(boost::asio::buffer(std::string("rd 5\r\n")), wrFn); 
  sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&wireChamberClient::readStatus, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 


}


void wireChamberClient::readStatus(const boost::system::error_code &ec, std::size_t bytes)  { 
  if (ec) { 

    std::cout << "Lost connection to the wire chamber reading the status" << std::endl; 
  }
  else { 

    std::string statusMsg(recv_.data()); 
    recv_.fill(0); 

    //    boost::format fmter("Data received %1%\r\n"); 
    //    std::cout << fmter % statusMsg;

    sock_.async_write_some(boost::asio::buffer(std::string("rd b\r\n")), wrFn); 
    sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&wireChamberClient::spillStatus, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
    

  }  

}


void wireChamberClient::spillStatus(const boost::system::error_code &ec, std::size_t bytes)  {
  if (ec) { 

    std::cout << "Lost connection to the wire chamber reading the spill status" << std::endl; 
  }
  else { 

    std::string statusMsg(recv_.data()); 
    boost::algorithm::trim(statusMsg); 
    try { 
      if (boost::lexical_cast<unsigned int>(statusMsg) == 8) { 

	//	std::cout << "In spill Now" << std::endl;

	reset_.expires_from_now(boost::posix_time::seconds(6)); 
	reset_.async_wait([this] (const boost::system::error_code &ec) { 
	    if (!ec)
	      wcStatus_.inSpill(false); 

	    }); 
	wcStatus_.inSpill(true); 

      }
      else {
	wcStatus_.inSpill(false); 
	reset_.cancel(); 
      }
    }
    catch(const boost::bad_lexical_cast &) { 
      wcStatus_.inSpill(false); 
    }
  

    
    recv_.fill(0); 

    //    boost::format fmter("Data received %1%\r\n"); 
    //    std::cout << fmter % statusMsg;
  }



}

void wireChamberClient::disconnect() { 

  sock_.close(); 

}
