// Filename: pStatClientImpl.h
// Created by:  drose (23Dec04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PSTATCLIENTIMPL_H
#define PSTATCLIENTIMPL_H

#include "pandabase.h"

// This class doesn't exist at all unless DO_PSTATS is defined.
#ifdef DO_PSTATS

#include "pStatFrameData.h"
#include "connectionManager.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"

#include "clockObject.h"
#include "luse.h"
#include "pmap.h"

class PStatClient;
class PStatServerControlMessage;
class PStatCollector;
class PStatCollectorDef;
class PStatThread;

////////////////////////////////////////////////////////////////////
//       Class : PStatClientImpl
// Description : This class is the implementation of the actual
//               PStatClient class (which is just for interface).  All
//               of the stuff to manage sending stats up to the server
//               is handled by this class.
//
//               This separation between PStatClient and
//               PStatClientImpl allows the global PStatClient to be
//               constructed at static init time, without having to
//               consult any config variables at that time.  We don't
//               actually do any real work until someone explicitly
//               calls PStatClient::connect().
//
//               This class doesn't exist at all unless DO_PSTATS is
//               defined.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatClientImpl : public ConnectionManager {
public:
  PStatClientImpl(PStatClient *client);
  ~PStatClientImpl();

  INLINE void set_client_name(const string &name);
  INLINE string get_client_name() const;
  INLINE void set_max_rate(float rate);
  INLINE float get_max_rate() const;

  INLINE const ClockObject &get_clock() const;

  INLINE void client_main_tick();
  bool client_connect(string hostname, int port);
  void client_disconnect();
  INLINE bool client_is_connected() const;

  INLINE void client_resume_after_pause();

  void new_frame(int thread_index);

private:
  void transmit_frame_data(int thread_index);

  void transmit_control_data();

  // Stats collecting stuff
  ClockObject _clock;

  // Networking stuff
  string get_hostname();
  void send_hello();
  void report_new_collectors();
  void report_new_threads();
  void handle_server_control_message(const PStatServerControlMessage &message);

  virtual void connection_reset(const PT(Connection) &connection, 
                                PRErrorCode errcode);

  PStatClient *_client;

  bool _is_connected;
  bool _got_udp_port;

  NetAddress _server;
  QueuedConnectionReader _reader;
  ConnectionWriter _writer;

  PT(Connection) _tcp_connection;
  PT(Connection) _udp_connection;

  int _collectors_reported;
  int _threads_reported;

  string _hostname;
  string _client_name;
  float _max_rate;

  float _tcp_count_factor;
  float _udp_count_factor;
  unsigned int _tcp_count;
  unsigned int _udp_count;
};

#include "pStatClientImpl.I"

#endif  // DO_PSTATS

#endif

