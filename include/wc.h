#ifndef _WC_H
#define _WC_H



#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <string>
#include <functional>


using boost::asio::ip::udp; 
using boost::asio::ip::tcp; 
using std::shared_ptr; 


class wireChamberClient { 


  std::function<void (const boost::system::error_code &ec, std::size_t bytes)> wrFn; 
  boost::asio::io_service &service_; 
  tcp::socket sock_; 
  tcp::resolver resolver_; 
  std::string address_; 
  std::string port_;
  tcp::resolver::query query_; 
  bool connected_; 

  std::array<char, 512> recv_; 


 public: 
 wireChamberClient(boost::asio::io_service &service, const std::string &address, const std::string& tcpport) : service_(service), sock_(service), 
    resolver_(service), address_(address), port_(tcpport), query_(address_, port_) { 
    connected_ = false; 

    wrFn = [this](const boost::system::error_code &ec, std::size_t bytes) { 
      if (ec) { 
       std::cout << boost::format("Failed to connect %1% \r\n") % ec; 
      }
    }; 


  }



  void connect(); 

  void checkStatus(); 
  void readStatus(const boost::system::error_code &ec, std::size_t bytes); 
  void spillStatus(const boost::system::error_code &ec, std::size_t bytes); 
  void disconnect(); 




};







#endif 

