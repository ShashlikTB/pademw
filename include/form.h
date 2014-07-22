#ifndef FORM_H
#define FORM_H

#include <QTimer>
#include "ui_form.h"

#include <functional> 
#include <boost/signals2/signal.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
 

class io_server { 
  
  boost::asio::io_service &service_; 
 public: 
  io_server(boost::asio::io_service &service) : service_(service) {}; 
  
  
  void stop() { 

    service_.stop(); 
    service_.reset(); 
  }

  void operator()() { 
    service_.reset(); 
    service_.poll(); 
  }
  

}; 


class MyForm : public QWidget, private Ui::Form
{
	Q_OBJECT
	std::shared_ptr<QTimer> timer_; 
	bool status_; 
	io_server &server_; 
	
 public:
	MyForm(QWidget *parent, io_server &server); 

	public Q_SLOTS:
	  void startButton_ServerRunner(); 
	  void timeout_Poll();


};
 
#endif

