// 
//
#include "padeServer.h"


int main(int argc, char *argv[]) { 


  std::string address = "127.0.0.1"; 
  unsigned int clientPort = 23; 
  boost::asio::io_service mainThreadService; 

  padeServer server(mainThreadService, address, clientPort); 
  server.connect(); 
  mainThreadService.run(); 
  std::cout << "...Annnnnnnnnd....we are done! Good Night Chicago!" << std::endl; 

  return 0; 
}
