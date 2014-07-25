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

  TFile file("test.root", "RECREATE"); 
  padeServer server(mainThreadService, address, clientPort, file); 
  server.startSpill(); 


  return app.exec();
}


