#include "form.h"
 
MyForm::MyForm(QWidget *parent, io_server &server)
  : QWidget(parent), server_(server)
{
  status_ = false; 
  setupUi(this);
  timer_ = std::make_shared<QTimer>(); 
  connect( this->startButton, SIGNAL( clicked () ), this, SLOT(startButton_ServerRunner() ) ); 
  connect( this->timer_.get(), SIGNAL( timeout() ), this, SLOT(timeout_Poll()) ); 
  
}
 


void MyForm::startButton_ServerRunner() { 
  if (status_) { 
      std::cout << "Stopping server" << std::endl; 
      timer_->stop(); 
      status_ = false; 
      startButton->setText("Stopped"); 

  }
  else { 
    std::cout << "Running server" << std::endl; 
    timer_->start(100); 
    status_ = true; 
    startButton->setText("Running"); 
  }
  
  
}


void MyForm::timeout_Poll() { 
 server_(); 
}
