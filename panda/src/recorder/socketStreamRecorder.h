// Filename: socketStreamRecorder.h
// Created by:  drose (28Jan04)
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

#ifndef SOCKETSTREAMRECORDER_H
#define SOCKETSTREAMRECORDER_H

#include "pandabase.h"
#include "socketStream.h"
#include "pdeque.h"

class BamReader;
class BamWriter;
class FactoryParams;

// At the present, this module is not compiled if OpenSSL is not
// available, since in that case socketStream.h is not compiled
// either.

#ifdef HAVE_SSL

////////////////////////////////////////////////////////////////////
//       Class : SocketStreamRecorder
// Description : Records any data received from the indicated socket
//               stream.  On playback, it will act as if the incoming
//               data is coming over the wire again even if an actual
//               connection is not available.
//
//               Outbound data will not be recorded, but will be sent
//               straight through to the socket if it is connected, or
//               silently ignored if it is not.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SocketStreamRecorder : public RecorderBase {
PUBLISHED:
  INLINE SocketStreamRecorder();
  INLINE SocketStreamRecorder(SocketStream *stream, bool owns_stream);
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

#endif  // HAVE_SSL

#endif  // SOCKETSTREAMRECORDER_H
