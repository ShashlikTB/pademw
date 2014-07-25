#include <QApplication>
#include <iostream>
 
#include "form.h"
#include "padeServer.h"



 
int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  boost::asio::io_service mainThreadService; 

  io_server bs(mainThreadService); 

  MyForm form(0, bs); 


  form.show();
 
  std::string address = "127.0.0.1"; 
  unsigned int clientPort = 23; 

  std::string WCaddress("FTBFWC01.FNAL.GOV"); 
  std::string WCport("5001"); 

  TFile file("test.root", "RECREATE"); 
  
  sharedWCData wcData; 
  wireChamberClient wc(mainThreadService, WCaddress, WCport, wcData); 
  padeServer server(mainThreadService, address, clientPort, file, wcData); 

  server.startSpill(); 


  return app.exec();
}


