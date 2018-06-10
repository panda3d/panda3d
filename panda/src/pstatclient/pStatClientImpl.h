/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClientImpl.h
 * @author drose
 * @date 2004-12-23
 */

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

#include "trueClock.h"
#include "pmap.h"

class PStatClient;
class PStatServerControlMessage;
class PStatCollector;
class PStatCollectorDef;
class PStatThread;

/**
 * This class is the implementation of the actual PStatClient class (which is
 * just for interface).  All of the stuff to manage sending stats up to the
 * server is handled by this class.
 *
 * This separation between PStatClient and PStatClientImpl allows the global
 * PStatClient to be constructed at static init time, without having to
 * consult any config variables at that time.  We don't actually do any real
 * work until someone explicitly calls PStatClient::connect().
 *
 * This class doesn't exist at all unless DO_PSTATS is defined.
 */
class EXPCL_PANDA_PSTATCLIENT PStatClientImpl : public ConnectionManager {
public:
  PStatClientImpl(PStatClient *client);
  ~PStatClientImpl();

  INLINE void set_client_name(const std::string &name);
  INLINE std::string get_client_name() const;
  INLINE void set_max_rate(double rate);
  INLINE double get_max_rate() const;

  INLINE double get_real_time() const;

  INLINE void client_main_tick();
  bool client_connect(std::string hostname, int port);
  void client_disconnect();
  INLINE bool client_is_connected() const;

  INLINE void client_resume_after_pause();

  void new_frame(int thread_index);
  void add_frame(int thread_index, const PStatFrameData &frame_data);

private:
  void transmit_frame_data(int thread_index, int frame_number,
                           const PStatFrameData &frame_data);

  void transmit_control_data();

  TrueClock *_clock;
  double _delta;
  double _last_frame;

  // Networking stuff
  std::string get_hostname();
  void send_hello();
  void report_new_collectors();
  void report_new_threads();
  void handle_server_control_message(const PStatServerControlMessage &message);

  virtual void connection_reset(const PT(Connection) &connection,
                                bool okflag);

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

  std::string _hostname;
  std::string _client_name;
  double _max_rate;

  double _tcp_count_factor;
  double _udp_count_factor;
  unsigned int _tcp_count;
  unsigned int _udp_count;
};

#include "pStatClientImpl.I"

#endif  // DO_PSTATS

#endif
