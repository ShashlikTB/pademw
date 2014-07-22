#include <QApplication>
#include <iostream>
 
#include "form.h"



 
class printer { 

public:
  printer(boost::asio::io_service &service, boost::posix_time::seconds s) : timer_(service, boost::posix_time::seconds(s)), count_(0) {
    timer_.async_wait(boost::bind(&printer::print, this)); 

  }

  ~printer() { 
    std::cout << "Final Count: " << count_ << std::endl; 
  }
  
  void print() {
      std::cout << "Fired! " << count_ << std::endl; 
      ++count_; 
      timer_.expires_at(timer_.expires_at()+boost::posix_time::seconds(1)); 
      timer_.async_wait(boost::bind(&printer::print, this)); 
      
  }

private: 
  boost::asio::deadline_timer timer_; 
  int count_; 
}; 


 
int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  boost::asio::io_service mainThreadService; 

  io_server bs(mainThreadService); 

  MyForm form(0, bs); 


  form.show();
 

  printer printing(mainThreadService, boost::posix_time::seconds(1)); 
	      




  return app.exec();
}


