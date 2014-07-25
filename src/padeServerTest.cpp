// 
//
#include "padeServer.h"


int main(int argc, char *argv[]) { 


  std::string address = "127.0.0.1"; 
  unsigned int clientPort = 23; 

  std::string WCaddress("FTBFWC01.FNAL.GOV"); 
  std::string WCport("5001"); 

  boost::asio::io_service mainThreadService; 

  TFile file("test.root", "RECREATE"); 
  sharedWCData wcData; 
  wireChamberClient wc(mainThreadService, WCaddress, WCport, wcData); 

  padeServer server(mainThreadService, address, clientPort, file, wcData); 
  server.startSpill(); 
  mainThreadService.run(); 
  file.Write(); 
  std::cout << "...Annnnnnnnnd....we are done! Good Night Chicago!" << std::endl; 

  return 0; 
}
