// Filename: socketStream.h
// Created by:  drose (15Oct02)
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

#ifndef SOCKETSTREAM_H
#define SOCKETSTREAM_H

#include "pandabase.h"
#include "clockObject.h"
#include "config_express.h" // for collect_tcp

// At the present, this module is not compiled if OpenSSL is not
// available, since the only current use for it is to implement
// OpenSSL-defined constructs (like ISocketStream).

#ifdef HAVE_SSL

class Datagram;

////////////////////////////////////////////////////////////////////
//       Class : ISocketStream
// Description : This is a base class for istreams implemented in
//               Panda that read from a (possibly non-blocking)
//               socket.  It adds is_closed(), which can be called
//               after an eof condition to check whether the socket
//               has been closed, or whether more data may be
//               available later.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ISocketStream : public istream {
public:
  INLINE ISocketStream(streambuf *buf);

PUBLISHED:
  bool receive_datagram(Datagram &dg);

  virtual bool is_closed() = 0;
  virtual void close() = 0;

private:
  size_t _data_expected;
  string _data_so_far;
};

////////////////////////////////////////////////////////////////////
//       Class : OSocketStream
// Description : A base class for ostreams that write to a (possibly
//               non-blocking) socket.  It adds is_closed(), which can
//               be called after any write operation fails to check
//               whether the socket has been closed, or whether more
//               data may be sent later.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS OSocketStream : public ostream {
public:
  INLINE OSocketStream(streambuf *buf);

PUBLISHED:
  bool send_datagram(const Datagram &dg);

  virtual bool is_closed() = 0;
  virtual void close() = 0;

  INLINE void set_collect_tcp(bool collect_tcp);
  INLINE bool get_collect_tcp() const;
  INLINE void set_collect_tcp_interval(double interval);
  INLINE double get_collect_tcp_interval() const;

  INLINE bool consider_flush();
  INLINE bool flush();

private:
  bool _collect_tcp;
  double _collect_tcp_interval;
  double _queued_data_start;
};

////////////////////////////////////////////////////////////////////
//       Class : SocketStream
// Description : A base class for iostreams that read and write to a
//               (possibly non-blocking) socket.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS SocketStream : public iostream {
public:
  INLINE SocketStream(streambuf *buf);

PUBLISHED:
  bool receive_datagram(Datagram &dg);
  bool send_datagram(const Datagram &dg);

  virtual bool is_closed() = 0;
  virtual void close() = 0;

  INLINE void set_collect_tcp(bool collect_tcp);
  INLINE bool get_collect_tcp() const;
  INLINE void set_collect_tcp_interval(double interval);
  INLINE double get_collect_tcp_interval() const;

  INLINE bool consider_flush();
  INLINE bool flush();

private:
  size_t _data_expected;
  string _data_so_far;

  bool _collect_tcp;
  double _collect_tcp_interval;
  double _queued_data_start;
};


#include "socketStream.I"

#endif  // HAVE_SSL


#endif


