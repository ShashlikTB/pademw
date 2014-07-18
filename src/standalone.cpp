#include "padeUDPServer.h"
#include "padeClient.h"
#include "TBEvent.h"
#include "TTree.h"

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
    std::cout << "Control software reports the following ids:" << std::endl; 
    for (auto key : keys) { 
      std::cout << std::dec << client.getBoard(key)->id() << " Triggers: " << client.getBoard(key)->availableTriggers() << std::endl; 
    }
    
    padeUDPServer server(mainThreadService, udpPort); 
    std::cout << "Attempting to read the triggers from the server" << std::endl; 
    client.read(); 
    if (client.readReady()) { 
      std::cout << "starting to listen for packets" << std::endl; 
      server.receive_loop(); 
    }
    mainThreadService.run(); 
    std::cout << "After main thread" << std::endl; 
    std::vector < struct event > &events = server.Events(); 
    std::cout << "# Events:" << events.size() << std::endl; 
    for (auto it : events) { 
      for (auto packet : it.eventPackets) {
	std::cout << packet.channel << std::endl; 
      }
    }

    shared_ptr<padeBoard> brd; 
    for (auto key : keys) { 
      if (client.getBoard(key)->isMaster())
	brd = client.getBoard(key); 
    }
    TTree *tree = new TTree("t1041", "T1041"); 
    TBEvent tbevent; 
    TBSpill tbspill; 
    
	

  }
  catch (const boost::exception &e) { 
    std::cerr << "Boost error"; 
  }
  catch (const std::exception &e) { 
    std::cerr << e.what(); 
  }


  return 0; 
} 
