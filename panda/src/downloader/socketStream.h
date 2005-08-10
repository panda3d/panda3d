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
#include "datagram.h"
#include "pdeque.h"

// At the present, this module is not compiled if OpenSSL is not
// available, since the only current use for it is to implement
// OpenSSL-defined constructs (like ISocketStream).

#ifdef HAVE_SSL

////////////////////////////////////////////////////////////////////
//       Class : SSReader
// Description : An internal class for reading from a socket stream.
//               This serves as a base class for both ISocketStream
//               and SocketStream; its purpose is to minimize
//               redundant code between them.  Do not use it directly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS SSReader {
public:
  INLINE SSReader(istream *stream);
  virtual ~SSReader();

PUBLISHED:
  INLINE bool receive_datagram(Datagram &dg);

  virtual bool is_closed() = 0;
  virtual void close() = 0;

private:
  bool do_receive_datagram(Datagram &dg);

  istream *_istream;
  size_t _data_expected;
  string _data_so_far;

#ifdef SIMULATE_NETWORK_DELAY
PUBLISHED:
  void start_delay(double min_delay, double max_delay);
  void stop_delay();

private:
  void delay_datagram(const Datagram &datagram);
  bool get_delayed(Datagram &datagram);

  class DelayedDatagram {
  public:
    double _reveal_time;
    Datagram _datagram;
  };
    
  typedef pdeque<DelayedDatagram> Delayed;
  Delayed _delayed;
  bool _delay_active;
  double _min_delay, _delay_variance;

#endif  // SIMULATE_NETWORK_DELAY
};

////////////////////////////////////////////////////////////////////
//       Class : SSWriter
// Description : An internal class for writing to a socket stream.
//               This serves as a base class for both OSocketStream
//               and SocketStream; its purpose is to minimize
//               redundant code between them.  Do not use it directly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS SSWriter {
public:
  INLINE SSWriter(ostream *stream);
  virtual ~SSWriter();

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
  ostream *_ostream;
  bool _collect_tcp;
  double _collect_tcp_interval;
  double _queued_data_start;
};

////////////////////////////////////////////////////////////////////
//       Class : ISocketStream
// Description : This is a base class for istreams implemented in
//               Panda that read from a (possibly non-blocking)
//               socket.  It adds is_closed(), which can be called
//               after an eof condition to check whether the socket
//               has been closed, or whether more data may be
//               available later.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ISocketStream : public istream, public SSReader {
public:
  INLINE ISocketStream(streambuf *buf);

PUBLISHED:
  virtual bool is_closed() = 0;
  virtual void close() = 0;
};

////////////////////////////////////////////////////////////////////
//       Class : OSocketStream
// Description : A base class for ostreams that write to a (possibly
//               non-blocking) socket.  It adds is_closed(), which can
//               be called after any write operation fails to check
//               whether the socket has been closed, or whether more
//               data may be sent later.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS OSocketStream : public ostream, public SSWriter {
public:
  INLINE OSocketStream(streambuf *buf);

PUBLISHED:
  virtual bool is_closed() = 0;
  virtual void close() = 0;

  INLINE bool flush();
};

////////////////////////////////////////////////////////////////////
//       Class : SocketStream
// Description : A base class for iostreams that read and write to a
//               (possibly non-blocking) socket.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS SocketStream : public iostream, public SSReader, public SSWriter {
public:
  INLINE SocketStream(streambuf *buf);

PUBLISHED:
  virtual bool is_closed() = 0;
  virtual void close() = 0;

  INLINE bool flush();
};


#include "socketStream.I"

#endif  // HAVE_SSL


#endif


