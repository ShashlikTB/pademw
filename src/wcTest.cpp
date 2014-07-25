#include "wc.h"

#include <functional>
#include <thread> 



int main()  {


  boost::asio::io_service mainThreadService; 
    std::string address("127.0.0.1"); 
  //  std::string address("FTBFWC01.FNAL.GOV"); 
  std::string port("5001"); 

  sharedWCData wcData; 
  wireChamberClient wc(mainThreadService, address, port, wcData); 
  boost::asio::deadline_timer wcUpdater(mainThreadService, boost::posix_time::seconds(1)); 


  std::function<void (const::boost::system::error_code &ec)> wcPoll = [&](const::boost::system::error_code &ec) { 
      wc.connect(); 
      wcUpdater.expires_from_now(boost::posix_time::seconds(1)); 
      wcUpdater.async_wait(wcPoll); 
  }; 


  wcUpdater.async_wait(wcPoll); 
  std::thread t1([&] { 
      unsigned int i = 0; 
      while (i < 500) { 
	if (wcData.inSpill()) 
	  std::cout << "In Spill" << std::endl; 
	else 
	  std::cout << "Not in spill" << std::endl; 
	boost::this_thread::sleep(boost::posix_time::seconds(1)); 
      }

    }); 


      

  mainThreadService.run(); 


  return 0; 
}
