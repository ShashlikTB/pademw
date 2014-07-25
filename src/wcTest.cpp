#include "wc.h"



int main()  {


  boost::asio::io_service mainThreadService; 
    std::string address("127.0.0.1"); 
  //  std::string address("FTBFWC01.FNAL.GOV"); 
  std::string port("5001"); 

  wireChamberClient wc(mainThreadService, address, port); 
  wc.connect(); 

  mainThreadService.run(); 
  wc.disconnect(); 

  return 0; 
}
