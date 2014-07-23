#ifndef _PADESERVER_H
#define _PADESERVER_H


#include <memory> 
#include <cstdlib>
#include <sys/time.h>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>
#include <queue>
#include "TString.h"
#include "TBEvent.h"
#include "TTree.h"
#include "TFile.h"
#include <string>
#include <functional>
using boost::asio::ip::udp; 
using boost::asio::ip::tcp; 
using std::shared_ptr; 
#include "padeBoard.h"
#include "udpServer.h"



class padeServer { 




 public: 
 padeServer(boost::asio::io_service &service, const std::string &address, unsigned short tcpport, TFile &file) : service_(service), sock_(service), 
    endpoint_(tcp::endpoint(boost::asio::ip::address::from_string(address), tcpport)), timer_(service, boost::posix_time::seconds(5)), padeListener_(service, 21331), timeoutLen_(boost::posix_time::seconds (5)), f_(file)

    {
    connected_ = false; 
    timeout_ = false; 
    anticipatedPackets_ = 0; 
    std::function<void ()> fn = [this](){ 
      std::cout << "Got all packets" << std::endl;       
      if (sock_.available() > 0) { 
	//need to clear recv buffer
	sock_.read_some(boost::asio::buffer(recv_)); 
	std::cout << "Excess stuff " << std::string(recv_.data()) << std::endl; 
	recv_.fill(0); 
      }
      sock_.async_write_some(boost::asio::buffer(std::string("clear\r\n")), boost::bind(&padeServer::writeHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
      sock_.async_receive(boost::asio::buffer(recv_), boost::bind(&padeServer::clearHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
      timer_.cancel(); 
      padePacketProcessing(); 

    }; 

    padeListener_.setCallback(fn); 

  }

  void addPacketsToBoards(std::vector<struct padePacket> &packets);

  void padePacketProcessing();


  void readHandler(const::boost::system::error_code &ec, std::size_t bytes);


  void statusHandler(const boost::system::error_code &ec, std::size_t bytes);

  void clearHandler(const::boost::system::error_code &ec, std::size_t bytes);


  void writeHandler(const::boost::system::error_code &ec, std::size_t bytes);


  void connectHandler(const boost::system::error_code &ec);

  void connect(); 


 private:

  boost::asio::io_service &service_; 
  tcp::socket sock_; 
  tcp::endpoint endpoint_; 
  boost::asio::deadline_timer timer_; 
  udpListener padeListener_; 
  boost::posix_time::seconds timeoutLen_; 
  bool connected_; 

  std::array<char, 512> recv_; 
  std::map<std::string, std::function< void() > > callbacks; 
  std::map<unsigned int, shared_ptr<padeBoard> > padeBoards; 
  unsigned int anticipatedPackets_; 
  bool timeout_; 
  TFile &f_; 


}; 

#endif 
