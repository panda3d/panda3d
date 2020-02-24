/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cConnectionRepository.h
 * @author drose
 * @date 2004-05-17
 */

#ifndef CCONNECTIONREPOSITORY_H
#define CCONNECTIONREPOSITORY_H

#include "directbase.h"
#include "pointerTo.h"

#include "dcbase.h"
#include "dcFile.h"
#include "dcField.h"  // to pick up Python.h
#include "pStatCollector.h"
#include "datagramIterator.h"
#include "clockObject.h"
#include "reMutex.h"
#include "reMutexHolder.h"

#ifdef HAVE_NET
#include "queuedConnectionManager.h"
#include "connectionWriter.h"
#include "queuedConnectionReader.h"
#include "connection.h"
#endif

#ifdef WANT_NATIVE_NET
#include "buffered_datagramconnection.h"
#include "socket_address.h"
#endif

class URLSpec;
class HTTPChannel;
class SocketStream;

/**
 * This class implements the C++ side of the ConnectionRepository object.  In
 * particular, it manages the connection to the server once it has been opened
 * (but does not open it directly).  It manages reading and writing datagrams
 * on the connection and monitoring for unexpected disconnects as well as
 * handling intentional disconnects.
 *
 * Certain server messages, like field updates, are handled entirely within
 * the C++ layer, while server messages that are not understood by the C++
 * layer are returned up to the Python layer for processing.
 */
class CConnectionRepository {
PUBLISHED:
  explicit CConnectionRepository(bool has_owner_view = false,
                                 bool threaded_net = false);
  ~CConnectionRepository();

/*
 * Any methods of this class that acquire _lock (which is most of them) *must*
 * be tagged BLOCKING, to avoid risk of a race condition in Python when
 * running in true threaded mode.  The BLOCKING tag releases the Python GIL
 * during the function call, and we re-acquire it when needed within these
 * functions to call out to Python.  If any functions acquire _lock while
 * already holding the Python GIL, there could be a deadlock between these
 * functions and the ones that are acquiring the GIL while already holding
 * _lock.
 */

  INLINE DCFile &get_dc_file();

  INLINE bool has_owner_view() const;

  INLINE void set_handle_c_updates(bool handle_c_updates);
  INLINE bool get_handle_c_updates() const;

  INLINE void set_client_datagram(bool client_datagram);
  INLINE bool get_client_datagram() const;

  INLINE void set_handle_datagrams_internally(bool handle_datagrams_internally);
  INLINE bool get_handle_datagrams_internally() const;

  void set_tcp_header_size(int tcp_header_size);
  INLINE int get_tcp_header_size() const;

#ifdef HAVE_PYTHON
  INLINE void set_python_repository(PyObject *python_repository);
#endif

#ifdef HAVE_OPENSSL
  BLOCKING void set_connection_http(HTTPChannel *channel);
  BLOCKING SocketStream *get_stream();
#endif
#ifdef HAVE_NET
  BLOCKING bool try_connect_net(const URLSpec &url);

  INLINE QueuedConnectionManager &get_qcm();
  INLINE ConnectionWriter &get_cw();
  INLINE QueuedConnectionReader &get_qcr();
#endif

#ifdef WANT_NATIVE_NET
  BLOCKING bool connect_native(const URLSpec &url);
  INLINE Buffered_DatagramConnection &get_bdc();
#endif

#ifdef SIMULATE_NETWORK_DELAY
  BLOCKING void start_delay(double min_delay, double max_delay);
  BLOCKING void stop_delay();
#endif

  BLOCKING bool check_datagram();

  BLOCKING INLINE void get_datagram(Datagram &dg);
  BLOCKING INLINE void get_datagram_iterator(DatagramIterator &di);
  BLOCKING INLINE CHANNEL_TYPE get_msg_channel(int offset = 0) const;
  BLOCKING INLINE int          get_msg_channel_count() const;
  BLOCKING INLINE CHANNEL_TYPE get_msg_sender() const;
// INLINE unsigned char get_sec_code() const;
  BLOCKING INLINE unsigned int get_msg_type() const;

  INLINE static const std::string &get_overflow_event_name();

  BLOCKING bool is_connected();

  BLOCKING bool send_datagram(const Datagram &dg);

  BLOCKING INLINE void set_want_message_bundling(bool flag);
  BLOCKING INLINE bool get_want_message_bundling() const;

  BLOCKING INLINE void set_in_quiet_zone(bool flag);
  BLOCKING INLINE bool get_in_quiet_zone() const;

  BLOCKING void start_message_bundle();
  BLOCKING INLINE bool is_bundling_messages() const;
  BLOCKING void send_message_bundle(unsigned int channel, unsigned int sender_channel);
  BLOCKING void abandon_message_bundles();
  BLOCKING void bundle_msg(const Datagram &dg);

  BLOCKING bool consider_flush();
  BLOCKING bool flush();

  BLOCKING void disconnect();
  BLOCKING void shutdown();

  INLINE void set_simulated_disconnect(bool simulated_disconnect);
  INLINE bool get_simulated_disconnect() const;

  INLINE void toggle_verbose();
  INLINE void set_verbose(bool verbose);
  INLINE bool get_verbose() const;

  INLINE void set_time_warning(float time_warning);
  INLINE float get_time_warning() const;

private:
  bool do_check_datagram();
  bool handle_update_field();
  bool handle_update_field_owner();

  void describe_message(std::ostream &out, const std::string &prefix,
                        const Datagram &dg) const;

private:
  ReMutex _lock;

#ifdef HAVE_PYTHON
  PyObject *_python_repository;
#endif

#ifdef HAVE_OPENSSL
  SocketStream *_http_conn;
#endif

#ifdef HAVE_NET
  QueuedConnectionManager _qcm;
  ConnectionWriter _cw;
  QueuedConnectionReader _qcr;
  PT(Connection) _net_conn;
#endif

#ifdef WANT_NATIVE_NET
  Buffered_DatagramConnection _bdc;
  bool _native;
#endif

  DCFile _dc_file;
  bool _has_owner_view;
  bool _handle_c_updates;
  bool _client_datagram;
  bool _handle_datagrams_internally;
  int _tcp_header_size;
  bool _simulated_disconnect;
  bool _verbose;
  bool _in_quiet_zone;
  float _time_warning;

  Datagram _dg;
  DatagramIterator _di;

  std::vector<CHANNEL_TYPE>             _msg_channels;
  CHANNEL_TYPE                          _msg_sender;
  unsigned int                          _msg_type;

  static const std::string _overflow_event_name;

  bool _want_message_bundling;
  unsigned int _bundling_msgs;
  typedef std::vector< std::string > BundledMsgVector;
  BundledMsgVector _bundle_msgs;

  static PStatCollector _update_pcollector;
};

#include "cConnectionRepository.I"

#endif  // CCONNECTIONREPOSITORY_H
