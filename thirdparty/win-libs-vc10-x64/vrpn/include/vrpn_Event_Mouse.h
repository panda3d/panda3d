/**************************************************************************************************/
/*                                                                                                */
/* Copyright (C) 2004 Bauhaus University Weimar                                                   */
/* Released into the public domain on 6/23/2007 as part of the VRPN project                       */
/* by Jan P. Springer.                                                                            */
/*                                                                                                */
/**************************************************************************************************/
/*                                                                                                */
/* module     :  vrpn_Event_Mouse.h                                                               */
/* project    :                                                                                   */
/* description:  mouse input using the event interface                                            */
/*                                                                                                */
/**************************************************************************************************/


#ifndef _VRPN_EVENT_MOUSE_H_
#define _VRPN_EVENT_MOUSE_H_

// includes, project
#include "vrpn_Event_Analog.h"
#include "vrpn_Button.h"


class VRPN_API vrpn_Event_Mouse: public vrpn_Event_Analog, 
                        public vrpn_Button_Server
{

public:

  //  creates a vrpn_Event_Mouse
  vrpn_Event_Mouse ( const char *name, vrpn_Connection *c = 0, 
                     const char* evdev_name = "/dev/input/event0" );

  // default dtor
  ~vrpn_Event_Mouse();

  // This routine is called each time through the server's main loop. It will
  // read from the mouse.
  void  mainloop (void);

private:

  //  This routine interpret data from the device
  void process_mouse_data ();

  // set all buttons and analogs to 0
  void clear_values();

private:

  struct timeval timestamp;       
};

#endif // _VRPN_EVENT_MOUSE_H_
