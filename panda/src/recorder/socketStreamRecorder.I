// Filename: socketStreamRecorder.I
// Created by:  drose (28Jan04)
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


////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SocketStreamRecorder::
SocketStreamRecorder() :
  _stream(NULL),
  _owns_stream(false),
  _closed(true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SocketStreamRecorder::
SocketStreamRecorder(SocketStream *stream, bool owns_stream) :
  _stream(stream),
  _owns_stream(owns_stream),
  _closed(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SocketStreamRecorder::
~SocketStreamRecorder() {
  if (_owns_stream) {
    delete _stream;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::send_datagram
//       Access: Public
//  Description: See SocketStream::send_datagram().
////////////////////////////////////////////////////////////////////
bool SocketStreamRecorder::
send_datagram(const Datagram &dg) {
  if (_stream != (SocketStream *)NULL) {
    return _stream->send_datagram(dg);
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::is_closed
//       Access: Published
//  Description: See SocketStream::is_closed().
////////////////////////////////////////////////////////////////////
INLINE bool SocketStreamRecorder::
is_closed() {
  if (_stream != (SocketStream *)NULL) {
    return _stream->is_closed();
  }
  return is_playing() && _closed;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::close
//       Access: Published
//  Description: See SocketStream::close().
////////////////////////////////////////////////////////////////////
INLINE void SocketStreamRecorder::
close() {
  if (_stream != (SocketStream *)NULL) {
    _stream->close();
  }
  _closed = true;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::set_collect_tcp
//       Access: Published
//  Description: See SocketStream::set_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE void SocketStreamRecorder::
set_collect_tcp(bool collect_tcp) {
  if (_stream != (SocketStream *)NULL) {
    _stream->set_collect_tcp(collect_tcp);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::get_collect_tcp
//       Access: Published
//  Description: See SocketStream::get_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE bool SocketStreamRecorder::
get_collect_tcp() const {
  if (_stream != (SocketStream *)NULL) {
    return _stream->get_collect_tcp();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::set_collect_tcp_interval
//       Access: Published
//  Description: See SocketStream::set_collect_tcp_interval().
////////////////////////////////////////////////////////////////////
INLINE void SocketStreamRecorder::
set_collect_tcp_interval(double interval) {
  if (_stream != (SocketStream *)NULL) {
    _stream->set_collect_tcp_interval(interval);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::get_collect_tcp_interval
//       Access: Published
//  Description: See SocketStream::get_collect_tcp_interval().
////////////////////////////////////////////////////////////////////
INLINE double SocketStreamRecorder::
get_collect_tcp_interval() const {
  if (_stream != (SocketStream *)NULL) {
    return _stream->get_collect_tcp_interval();
  }
  return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::consider_flush
//       Access: Published
//  Description: See SocketStream::consider_flush()
////////////////////////////////////////////////////////////////////
INLINE bool SocketStreamRecorder::
consider_flush() {
  if (_stream != (SocketStream *)NULL) {
    return _stream->consider_flush();
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStreamRecorder::flush
//       Access: Published
//  Description: See SocketStream::flush()
////////////////////////////////////////////////////////////////////
INLINE bool SocketStreamRecorder::
flush() {
  if (_stream != (SocketStream *)NULL) {
    return _stream->flush();
  }
  return true;
}
