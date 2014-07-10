#include <pademw.h>



void padeListener::Stop() { 
  _listener.close(); 
  

}

std::tuple<bool, padeListener *> setupListener(const TString &addr, unsigned int port) { 
  padeListener *listen = NULL; 
  boost::asio::io_service io_service; 
  listen = new padeListener(io_service, port); 

  return std::make_tuple(true, listen); 

}



padeEventLoop::padeEventLoop(const padeEventLoop &pade) { 
  _listener = pade._listener; 

}



/*padeListener::padeListener(const padeListener &listener) { 
  _listener = listener._listener;
  _io_service = listener._io_service; 
  _remote = listener._remote; 
  _recv = listener._recv; 

  }*/





/* {
  int sock = -1; 
  padeListener *listen = new padeListener(port); 

  //attempt to setup boost 
  try { 
    boost::asio::io_service io_service; 
    udp::socket socket(io_service, udp::endpoint(udp::v4(), port)); 
    std::array<char, 500> recv; 
    udp::endpoint remote; 
    boost::system::error_code error; 
    socket.receive_from(boost::asio::buffer(recv), remote, 0, error); 
    if (error && error != boost::asio::error::message_size)
      throw boost::system::system_error(error); 
    std::cout << "Client sent us:" << std::string(recv.data()) << std::endl; 
    std::string msg = "hello you filthy....\n"; 
    
    boost::system::error_code ignored_error; 
    socket.send_to(boost::asio::buffer(msg), remote, 0, ignored_error); 
      

  }
  catch (std::exception& e) { 
    std::cerr << e.what() << std::endl; 
    return std::make_tuple(false, listenSocket); 
  }
  return std::make_tuple(true, listenSocket); 

} 

*/

