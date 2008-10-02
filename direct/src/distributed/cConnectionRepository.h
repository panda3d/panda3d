// Filename: cConnectionRepository.h
// Created by:  drose (17May04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////
//       Class : CConnectionRepository
// Description : This class implements the C++ side of the
//               ConnectionRepository object.  In particular, it
//               manages the connection to the server once it has been
//               opened (but does not open it directly).  It manages
//               reading and writing datagrams on the connection and
//               monitoring for unexpected disconnects as well as
//               handling intentional disconnects.
//
//               Certain server messages, like field updates, are
//               handled entirely within the C++ layer, while server
//               messages that are not understood by the C++ layer are
//               returned up to the Python layer for processing.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT CConnectionRepository {
PUBLISHED:
  CConnectionRepository(bool has_owner_view = false);
  ~CConnectionRepository();

  INLINE DCFile &get_dc_file();

  INLINE bool has_owner_view() const;

  INLINE void set_handle_c_updates(bool handle_c_updates);
  INLINE bool get_handle_c_updates() const;

  INLINE void set_client_datagram(bool client_datagram);
  INLINE bool get_client_datagram() const;

#ifdef HAVE_PYTHON
  INLINE void set_python_repository(PyObject *python_repository);
#endif

#ifdef HAVE_OPENSSL
  void set_connection_http(HTTPChannel *channel);
  SocketStream *get_stream();
#endif
#ifdef HAVE_NET
  bool try_connect_net(const URLSpec &url);

  INLINE QueuedConnectionManager &get_qcm();
  INLINE ConnectionWriter &get_cw();
  INLINE QueuedConnectionReader &get_qcr();
#endif

#ifdef WANT_NATIVE_NET
  bool connect_native(const URLSpec &url);
  INLINE Buffered_DatagramConnection &get_bdc();
#endif

#ifdef SIMULATE_NETWORK_DELAY
  void start_delay(double min_delay, double max_delay);
  void stop_delay();
#endif

  bool check_datagram();
#ifdef HAVE_PYTHON
#ifdef WANT_NATIVE_NET
    bool check_datagram_ai(PyObject *PycallBackFunction);
    bool network_based_reader_and_yielder(PyObject *PycallBackFunction,ClockObject &clock, float returnBy);
#endif
#endif
    
  INLINE void get_datagram(Datagram &dg);
  INLINE void get_datagram_iterator(DatagramIterator &di);
  INLINE CHANNEL_TYPE get_msg_channel(int offset = 0) const;
  INLINE int          get_msg_channel_count() const;
  INLINE CHANNEL_TYPE get_msg_sender() const;
//  INLINE unsigned char get_sec_code() const;
  INLINE unsigned int get_msg_type() const;

  INLINE static const string &get_overflow_event_name();

  bool is_connected();

  bool send_datagram(const Datagram &dg);

  INLINE void set_want_message_bundling(bool flag);
  INLINE bool get_want_message_bundling() const;

  void start_message_bundle();
  INLINE bool is_bundling_messages() const;
  void send_message_bundle(unsigned int channel, unsigned int sender_channel);
  void abandon_message_bundles();
  void bundle_msg(const Datagram &dg);

  bool consider_flush();
  bool flush();

  void disconnect();

  INLINE void set_simulated_disconnect(bool simulated_disconnect);
  INLINE bool get_simulated_disconnect() const;

  INLINE void toggle_verbose();
  INLINE void set_verbose(bool verbose);
  INLINE bool get_verbose() const;

private:
#ifdef HAVE_PYTHON
#ifdef WANT_NATIVE_NET
    bool handle_update_field_ai(PyObject *doId2do);
#endif
#endif


  bool do_check_datagram();
  bool handle_update_field();
  bool handle_update_field_owner();

  void describe_message(ostream &out, const string &prefix, 
                        const Datagram &dg) const;

private:
  ReMutex _lock;

#ifdef HAVE_PYTHON
  PyObject *_python_repository;
  PyObject *_python_ai_datagramiterator;
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
  bool _simulated_disconnect;
  bool _verbose;

  Datagram _dg;
  DatagramIterator _di;

  std::vector<CHANNEL_TYPE>             _msg_channels;
  CHANNEL_TYPE                          _msg_sender;
  unsigned int                          _msg_type;

  static const string _overflow_event_name;

  bool _want_message_bundling;
  unsigned int _bundling_msgs;
  typedef std::vector< string > BundledMsgVector;
  BundledMsgVector _bundle_msgs;

  static PStatCollector _update_pcollector;
};

#include "cConnectionRepository.I"

#endif  // CCONNECTIONREPOSITORY_H
