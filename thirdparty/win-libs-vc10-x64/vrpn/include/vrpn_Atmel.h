// -*- Mode:C++ -*-
/***************************************************************************************************/
/*                                                                                                 */
/* Copyright (C) 2004 Bauhaus University Weimar                                                    */
/* Released into the public domain on 6/23/2007 as part of the VRPN project                        */
/* by Jan P. Springer.                                                                             */
/*                                                                                                 */
/***************************************************************************************************/
/*                                                                                                 */
/*  module     :  vrpn_Atmel.h                                                                     */
/*  project    :  vrpn_Avango                                                                      */
/*  description:  server for microcontroller board based on Atmel's ATMEGA32                       */
/*                hardware developed by Albotronic: www.albotronic.de                              */
/*                                                                                                 */
/***************************************************************************************************/

#ifndef VRPN_ATMEL
#define VRPN_ATMEL

/***************************************************************************************************/
/* compiler flags */

/* serial lib
   default is atmellib 
   can be changed to vrpn_Serial (problems when dropping connection from client side,
     read blocks for unknown reason
*/
//#define VRPN_ATMEL_SERIAL_VRPN

/* debug flags */
#define VRPN_ATMEL_VERBOSE
//#define VRPN_ATMEL_TIME_MEASURE


/***************************************************************************************************/
/* vrpn atmellib error values -> reported to the client */

#define VRPN_ATMEL_ERROR_READING_IN               -21
#define VRPN_ATMEL_ERROR_WRITING_DOWN             -22
#define VRPN_ATMEL_ERROR_OUT_OF_RANGE             -23
#define VRPN_ATMEL_ERROR_NOT_WRITABLE             -24

#define VRPN_ATMEL_MODE_RO                        101
#define VRPN_ATMEL_MODE_RW                        102
#define VRPN_ATMEL_MODE_WO                        103
#define VRPN_ATMEL_MODE_NA                        104

#define VRPN_ATMEL_STATUS_WAITING_FOR_CONNECTION  201
#define VRPN_ATMEL_STATUS_RUNNING                 202
#define VRPN_ATMEL_STATUS_ERROR                  -200

#define VRPN_ATMEL_CHANNEL_NOT_VALID                -1

#define VRPN_ATMEL_ALIVE_TIME_LOOK_SEC               3 
#define VRPN_ATMEL_ALIVE_TIME_LOOK_USEC              0
#define VRPN_ATMEL_ALIVE_INTERVAL_SEC                1

/***************************************************************************************************/
/* system includes */
#include <vector>

/***************************************************************************************************/
/* project includes */
#include "vrpn_Analog.h"
#include "vrpn_Analog_Output.h"

#ifdef VRPN_ATMEL_SERIAL_VRPN
#  include "vrpn_Serial.h"
#endif

/***************************************************************************************************/
class VRPN_API vrpn_Atmel : public vrpn_Analog_Server, vrpn_Analog_Output_Server {

public:
	
  static vrpn_Atmel *
  Create(char* name, vrpn_Connection *c,
         const char *port="/dev/ttyS0/", long baud=9600,
         int channel_count=0,
         int * channel_mode=NULL);

  ~vrpn_Atmel();

  void mainloop();  

private:
  
  // constructor
  vrpn_Atmel(char* name, vrpn_Connection *c,  int fd);
             
private:

  void init_channel_mode(int * channel_mode);
 
  // do the serial communication in mainloop
  bool mainloop_serial_io();
 
  // things which have to be done when a new connection has been established
  bool handle_new_connection();

  // enable connection reliability checking by additional select
  bool Check_Serial_Alive();

private:
 
  // indicator for current status of server
  // one of the predefined value: VRPN_ATMEL_STATUS_*
  int _status;

  // time of report
  struct timeval timestamp;
  
  int serial_fd;

  // indicator for io-mode of the channels
  // one of the predefined values: VRPN_ATMEL_MODE_* 
  std::vector<int> _channel_mode;

  // helper for Serial_Alive: do the stuff not in every mainloop -> _time_alive
  struct timeval _time_alive;
};

#endif // #ifndef VRPN_ATMEL
