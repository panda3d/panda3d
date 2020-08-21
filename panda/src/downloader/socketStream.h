/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socketStream.h
 * @author drose
 * @date 2002-10-15
 */

#ifndef SOCKETSTREAM_H
#define SOCKETSTREAM_H

#include "pandabase.h"
#include "trueClock.h"
#include "config_express.h" // for collect_tcp
#include "datagram.h"
#include "pdeque.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "vector_uchar.h"

// At the present, this module is not compiled if OpenSSL is not available,
// since the only current use for it is to implement OpenSSL-defined
// constructs (like ISocketStream).

#ifdef HAVE_OPENSSL

class HTTPChannel;

/**
 * An internal class for reading from a socket stream.  This serves as a base
 * class for both ISocketStream and SocketStream; its purpose is to minimize
 * redundant code between them.  Do not use it directly.
 */
class EXPCL_PANDA_DOWNLOADER SSReader {
public:
  SSReader(std::istream *stream);
  virtual ~SSReader();

PUBLISHED:
  INLINE bool receive_datagram(Datagram &dg);

  virtual bool is_closed() = 0;
  virtual void close() = 0;

  INLINE void set_tcp_header_size(int tcp_header_size);
  INLINE int get_tcp_header_size() const;

private:
  bool do_receive_datagram(Datagram &dg);

  std::istream *_istream;
  size_t _data_expected;
  vector_uchar _data_so_far;
  int _tcp_header_size;

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

/**
 * An internal class for writing to a socket stream.  This serves as a base
 * class for both OSocketStream and SocketStream; its purpose is to minimize
 * redundant code between them.  Do not use it directly.
 */
class EXPCL_PANDA_DOWNLOADER SSWriter {
public:
  SSWriter(std::ostream *stream);
  virtual ~SSWriter();

PUBLISHED:
  bool send_datagram(const Datagram &dg);

  virtual bool is_closed() = 0;
  virtual void close() = 0;

  INLINE void set_collect_tcp(bool collect_tcp);
  INLINE bool get_collect_tcp() const;
  INLINE void set_collect_tcp_interval(double interval);
  INLINE double get_collect_tcp_interval() const;

  INLINE void set_tcp_header_size(int tcp_header_size);
  INLINE int get_tcp_header_size() const;

  INLINE bool consider_flush();
  INLINE bool flush();

private:
  std::ostream *_ostream;
  bool _collect_tcp;
  double _collect_tcp_interval;
  double _queued_data_start;
  int _tcp_header_size;
};

/**
 * This is a base class for istreams implemented in Panda that read from a
 * (possibly non-blocking) socket.  It adds is_closed(), which can be called
 * after an eof condition to check whether the socket has been closed, or
 * whether more data may be available later.
 */
class EXPCL_PANDA_DOWNLOADER ISocketStream : public std::istream, public SSReader {
public:
  INLINE ISocketStream(std::streambuf *buf);
  virtual ~ISocketStream();

#if _MSC_VER >= 1800
  INLINE ISocketStream(const ISocketStream &copy) = delete;
#endif

PUBLISHED:
  enum ReadState {
    RS_initial,
    RS_reading,
    RS_complete,
    RS_error,
  };

  virtual bool is_closed() = 0;
  virtual void close() = 0;
  virtual ReadState get_read_state() = 0;

protected:
  HTTPChannel *_channel;

private:
  friend class HTTPChannel;
};

/**
 * A base class for ostreams that write to a (possibly non-blocking) socket.
 * It adds is_closed(), which can be called after any write operation fails to
 * check whether the socket has been closed, or whether more data may be sent
 * later.
 */
class EXPCL_PANDA_DOWNLOADER OSocketStream : public std::ostream, public SSWriter {
public:
  INLINE OSocketStream(std::streambuf *buf);

#if _MSC_VER >= 1800
  INLINE OSocketStream(const OSocketStream &copy) = delete;
#endif

PUBLISHED:
  virtual bool is_closed() = 0;
  virtual void close() = 0;

  INLINE bool flush();
};

/**
 * A base class for iostreams that read and write to a (possibly non-blocking)
 * socket.
 */
class EXPCL_PANDA_DOWNLOADER SocketStream : public std::iostream, public SSReader, public SSWriter {
public:
  INLINE SocketStream(std::streambuf *buf);

#if _MSC_VER >= 1800
  INLINE SocketStream(const SocketStream &copy) = delete;
#endif

PUBLISHED:
  virtual bool is_closed() = 0;
  virtual void close() = 0;

  INLINE void set_tcp_header_size(int tcp_header_size);
  INLINE int get_tcp_header_size() const;

  INLINE bool flush();
};


#include "socketStream.I"

#endif  // HAVE_OPENSSL


#endif
