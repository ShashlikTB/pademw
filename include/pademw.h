#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

static const char *menu = "You have the following options:\n (A)rm the trigger\n (D)isarm the Trigger\n (T)rigger Controller\n (R)ead Data\n (S)tatus\n (C)lear\n (Q)uit\n";   


struct manager_data { 
  struct event_base *base; 
  struct event *arm; 
  struct event *disarm; 
  struct event *trigger; 
  struct event *read; 
  struct event *clear; 
  struct event *status; 
  int tcpsock; 
  int udpsock; 
}; 



