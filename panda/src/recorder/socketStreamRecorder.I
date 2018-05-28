/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socketStreamRecorder.I
 * @author drose
 * @date 2004-01-28
 */

/**
 *
 */
INLINE SocketStreamRecorder::
SocketStreamRecorder() :
  _stream(nullptr),
  _owns_stream(false),
  _closed(true)
{
}

/**
 *
 */
INLINE SocketStreamRecorder::
SocketStreamRecorder(SocketStream *stream, bool owns_stream) :
  _stream(stream),
  _owns_stream(owns_stream),
  _closed(false)
{
}

/**
 *
 */
INLINE SocketStreamRecorder::
~SocketStreamRecorder() {
  if (_owns_stream) {
    delete _stream;
  }
}

/**
 * See SocketStream::send_datagram().
 */
bool SocketStreamRecorder::
send_datagram(const Datagram &dg) {
  if (_stream != nullptr) {
    return _stream->send_datagram(dg);
  }
  return true;
}

/**
 * See SocketStream::is_closed().
 */
INLINE bool SocketStreamRecorder::
is_closed() {
  if (_stream != nullptr) {
    return _stream->is_closed();
  }
  return is_playing() && _closed;
}

/**
 * See SocketStream::close().
 */
INLINE void SocketStreamRecorder::
close() {
  if (_stream != nullptr) {
    _stream->close();
  }
  _closed = true;
}

/**
 * See SocketStream::set_collect_tcp().
 */
INLINE void SocketStreamRecorder::
set_collect_tcp(bool collect_tcp) {
  if (_stream != nullptr) {
    _stream->set_collect_tcp(collect_tcp);
  }
}

/**
 * See SocketStream::get_collect_tcp().
 */
INLINE bool SocketStreamRecorder::
get_collect_tcp() const {
  if (_stream != nullptr) {
    return _stream->get_collect_tcp();
  }
  return false;
}

/**
 * See SocketStream::set_collect_tcp_interval().
 */
INLINE void SocketStreamRecorder::
set_collect_tcp_interval(double interval) {
  if (_stream != nullptr) {
    _stream->set_collect_tcp_interval(interval);
  }
}

/**
 * See SocketStream::get_collect_tcp_interval().
 */
INLINE double SocketStreamRecorder::
get_collect_tcp_interval() const {
  if (_stream != nullptr) {
    return _stream->get_collect_tcp_interval();
  }
  return 0.0;
}

/**
 * See SocketStream::consider_flush()
 */
INLINE bool SocketStreamRecorder::
consider_flush() {
  if (_stream != nullptr) {
    return _stream->consider_flush();
  }
  return true;
}

/**
 * See SocketStream::flush()
 */
INLINE bool SocketStreamRecorder::
flush() {
  if (_stream != nullptr) {
    return _stream->flush();
  }
  return true;
}
