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
      std::vector<char> charStyle; 
      for (unsigned int i = 0; i < recv_.size(); i++) { 
	charStyle.push_back(recv_[i]); 
      }
      TString str(charStyle.data()); 
      std::cout << "Message: " << str; 
      if (str.Contains("quit")) { 
	finishedFlag_ = true; 
	return; 
      }
	  
    }
    else { 
      //Pade Packet
      packetCount_++; 
      packets.push_back(parsePadePacket(recv_)); 
      if (packets.back().pktCount != packetCount_) { 
	std::cout << std::dec << packets.back().pktCount << "Ext Count " << packetCount_ << " Internal, Desynced!" << std::endl; 
      }
      for (auto& s: recv_) 
	std::cout << std::hex << (unsigned int) s << " "; 

      std::cout << std::endl; 
    }
  
    recv_.fill(0); 


    receive_loop(); 
  }
}

struct padePacket padeUDPServer::parsePadePacket(const std::array<unsigned char, 270> &array) { 
  struct padePacket pkt; 
  pkt.pktCount = array[4] << 8 | array[5]; 
  pkt.channel = array[6]; 
  pkt.hitCount = array[7] << 8 | array[8]; 
  if (array.size() > 70) { 
    pkt.waveform.reserve(260); 
    for (unsigned int i = 10; i < array.size(); i++) { 
      pkt.waveform.push_back(array[i]); 
    }
  }
  return pkt; 
}


bool send_bytes(const TString &msg) { 

  return false; 

}
