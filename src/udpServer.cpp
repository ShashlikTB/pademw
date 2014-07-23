#include "udpServer.h"



void udpListener::packetHandler(const::boost::system::error_code &ec, std::size_t bytes) {
    anticipatedPackets_--; 
    unprocessedPackets.push(recv_); 

    
    struct padePacket pkt = parsePadePacket(recv_); 
    pkt.ts = bytes; 
    packets.push_back(pkt); 
    if (packets.back().pktCount != packetCount_) { 
      std::cout << std::dec << packets.back().pktCount << " Ext Count " << packetCount_ << " Internal, Desynced!" << std::endl; 
      std::cout << "Attempting resync on next event boundary" << std::endl; 
    }
    /*    std::cout << std::dec << "Packet Count: " << packets.back().pktCount << " Channel: " 
	  << packets.back().channel << " Board ID:" << packets.back().boardID << std::endl; */
    packetCount_++; 
  
    recv_.fill(0); 
    if (running_ && anticipatedPackets_ > 0) {
      sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&udpListener::packetHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
    }


}


void udpListener::startListener(unsigned int anticipatedPackets) { 
    anticipatedPackets_ = anticipatedPackets; 
    //    std::cout << "Starting UDP Listener" << std::endl; 
    recv_.fill(0); 
    running_ = true; 
    sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&udpListener::packetHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 

}



void udpListener::stopListener() { 
    std::cout << "Stopping Listener" << std::endl; 
    running_ = false; 
    std::cout << unprocessedPackets.size() << " packets" << std::endl; 
    /*    while (!unprocessedPackets.empty()) { 
      for (auto ch : unprocessedPackets.front()) { 
	std::cout << std::hex << ch << " ";
      }
      std::cout << std::endl << std::endl; */
      
  }
