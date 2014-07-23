#include "padeServer.h"
#include "udpServer.h"

struct padePacket udpListener::parsePadePacket(const std::array<unsigned char, 270> &array) { 
  struct padePacket pkt; 
  pkt.boardID = array[2]; 
  pkt.pktCount = array[4] << 8 | array[5]; 
  pkt.channel = array[6]; 
  pkt.event = array[7] << 8 | array[8]; 
  if (array.size() > 70) { 
    pkt.waveform.reserve(60); 
    int adc = 0; 
    for (unsigned int i = 0; i < 60; i++ ) { 
      //ADC position calculation pulled from Paul's C# Code 
      adc = array[17+4*i] * 256 + array[16+4*i]; 
      pkt.waveform.push_back(adc); 
      adc = array[15+4*i]*256 + array[14+4*i]; 
      pkt.waveform.push_back(adc); 
      adc = 0; 
    }
    pkt.waveform.shrink_to_fit(); 

  }
  return pkt; 
};
