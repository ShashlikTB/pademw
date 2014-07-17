#include "gmock/gmock.h"    

#include "padeUDPServer.h"
#include "padeClient.h"


using namespace std; 

using ::testing::Eq; 



class udpTestClient { 

  shared_ptr<udp::socket> _udpSocket; 
  shared_ptr<udp::resolver> _client; 
  shared_ptr<udp::resolver::query> _query; 
  //  udp::socket *_udpSocket; 
  //  udp::resolver *_client; 
  //  udp::resolver::query *_query; 
  udp::endpoint _clientEndpoint; 
  udp::endpoint _serverEndpoint; 
  boost::asio::io_service& _eventHandler; 


public:
  udpTestClient(const TString &address, unsigned int port,  boost::asio::io_service &eventHandler); 
  ~udpTestClient()  { }; //delete _udpSocket; delete _client; delete _query; };
  
  shared_ptr<udp::socket> socket() { return _udpSocket; }; 
  shared_ptr<udp::resolver::query> query() { return _query; }; 
  bool sendMsg(const std::string &msg);
  void connect() {  _udpSocket->open(udp::v4()); }; 
  void disconnect() {  _udpSocket->close(); }; 
  bool is_open() { return _udpSocket->is_open(); }; 
  //  udp::endpoint *clientEndpoint() { return _clientEndpoint; }; 
  //  udp::endpoint *serverEndpoint() { return _serverEndpoint; }; 
  //  boost::asio::io_service *io_service() { return _eventHandler; }; 
  
}; 


udpTestClient::udpTestClient(const TString &address, unsigned int port,  boost::asio::io_service &eventHandler) : _eventHandler(eventHandler) { 
  
  _client = shared_ptr<udp::resolver>( new udp::resolver(_eventHandler)); 
  std::cout <<" Connecting to " << std::string(address.Data()) << ":" <<  std::string(TString::UItoa(port, 10)) << std::endl; 

  _query =  shared_ptr<udp::resolver::query>(new udp::resolver::query(udp::v4(), std::string(address.Data()), std::string((address.UItoa(port, 10)).Data()))); 
  _clientEndpoint =   *_client->resolve(*_query); 
  _udpSocket = shared_ptr<udp::socket>(new udp::socket((_eventHandler))); 
 
}


bool udpTestClient::sendMsg(const std::string &msg) { 
  boost::system::error_code ignored_error; 
  _udpSocket->send_to(boost::asio::buffer(msg), _clientEndpoint, 0, ignored_error); 
  if (ignored_error && ignored_error != boost::asio::error::message_size) { 
    std::cout << "Error code:" << ignored_error.value() << std::endl;
    std::cout << ignored_error.category().name() << std::endl;
    return false; 
  }

  return true; 

}

class ListenerTesting: public testing::Test { 
public:
  TString address; 
  unsigned int port;


  virtual void SetUp() { 
    address = "127.0.0.1"; 
    port = 21331; 


  }

  virtual void TearDown() { 


    boost::posix_time::seconds wait(0.25); 
    boost::this_thread::sleep(wait); 
  }

};  

class ClientTesting: public testing::Test { 

public:
  std::string address; 
  unsigned int port; 
  boost::asio::io_service mainThreadService; 
  std::string portString;

  virtual void SetUp() { 


    address = "127.0.0.1"; 
    port = 23; 
    portString = TString::UItoa(port, 10); 


  }

  virtual void TearDown() { 

    boost::posix_time::seconds wait(0.25); 
    boost::this_thread::sleep(wait); 

  }
}; 


class EventLoopTesting: public testing::Test { 
public:
  TString address; 
  unsigned int port;


  boost::asio::io_service mainThreadService; 
  virtual void SetUp() { 
    address = "127.0.0.1"; 
    port = 21331; 
    shared_ptr<udpTestClient> client(new udpTestClient(TString(address), port, mainThreadService)); 


  }

  virtual void TearDown() { 
    mainThreadService.stop(); 



    boost::posix_time::seconds wait(0.25); 
    boost::this_thread::sleep(wait); 

  }

};  



TEST_F(ListenerTesting, TestListenerSocket) { 
  boost::asio::io_service mainThreadService; 
  padeUDPServer padeServer(mainThreadService, port); 
  mainThreadService.run(); 
  ASSERT_TRUE(padeServer.finished()); 
}

TEST_F(ListenerTesting, TestPadePackets) { 
  std::cout << "Test Packet Count" << std::endl; 
  boost::asio::io_service mainThreadService; 
  padeUDPServer padeServer(mainThreadService, port); 
  //  padeServer.setpacketCount(231); 
  mainThreadService.run(); 
  ASSERT_TRUE(padeServer.packetCount() > 0); 
  std::vector<struct padePacket> *packets = padeServer.getPackets(); 
  for (std::vector<struct padePacket>::iterator it = packets->begin(); it != packets->end(); ++it) { 
    std::cout << "Packet Channel: " << (*it).channel << std::endl; 
    std::vector<int > &waveform = ((*it).waveform); 
    std::cout << "Packet Data:" << std::endl; 

    for (auto vt = waveform.begin(); vt != waveform.end(); ++vt) { 
      
      std::cout << std::hex << std::setfill('0') << std::setw(3) << *vt << " "; 
    }
    std::cout << std::dec << std::endl; 
  }

}


TEST_F(ListenerTesting, TestClient) { 
  port = 23; 
  boost::asio::io_service mainThreadService; 
  std::string portString(TString::UItoa(port, 10)); 
  padeControlClient client(mainThreadService, std::string(address), portString); 
  ASSERT_TRUE(client.connect()); 
  

}



TEST_F(ClientTesting, padeClientArming) { 
  padeControlClient client(mainThreadService, std::string(address), portString); 
  if (client.connect()) { 
    client.arm(); 
    ASSERT_TRUE(client.armed()); 
  }

}

