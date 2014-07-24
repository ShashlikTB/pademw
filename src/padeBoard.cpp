#include "padeBoard.h"




padeBoard::padeBoard(const std::string &msg) {
  std::vector <std::string> split; 
  boost::algorithm::split(split, msg, boost::algorithm::is_any_of(" ")); 

  std::vector<std::string>::iterator it = split.begin(); 
  if (boost::algorithm::contains(msg, "Master")) {
    //Master has 2 extra entries
    it += 2; 
    master_ = true; 
  }
  // Layout should be as follows 
  // # BoardId statusRegister Armed #Triggers Error Register LastTrigger PadeTemp SipmTemp

  boardid_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  statusReg_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  long int tmp = strtol((*it).data(), NULL, 16); 
  if (tmp == 1)
    armed_ = true; 
  ++it; 
  triggerInMem_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  errorReg_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  lastTrigger_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  ptemp_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  stemp_ = strtol((*it).data(), NULL, 16); 

}


void padeBoard::updatePadeBoard(const std::string &msg) { 

  std::vector <std::string> split; 
  boost::algorithm::split(split, msg, boost::algorithm::is_any_of(" ")); 

  std::vector<std::string>::iterator it = split.begin(); 
  if (boost::algorithm::contains(msg, "Master")) {
    //Master has 2 extra entries
    it += 2; 
    master_ = true; 
  }
  // Layout should be as follows 
  // # BoardId statusRegister Armed #Triggers Error Register LastTrigger PadeTemp SipmTemp

  boardid_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  statusReg_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  long int tmp = strtol((*it).data(), NULL, 16); 
  if (tmp == 1)
    armed_ = true; 
  ++it; 
  triggerInMem_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  errorReg_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  lastTrigger_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  ptemp_ = strtol((*it).data(), NULL, 16); 
  ++it; 
  stemp_ = strtol((*it).data(), NULL, 16); 

}
