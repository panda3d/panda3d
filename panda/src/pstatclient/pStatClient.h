// Filename: pStatClient.h
// Created by:  drose (09Jul00)
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

#ifndef PSTATCLIENT_H
#define PSTATCLIENT_H

#include "pandabase.h"

#include "pStatFrameData.h"

#include "clockObject.h"
#include "luse.h"
#include "pmap.h"

#ifdef HAVE_NET
#include "connectionManager.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#endif

class PStatServerControlMessage;
class PStatCollector;
class PStatCollectorDef;
class PStatThread;

////////////////////////////////////////////////////////////////////
//       Class : PStatClient
// Description : Manages the communications to report statistics via a
//               network connection to a remote PStatServer.
//
//               Normally, there is only one PStatClient in the world,
//               although it is possible to have multiple PStatClients
//               if extraordinary circumstances require in.  Since
//               each PStatCollector registers itself with the
//               PStatClient when it is created, having multiple
//               PStatClients requires special care when constructing
//               the various PStatCollectors.
//
//               If DO_PSTATS is not defined, we don't want to use
//               stats at all.  This class is therefore defined as a
//               stub class.
////////////////////////////////////////////////////////////////////
#ifdef DO_PSTATS
class EXPCL_PANDA PStatClient : public ConnectionManager {
public:
  PStatClient();
  ~PStatClient();

  INLINE void set_client_name(const string &name);
  INLINE string get_client_name() const;
  INLINE void set_max_rate(float rate);
  INLINE float get_max_rate() const;

  int get_num_collectors() const;
  PStatCollector get_collector(int index) const;
  const PStatCollectorDef &get_collector_def(int index) const;
  string get_collector_name(int index) const;
  string get_collector_fullname(int index) const;

  int get_num_threads() const;
  PStatThread get_thread(int index) const;
  string get_thread_name(int index) const;

  const ClockObject &get_clock() const;
  PStatThread get_main_thread() const;

PUBLISHED:
  INLINE static bool connect(const string &hostname = string(), int port = -1);
  INLINE static void disconnect();
  INLINE static bool is_connected();

  INLINE static void resume_after_pause();

public:
  static PStatClient *get_global_pstats();

  static void main_tick();
  void client_main_tick();
  bool client_connect(string hostname, int port);
  void client_disconnect();
  bool client_is_connected() const;

  void client_resume_after_pause();

private:
  PStatCollector make_collector_with_relname(int parent_index, string relname);
  PStatCollector make_collector_with_name(int parent_index, const string &name);
  PStatThread make_thread(const string &name);

  bool is_active(int collector_index, int thread_index) const;
  bool is_started(int collector_index, int thread_index) const;

  void start(int collector_index, int thread_index);
  void start(int collector_index, int thread_index, float as_of);
  void stop(int collector_index, int thread_index);
  void stop(int collector_index, int thread_index, float as_of);

  void clear_level(int collector_index, int thread_index);
  void set_level(int collector_index, int thread_index, float level);
  void add_level(int collector_index, int thread_index, float increment);
  float get_level(int collector_index, int thread_index) const;

  void new_frame(int thread_index);
  void transmit_frame_data(int thread_index);

  void transmit_control_data();

  // Stats collecting stuff
  ClockObject _clock;

  typedef pmap<string, int> ThingsByName;
  ThingsByName _threads_by_name;

  // This is for the data that is per-collector, per-thread.  A vector
  // of these is stored in each Collector object, below, indexed by
  // thread index.
  class PerThreadData {
  public:
    PerThreadData();
    bool _has_level;
    float _level;
    int _nested_count;
  };
  typedef pvector<PerThreadData> PerThread;

  // This is where the meat of the Collector data is stored.  (All the
  // stuff in PStatCollector and PStatCollectorDef is just fluff.)
  class Collector {
  public:
    PStatCollectorDef *_def;
    ThingsByName _children;

    PerThread _per_thread;
  };
  typedef pvector<Collector> Collectors;
  Collectors _collectors;

  // This defines a single thread, i.e. a separate chain of execution,
  // independent of all other threads.  Timing and level data are
  // maintained separately for each thread.
  class Thread {
  public:
    string _name;
    PStatFrameData _frame_data;
    bool _is_active;
    int _frame_number;
    float _next_packet;
  };
  typedef pvector<Thread> Threads;
  Threads _threads;


private:
  // Networking stuff
  string get_hostname();
  void send_hello();
  void report_new_collectors();
  void report_new_threads();
  void handle_server_control_message(const PStatServerControlMessage &message);

  virtual void connection_reset(const PT(Connection) &connection, 
                                PRErrorCode errcode);

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

  static PStatClient *_global_pstats;
  friend class PStatCollector;
  friend class PStatThread;
};

#include "pStatClient.I"

#else  // DO_PSTATS

class EXPCL_PANDA PStatClient {
public:
  PStatClient() { }
  ~PStatClient() { }

PUBLISHED:
  INLINE static bool connect(const string & = string(), int = -1) { return false; }
  INLINE static void disconnect() { }
  INLINE static bool is_connected() { return false; }
  INLINE static void resume_after_pause() { }

public:
  static void main_tick() { }
};

#endif  // DO_PSTATS

#endif

