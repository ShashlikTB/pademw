#ifndef PADEBOARD_H
#define PADEBOARD_H

#include <string>
#include <vector>
#include <map> 
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>


struct padePacket { 
  unsigned int ts; 
  unsigned int pktCount; 
  unsigned int channel; 
  unsigned int event; 
  unsigned short boardID; 
  std::vector<int> waveform; 
};



class padeBoard { 

  unsigned int boardid_; 
  unsigned int ptemp_; 
  unsigned int stemp_; 
  unsigned int triggerInMem_; 
  unsigned int lastTrigger_; 
  unsigned int statusReg_; 
  unsigned int errorReg_; 
  bool armed_;   
  bool master_; 
  std::map<unsigned int, std::vector<struct padePacket> > events; 

 public: 
  padeBoard(const std::string &msg);
  unsigned int id() { return boardid_; }; 
  unsigned int padeTemperature() { return ptemp_; }; 
  unsigned int sipmTemperature() { return stemp_; }; 
  unsigned int availableTriggers() { return triggerInMem_; }; 
  unsigned int lastTrigger() { return lastTrigger_; }; 
  unsigned int statusRegister() { return statusReg_; };
  unsigned int errorRegister() { return errorReg_; }; 
  unsigned int nEvents() { return events.size(); }; 
  unsigned int eventCount() { return events.size(); }; 


  void updatePadeBoard(const std::string &msg); 

  void clearEvents() { 
    events.clear(); 
  }

  void addEvent(unsigned int key, struct padePacket &pkt) { 
    if (events.count(key) == 0) { 
      events[key] = std::vector<struct padePacket>(); 
    }
    std::vector<struct padePacket> &packets = events[key]; 
    packets.push_back(pkt); 
  }

  std::vector<padePacket> &getEvent(unsigned int key) { 
    return events[key]; 
  }
  
  std::map<unsigned int, std::vector<struct padePacket> >::iterator begin() { return events.begin(); }; 
  std::map<unsigned int, std::vector<struct padePacket> >::iterator end() { return events.end(); }; 
  
  void printEvents() { 
    for (auto pkts : events) { 
      std::cout << std::get<1>(pkts).size(); 
    }
  }
  bool armed() { return armed_; };
  bool isMaster() { return master_; }; 


}; 


#endif 
