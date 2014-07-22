#include "padeUDPServer.h"






void padeUDPServer::receive_loop() { 
  std::cout << "Receive Loop" << std::endl; 
  if (!finishedFlag_) { 
    sock_.async_receive_from(
			     boost::asio::buffer(recv_), remote_endpoint_,
			     boost::bind(&padeUDPServer::handle_receive, this,
					 boost::asio::placeholders::error,
					 boost::asio::placeholders::bytes_transferred));
  }


}


void padeUDPServer::padedataPacketHandler() { 
  packets.push_back(parsePadePacket(recv_)); 
  if (packets.back().pktCount != packetCount_) { 
    std::cout << std::dec << packets.back().pktCount << " Ext Count " << packetCount_ << " Internal, Desynced!" << std::endl; 
    std::cout << "Attempting resync on next event boundary" << std::endl; 
  }
  std::cout << std::dec << "Packet Count: " << packets.back().pktCount << " Channel: " << packets.back().channel << std::endl; 
      /* for (auto& s: recv_) 
	std::cout << std::hex << (unsigned int) s << " "; 
	std::cout << std::endl; */
  packetCount_++; 




}


void padeUDPServer::endPacketHandler() { 
  packetCount_++; 
  //Add event handler stuff
   event evt(packets); 
   events.push_back(evt); 
   packets.clear(); 
   finishedFlag_ = true; 
}

void padeUDPServer::unknownPacketHandler() { 
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

void padeUDPServer::handle_receive(const boost::system::error_code &ec, std::size_t bytes) { 
  if (!ec || ec == boost::asio::error::message_size) { 
    //    std::cout << bytes << " bytes Received" << std::endl; 
    switch (bytes) { 
    case 266:
      padedataPacketHandler(); 
      break; 
    case 70:
      endPacketHandler(); 
      break; 
    default:
      unknownPacketHandler(); 
    }

    recv_.fill(0); 
    receive_loop(); 
  }
}

struct padePacket padeUDPServer::parsePadePacket(const std::array<unsigned char, 270> &array) { 
  struct padePacket pkt; 
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
    //    std::cout << "Waveform Count:" << pkt.waveform.size() << std::endl; 
  }
  return pkt; 
}


bool send_bytes(const TString &msg) { 

  return false; 

}


