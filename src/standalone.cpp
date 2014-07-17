#include "pademw.h"



int main(int argc, char *argv[]) { 

  std::string address = "127.0.0.1"; 
  unsigned int udpPort = 21331; 
  unsigned int clientPort = 23; 
  boost::asio::io_service mainThreadService; 
  std::string udpPortString(TString::UItoa(udpPort, 10)); 
  std::string clientPortString(TString::UItoa(clientPort, 10)); 
  
  padeControlClient client(mainThreadService, std::string(address), clientPortString); 
  try { 
    client.connect(); 
    client.clear(); 
    if (!client.active()) { 
      std::cout << "I don't think I got connected to the pade Control software, disconnecting" << std::endl; 
      client.disconnect(); 
      exit(-1); 
      }
    client.status(); 
    std::vector <unsigned int> keys = client.getIDs(); 
    for (auto key : keys) { 
      std::cout << std::dec << "Board ids:" << client.getBoard(key)->id() << " Triggers: " << client.getBoard(key)->availableTriggers() << std::endl; 
    }

  }
  catch (const boost::exception &e) { 
    std::cerr << "Boost error"; 
  }
  catch (const std::exception &e) { 
    std::cerr << e.what(); 
  }


  return 0; 
} 