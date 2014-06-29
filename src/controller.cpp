#include <controller.h>


CntrlCmd::CntrlCmd(void) { 
  _cmd = Invalid;
  _cmdstr ="Bad Command"; 
  _value = 99; 
} 

CntrlCmd::CntrlCmd(Enum cmd) { 
  _cmd = cmd;
  _cmdstr = toString(_cmd); 
  _value = toValue(cmd); 
} 

 CntrlCmd::CntrlCmd(const TString &s) { 
  _cmdstr = s; 
  _cmd = fromString(_cmdstr); 
  _value = toValue(_cmd); 

} 
CntrlCmd& CntrlCmd::operator= (const CntrlCmd& cmd) { 
  _cmdstr = cmd.getString(); 
  _cmd = cmd.getEnum(); 
  _value = cmd.getValue(); 
  return *this; 
} 
CntrlCmd& CntrlCmd::operator= (const TString &s) { 
  _cmdstr = s; 
  _cmd = fromString(_cmdstr); 
  _value = toValue(_cmd); 
  return *this; 
} 
CntrlCmd& CntrlCmd::operator= (Enum cmd) { 
  _cmd = cmd; 
  _cmdstr = toString(cmd); 
  _value = toValue(cmd); 
  return *this; 
} 
bool CntrlCmd::operator== (const CntrlCmd& cmd) const { 
  if (_cmd == cmd.getEnum())
    return true; 
  return false; 
	      
} 
bool CntrlCmd::operator== (const TString &s) const { 
  if (_cmd == fromString(s)) 
    return true; 
  return false; 
} 
bool CntrlCmd::operator== (Enum cmd) const { 
  if (_cmd == cmd)
    return true; 
  return false; 

} 

CntrlCmd::Enum CntrlCmd::fromString(TString s) { 
  s.ToLower(); 
  if (s == "arm")     return arm; 
  else if (s == "disarm") return disarm; 
  else if (s == "trig")  return trig; 
  else if (s == "read") return read; 
  else if (s == "clear")  return clear; 
  else if (s == "status") return status; 
  else return Invalid; 

} 

TString CntrlCmd::toString(CntrlCmd::Enum cmd) { 

  switch (cmd) {
  case arm:
    return "arm"; 
  case disarm:
    return "disarm"; 
  case trig:
    return "trig"; 
  case read: 
    return "read"; 
  case clear:
    return "clear"; 
  case status: 
    return "status"; 
  case Invalid:
    break; 
  }
  return "Invalid Command"; 

}



int CntrlCmd::toValue(CntrlCmd::Enum cmd) { 
  switch (cmd) { 
  case arm:
    return 0;
  case disarm:
    return 1; 
  case trig:
    return 2; 
  case read:
    return 3; 
  case clear:
    return 4; 
  case status:
    return 5; 
  case Invalid:
    break; 

  }
  return 99; 

}

