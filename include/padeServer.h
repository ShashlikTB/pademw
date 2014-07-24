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
#include <set>
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
      triggerCount_ = 0; 
      padeListener_.resetSync(); 
      currentSpill_ = 0; 
    connected_ = false; 
    timeout_ = false; 
    anticipatedPackets_ = 0; 
    whFn = boost::bind(&padeServer::writeHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred); 


  }


  void connectControl(const boost::system::error_code &ec); 
  void addPacketsToBoards(std::vector<struct padePacket> &packets);

  std::string buildReadString();
  void packetLoop(const boost::system::error_code &ec, std::size_t bytes); 
  void padePacketProcessing();
  void finished(); 



  void clearPackets();
  void parseStatusMessage(const std::string &status); 







  void startSpill(); 
  void livingControl(const::boost::system::error_code &ec, std::size_t bytes);

  void statusControl(const::boost::system::error_code &ec, std::size_t bytes);
  void disarmControl(const boost::system::error_code &ec, std::size_t bytes); 
  void dataPacketControl(const boost::system::error_code &ec, std::size_t bytes); 

  void udpControl(const boost::system::error_code &ec, std::size_t bytes); 

  void processPackets(); 
  void endSpill(const boost::system::error_code &ec); 
  void writeHandler(const::boost::system::error_code &ec, std::size_t bytes);

  shared_ptr<padeBoard> findMaster(); 


 private:

  void clearEvents() { 
    for (auto brd : padeBoards) { 

      std::get<1>(brd)->clearEvents(); 

    }

  }

  boost::asio::io_service &service_; 
  tcp::socket sock_; 
  tcp::endpoint endpoint_; 


  boost::asio::deadline_timer timer_; 
  udpListener padeListener_; 
  boost::posix_time::seconds timeoutLen_; 
  bool connected_; 

  std::array<char, 512> recv_; 
  std::map<unsigned int, shared_ptr<padeBoard> > padeBoards; 
  unsigned int anticipatedPackets_; 
  std::set<unsigned int> readEvents_; 
  bool desynced_; 
  bool timeout_; 
  unsigned short triggerCount_; 
  TFile &f_; 
  std::function<void (const::boost::system::error_code &ec, std::size_t bytes)> whFn; 

  unsigned int currentSpill_; 

}; 

#endif 
