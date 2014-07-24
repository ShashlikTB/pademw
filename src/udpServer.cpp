#include "udpServer.h"

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



void udpListener::packetHandler(const::boost::system::error_code &ec, std::size_t bytes) {
    anticipatedPackets_--; 
    //    unprocessedPackets.push(recv_); 

   
    if (bytes == 70) { 
      // End Packet 
      unsigned int bid = recv_[2]; 
      std::cout << "End Packet Board ID: " << bid << std::endl; 
      boardPacketCount[bid]++; 

    }
    else if (bytes == 266) { 
      //Pade data packet
      struct padePacket pkt = parsePadePacket(recv_); 
      pkt.ts = bytes; 
      packets.push_back(pkt); 
      //      std::cout << "TS: " << pkt.ts << " Packet: " << pkt.pktCount << " Board: " << pkt.boardID << std::endl; 
      if (boardPacketCount[pkt.boardID] != pkt.pktCount) { 
	std::cout << "Caution! Packet Count Desynced! ID:" << pkt.boardID << " Internal:" << boardPacketCount[pkt.boardID] << " External: " << pkt.pktCount << std::endl; 
	desynced_ = true; 

      }
      boardPacketCount[pkt.boardID]++; 
    }
    recv_.fill(0); 
    if (running_ && anticipatedPackets_ > 0) {
      sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&udpListener::packetHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
    }
    else if (anticipatedPackets_ == 0) { 
      running_ = false; 
      std::cout << "We've received the expected number of packets" << std::endl; 
      if (tcpCB_ != std::nullptr_t()) { 
	tcpCB_(); 
	
      }
    }
	


}


void udpListener::startListener(unsigned int anticipatedPackets) { 
  packets.clear(); 
  anticipatedPackets_ = anticipatedPackets; 
  if (!sock_.is_open()) { 
    sock_.open(boost::asio::ip::udp::v4()); 
  }

  //    std::cout << "Starting UDP Listener" << std::endl; 
  recv_.fill(0); 
  running_ = true; 
  sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&udpListener::packetHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 

}



void udpListener::stopListener() { 
  //    std::cout << "Stopping Listener" << std::endl; 
    running_ = false; 
    //    std::cout << unprocessedPackets.size() << " packets" << std::endl; 
    sock_.close(); 
    /*    while (!unprocessedPackets.empty()) { 
      for (auto ch : unprocessedPackets.front()) { 
	std::cout << std::hex << ch << " ";
      }
      std::cout << std::endl << std::endl; */
      
  }
