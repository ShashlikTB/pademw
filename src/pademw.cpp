#include <pademw.h>


void padeListener::Stop() { 

  try {
    _io_service.stop(); 
    //    _listener.shutdown(boost::asio::ip::udp::socket::shutdown_receive); 
  }
  catch (std::exception& e) { 
    std::cerr << e.what() << std::endl; 
  }


}


padeListener::~padeListener() { 
  boost::system::error_code error; 
  std::cout << "Shutting Down listener!" << std::endl; 
  _listener->shutdown(boost::asio::ip::udp::socket::shutdown_both, error); 
  std::cout << "Problem shutting down listener: " << error.value() << std::endl; 
  std::cout << "Problem shutting down listener: " << error.category().name() << std::endl; 
  /*try { 
        _listener.close(error); 
    std::cout << "Problem closing listener: " << error.value() << std::endl; 
    std::cout << "Problem closing listener: " << error.category().name() << std::endl;
    std::cout << "Finished destroying listener!" << std::endl; 
  }
  catch (std::exception &e) { 
    std::cout << "Failed in destroying padelistener." << std::endl; 
    std::cerr << e.what() << std::endl; 
    }*/

}

bool padeListener::Running() { 
  return _listener->is_open(); 
}

bool padeListener::simplemanagementLoop() { 

  std::cout << "Starting management loop" << std::endl; 
  boost::system::error_code error; 
  udp::endpoint remote_endpoint; 
  for (;;) { 

    _listener->receive_from(boost::asio::buffer(recvBuffer), remote_endpoint, 0, error); 
    if (error && error != boost::asio::error::message_size)
      throw boost::system::system_error(error); 

    TString *recvd = new TString(recvBuffer.data()); 
    recvBuffer = {{ 0 }}; 
    std::cout << "We received: " << recvd->Data() << std::endl; 
    if (recvd->Contains("Hello")) {
      return true; 
    }
    if (recvd->Contains("Stop")) { 
      return false; 
    }
    delete recvd; 

  }
  return false; 

}

void padeListener::operator()() { 
    _io_service.run(); 
  std::cout << "Starting management loop" << std::endl; 
  boost::system::error_code error; 

  for (;;) { 
    udp::endpoint remote_endpoint;     
    _listener->receive_from(boost::asio::buffer(recvBuffer), remote_endpoint); 

    if (error && error != boost::asio::error::message_size)
      throw boost::system::system_error(error); 

    TString *recvd = new TString(recvBuffer.data()); 
    recvBuffer = {{ 0 }}; 
    std::cout << "We received: " << recvd->Data() << std::endl; 
    if (recvd->Contains("Hello")) {
      return; 
    }
    if (recvd->Contains("Stop")) { 
      return; 
    }
    delete recvd; 

  }
  return; 

}

  

std::tuple<bool, padeListener *> setupListener(const TString &addr, unsigned int port, boost::asio::io_service &io_service) {
  padeListener *listen = NULL; 

  listen = new padeListener(io_service, port); 
  
  return std::make_tuple(true, listen); 

}



