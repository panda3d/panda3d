/**************************************************************************************************/
/*                                                                                                */
/* Copyright (C) 2004 Bauhaus University Weimar                                                   */
/* Released into the public domain on 6/23/2007 as part of the VRPN project                       */
/* by Jan P. Springer.                                                                            */
/*                                                                                                */
/*************************************************************************************************/
/*                                                                                                */
/* module     :  vrpn_Event.h                                                                     */
/* project    :                                                                                   */
/* description:  provide functionality for event interface                                        */
/*                                                                                                */
/**************************************************************************************************/

#ifndef _VRPN_EVENT_H_
#define _VRPN_EVENT_H_

#include "vrpn_Shared.h"  // For struct timeval

namespace vrpn_Event {

  // the struct read by the system when reading 
  struct input_event {
  
    struct timeval time;
    unsigned short type;
    unsigned short code;
    unsigned int value;
  };

  // open the specified event interface
  // return a valid handle to the event interface or -1 if the open fails
  // file - full path of the event interface file 
  int vrpn_open_event( const char* file);

  // close the event interface
  // fd - handle to the event interface
  void vrpn_close_event( const int fd);

  // read from the interface
  // returns the number of bytes read successfully
  // fd - handle for the event interface
  // data - handle to the read data
  // max_elements - maximum number of elements to read
  int vrpn_read_event( int fd, 
                       input_event * data, 
                       int max_elements);
}

#endif // _VRPN_EVENT_H_
