/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatReader.h
 * @author drose
 * @date 2000-07-09
 */

#ifndef PSTATREADER_H
#define PSTATREADER_H

#include "pandatoolbase.h"

#include "pStatClientData.h"
#include "pStatMonitor.h"

#include "connectionReader.h"
#include "connectionWriter.h"
#include "referenceCount.h"
#include "circBuffer.h"

class PStatServer;
class PStatMonitor;
class PStatClientControlMessage;
class PStatFrameData;

// This is the maximum number of frame records that will be queued up from
// this particular client between processing loops.
static const int queued_frame_records = 500;

/**
 * This is the class that does all the work for handling communications from a
 * single Panda client.  It reads sockets received from the client and boils
 * them down into PStatData.
 */
class PStatReader : public ConnectionReader {
public:
  PStatReader(PStatServer *manager, PStatMonitor *monitor);
  ~PStatReader();

  void close();

  void set_tcp_connection(Connection *tcp_connection);
  void lost_connection();
  void idle();

  PStatMonitor *get_monitor();

private:
  std::string get_hostname();
  void send_hello();

  virtual void receive_datagram(const NetDatagram &datagram);

  void handle_client_control_message(const PStatClientControlMessage &message);
  void handle_client_udp_data(const Datagram &datagram);
  void dequeue_frame_data();

private:
  PStatServer *_manager;
  PT(PStatMonitor) _monitor;
  ConnectionWriter _writer;

  PT(Connection) _tcp_connection;
  PT(Connection) _udp_connection;
  int _udp_port;

  PT(PStatClientData) _client_data;

  std::string _hostname;

  class FrameData {
  public:
    int _thread_index;
    int _frame_number;
    PStatFrameData *_frame_data;
  };
  typedef CircBuffer<FrameData, queued_frame_records> QueuedFrameData;
  QueuedFrameData _queued_frame_data;
};

#endif
