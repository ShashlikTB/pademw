#include <pademw.h>



void padeUDPServer::receive_loop() { 
  sock_.async_receive_from(
        boost::asio::buffer(recv_), remote_endpoint_,
        boost::bind(&padeUDPServer::handle_receive, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));

    }

void padeUDPServer::handle_receive(const boost::system::error_code &ec, std::size_t bytes) { 
  if (!ec || ec == boost::asio::error::message_size) { 
    std::cout << bytes << " bytes Received" << std::endl; 
    if (!(bytes == 266 || bytes == 70)) { 
      std::cout << "Not a pade packet, printing only" << std::endl;
      TString str(recv_.data()); 
      std::cout << "Message: " << std::string(recv_.data()); 
      if (str.Contains("quit")) { 
	finishedFlag_ = true; 
	return; 
      }
	  
    }
    else { 
      //Pade Packet
      packetCount_++; 
      for (auto& s: recv_) 
	std::cout << std::hex << (unsigned int) s << " "; 

      std::cout << std::endl; 
    }

    recv_.fill(0); 


    receive_loop(); 
  }
}




bool send_bytes(const TString &msg) { 

  return false; 

}
