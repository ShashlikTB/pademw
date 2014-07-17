#ifndef PADEBOARD_H
#define PADEBOARD_H

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>



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

 public: 
  padeBoard(const std::string &msg);
  
  unsigned int id() { return boardid_; }; 
  unsigned int padeTemperature() { return ptemp_; }; 
  unsigned int sipmTemperature() { return stemp_; }; 
  unsigned int availableTriggers() { return triggerInMem_; }; 
  unsigned int lastTrigger() { return lastTrigger_; }; 
  unsigned int statusRegister() { return statusReg_; };
  unsigned int errorRegister() { return errorReg_; }; 
  bool armed() { return armed_; };
  bool isMaster() { return master_; }; 



}; 


#endif 
