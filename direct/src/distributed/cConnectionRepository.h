// Filename: cConnectionRepository.h
// Created by:  drose (17May04)
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

#ifndef CCONNECTIONREPOSITORY_H
#define CCONNECTIONREPOSITORY_H

#include "directbase.h"
#include "pointerTo.h"

#include "dcbase.h"
#include "dcFile.h"
#include "dcField.h"  // to pick up Python.h
#include "pStatCollector.h"

#ifdef HAVE_NSPR
#include "queuedConnectionManager.h"
#include "connectionWriter.h"
#include "queuedConnectionReader.h"
#include "connection.h"
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
  CConnectionRepository(bool has_owner_view=false);
  ~CConnectionRepository();

  INLINE DCFile &get_dc_file();

  INLINE bool has_owner_view() const;

  INLINE void set_client_datagram(bool client_datagram);
  INLINE bool get_client_datagram() const;

#ifdef HAVE_PYTHON
  INLINE void set_python_repository(PyObject *python_repository);
#endif

#ifdef HAVE_OPENSSL
  void set_connection_http(HTTPChannel *channel);
  SocketStream *get_stream();
#endif
#ifdef HAVE_NSPR
  bool try_connect_nspr(const URLSpec &url);

  INLINE QueuedConnectionManager &get_qcm();
  INLINE ConnectionWriter &get_cw();
  INLINE QueuedConnectionReader &get_qcr();
#endif

#ifdef SIMULATE_NETWORK_DELAY
  void start_delay(double min_delay, double max_delay);
  void stop_delay();
#endif

  bool check_datagram();
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

  bool consider_flush();
  bool flush();

  void disconnect();

  INLINE void set_simulated_disconnect(bool simulated_disconnect);
  INLINE bool get_simulated_disconnect() const;

#ifndef NDEBUG
  INLINE void toggle_verbose();
  INLINE void set_verbose(bool verbose);
  INLINE bool get_verbose() const;
#endif  // NDEBUG

private:
  bool do_check_datagram();
  bool handle_update_field();
  bool handle_update_field_owner();

#ifndef NDEBUG
  void describe_message(ostream &out, const string &prefix, 
                        const string &message_data) const;
#endif  // NDEBUG

#ifdef HAVE_PYTHON
  PyObject *_python_repository;
#endif

#ifdef HAVE_OPENSSL
  SocketStream *_http_conn;
#endif

#ifdef HAVE_NSPR
  QueuedConnectionManager _qcm;
  ConnectionWriter _cw;
  QueuedConnectionReader _qcr;
  PT(Connection) _nspr_conn;
#endif

  DCFile _dc_file;
  bool _has_owner_view;
  bool _client_datagram;
  bool _simulated_disconnect;
  bool _verbose;

  Datagram _dg;
  DatagramIterator _di;

  std::vector<CHANNEL_TYPE>             _msg_channels;
  CHANNEL_TYPE                          _msg_sender;
  unsigned int                          _msg_type;

  static const string _overflow_event_name;

  static PStatCollector _update_pcollector;
};

#include "cConnectionRepository.I"

#endif  // CCONNECTIONREPOSITORY_H
