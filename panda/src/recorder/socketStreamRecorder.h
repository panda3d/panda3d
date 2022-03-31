/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socketStreamRecorder.h
 * @author drose
 * @date 2004-01-28
 */

#ifndef SOCKETSTREAMRECORDER_H
#define SOCKETSTREAMRECORDER_H

#include "pandabase.h"
#include "socketStream.h"
#include "recorderBase.h"
#include "pdeque.h"

class BamReader;
class BamWriter;
class FactoryParams;
class DatagramIterator;

// At the present, this module is not compiled if OpenSSL is not available,
// since in that case socketStream.h is not compiled either.

#ifdef HAVE_OPENSSL

/**
 * Records any data received from the indicated socket stream.  On playback,
 * it will act as if the incoming data is coming over the wire again even if
 * an actual connection is not available.
 *
 * Outbound data will not be recorded, but will be sent straight through to
 * the socket if it is connected, or silently ignored if it is not.
 */
class EXPCL_PANDA_RECORDER SocketStreamRecorder : public RecorderBase,
                                                  public ReferenceCount {
PUBLISHED:
  INLINE SocketStreamRecorder();
  INLINE explicit SocketStreamRecorder(SocketStream *stream, bool owns_stream);
  INLINE ~SocketStreamRecorder();

  bool receive_datagram(Datagram &dg);
  INLINE bool send_datagram(const Datagram &dg);

  INLINE bool is_closed();
  INLINE void close();

  INLINE void set_collect_tcp(bool collect_tcp);
  INLINE bool get_collect_tcp() const;
  INLINE void set_collect_tcp_interval(double interval);
  INLINE double get_collect_tcp_interval() const;

  INLINE bool consider_flush();
  INLINE bool flush();

public:
  virtual void record_frame(BamWriter *manager, Datagram &dg);
  virtual void play_frame(DatagramIterator &scan, BamReader *manager);

private:
  SocketStream *_stream;
  bool _owns_stream;

  typedef pdeque<Datagram> Data;
  Data _data;
  bool _closed;

public:
  static void register_with_read_factory();
  virtual void write_recorder(BamWriter *manager, Datagram &dg);

  INLINE virtual int get_ref_count() const final { return ReferenceCount::get_ref_count(); };
  INLINE virtual void ref() const final { ReferenceCount::ref(); };
  INLINE virtual bool unref() const final { return ReferenceCount::unref(); };

protected:
  static RecorderBase *make_recorder(const FactoryParams &params);
  void fillin_recorder(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RecorderBase::init_type();
    register_type(_type_handle, "SocketStreamRecorder",
                  RecorderBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "socketStreamRecorder.I"

#endif  // HAVE_OPENSSL

#endif  // SOCKETSTREAMRECORDER_H
