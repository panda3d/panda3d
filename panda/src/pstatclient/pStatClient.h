// Filename: pStatClient.h
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATCLIENT_H
#define PSTATCLIENT_H

#include <pandabase.h>

#include "pStatFrameData.h"

#include <clockObject.h>
#include <vector_int.h>
#include <luse.h>

#ifdef DO_PSTATS
#include <connectionManager.h>
#include <queuedConnectionReader.h>
#include <connectionWriter.h>
#include <netAddress.h>
#endif

class PStatServerControlMessage;
class PStatCollector;
class PStatCollectorDef;
class PStatThread;

////////////////////////////////////////////////////////////////////
// 	 Class : PStatClient
// Description : Manages the communications to report statistics via a
//               network connection to a remote PStatServer.
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
  INLINE void set_max_rate(double rate);
  INLINE double get_max_rate() const;

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

  static void main_tick();
  static PStatClient *get_global_pstats();

PUBLISHED:
  INLINE static bool connect(const string &hostname = string(), int port = -1);
  INLINE static void disconnect();
  INLINE static bool is_connected();

private:
  bool ns_connect(string hostname, int port);
  void ns_disconnect();
  bool ns_is_connected() const;

  PStatCollector make_collector(int parent_index, string fullname);
  PStatCollector make_collector(int parent_index, const string &fullname,
				const RGBColorf &suggested_color, int sort);
  PStatThread make_thread(const string &name);

  void start(int collector_index, int thread_index, double as_of);
  void stop(int collector_index, int thread_index, double as_of);

  void new_frame(int thread_index);
  void transmit_frame_data(int thread_index);

  void transmit_control_data();

  // Stats collecting stuff
  ClockObject _clock;
  
  typedef map<string, int> ThingsByName;
  ThingsByName _threads_by_name;

  class Collector {
  public:
    PStatCollectorDef *_def;
    vector_int _nested_count;
    ThingsByName _children;
  };
  typedef vector<Collector> Collectors;
  Collectors _collectors;

  class Thread {
  public:
    string _name;
    PStatFrameData _frame_data;
    bool _is_active;
    int _frame_number;
    double _last_packet;
  };

  typedef vector<Thread> Threads;
  Threads _threads;

private:
  // Networking stuff
  string get_hostname();
  void send_hello();
  void report_new_collectors();
  void report_new_threads();
  void handle_server_control_message(const PStatServerControlMessage &message);

  virtual void connection_reset(const PT(Connection) &connection);

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
  double _max_rate;

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

  static void main_tick() { }
};

#endif  // DO_PSTATS

#endif

