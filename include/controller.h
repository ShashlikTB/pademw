// Controller handler for the PADE system
// Thomas Anderson 
// 6-27-2014 

#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <TString.h>



class CntrlCmd { 
 public:
  enum Enum { 
    arm=0,
    disarm,
    trig,
    read,
    clear,
    status,
    Invalid
  }; 

  CntrlCmd(void); 
  CntrlCmd(Enum cmd); 
  explicit CntrlCmd(const TString &s); 

  CntrlCmd& operator= (const CntrlCmd& cmd); 
  CntrlCmd& operator= (const TString &s); 
  CntrlCmd& operator= (Enum cmd); 


  bool operator== (const CntrlCmd& cmd) const; 
  bool operator== (const TString &s) const; 
  bool operator== (Enum cmd) const; 
  
  inline TString getString (void) const; 
  inline Enum getEnum(void) const; 
  inline int getValue (void) const; 

 private: 
  static Enum fromString(TString s); 
  static TString toString(Enum cmd); 
  static int toValue(Enum cmd); 

  Enum _cmd; 
  TString _cmdstr; 
  int _value; 
}; 



inline TString CntrlCmd::getString(void) const { 
  return _cmdstr; 
}
  
  

inline CntrlCmd::Enum CntrlCmd::getEnum(void) const  {
  return _cmd; 
}


inline int CntrlCmd::getValue (void) const { 
  return _value; 
}



#endif

  

