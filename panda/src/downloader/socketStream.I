// Filename: socketStream.I
// Created by:  drose (15Oct02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ISocketStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ISocketStream::
ISocketStream(streambuf *buf) : istream(buf) {
  _data_expected = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: OSocketStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OSocketStream::
OSocketStream(streambuf *buf) : ostream(buf) {
  _collect_tcp = collect_tcp;
  _collect_tcp_interval = collect_tcp_interval;
  _queued_data_start = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: OSocketStream::set_collect_tcp
//       Access: Published
//  Description: Enables or disables "collect-tcp" mode.  In this
//               mode, individual TCP packets are not sent
//               immediately, but rather they are collected together
//               and accumulated to be sent periodically as one larger
//               TCP packet.  This cuts down on overhead from the
//               TCP/IP protocol, especially if many small packets
//               need to be sent on the same connection, but it
//               introduces additional latency (since packets must be
//               held before they can be sent).
//
//               See set_collect_tcp_interval() to specify the
//               interval of time for which to hold packets before
//               sending them.
//
//               If you enable this mode, you may also need to
//               periodically call consider_flush() to flush the queue
//               if no packets have been sent recently.
////////////////////////////////////////////////////////////////////
INLINE void OSocketStream::
set_collect_tcp(bool collect_tcp) {
  _collect_tcp = collect_tcp;
}

////////////////////////////////////////////////////////////////////
//     Function: OSocketStream::get_collect_tcp
//       Access: Published
//  Description: Returns the current setting of "collect-tcp" mode.
//               See set_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE bool OSocketStream::
get_collect_tcp() const {
  return _collect_tcp;
}

////////////////////////////////////////////////////////////////////
//     Function: OSocketStream::set_collect_tcp_interval
//       Access: Published
//  Description: Specifies the interval in time, in seconds, for which
//               to hold TCP packets before sending all of the
//               recently received packets at once.  This only has
//               meaning if "collect-tcp" mode is enabled; see
//               set_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE void OSocketStream::
set_collect_tcp_interval(double interval) {
  _collect_tcp_interval = interval;
}

////////////////////////////////////////////////////////////////////
//     Function: OSocketStream::get_collect_tcp_interval
//       Access: Published
//  Description: Returns the interval in time, in seconds, for which
//               to hold TCP packets before sending all of the
//               recently received packets at once.  This only has
//               meaning if "collect-tcp" mode is enabled; see
//               set_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE double OSocketStream::
get_collect_tcp_interval() const {
  return _collect_tcp_interval;
}

////////////////////////////////////////////////////////////////////
//     Function: OSocketStream::consider_flush
//       Access: Published
//  Description: Sends the most recently queued data if enough time
//               has elapsed.  This only has meaning if
//               set_collect_tcp() has been set to true.
////////////////////////////////////////////////////////////////////
INLINE bool OSocketStream::
consider_flush() {
  if (!_collect_tcp) {
    return flush();

  } else {
    double elapsed = 
      ClockObject::get_global_clock()->get_real_time() - _queued_data_start;
    // If the elapsed time is negative, someone must have reset the
    // clock back, so just go ahead and flush.
    if (elapsed < 0.0 || elapsed >= _collect_tcp_interval) {
      return flush();
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OSocketStream::flush
//       Access: Published
//  Description: Sends the most recently queued data now.  This only
//               has meaning if set_collect_tcp() has been set to
//               true.
////////////////////////////////////////////////////////////////////
INLINE bool OSocketStream::
flush() {
  ostream::flush();
  _queued_data_start = ClockObject::get_global_clock()->get_real_time();
  return !is_closed();
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SocketStream::
SocketStream(streambuf *buf) : iostream(buf) {
  _data_expected = 0;
  _collect_tcp = collect_tcp;
  _collect_tcp_interval = collect_tcp_interval;
  _queued_data_start = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStream::set_collect_tcp
//       Access: Published
//  Description: Enables or disables "collect-tcp" mode.  In this
//               mode, individual TCP packets are not sent
//               immediately, but rather they are collected together
//               and accumulated to be sent periodically as one larger
//               TCP packet.  This cuts down on overhead from the
//               TCP/IP protocol, especially if many small packets
//               need to be sent on the same connection, but it
//               introduces additional latency (since packets must be
//               held before they can be sent).
//
//               See set_collect_tcp_interval() to specify the
//               interval of time for which to hold packets before
//               sending them.
//
//               If you enable this mode, you may also need to
//               periodically call consider_flush() to flush the queue
//               if no packets have been sent recently.
////////////////////////////////////////////////////////////////////
INLINE void SocketStream::
set_collect_tcp(bool collect_tcp) {
  _collect_tcp = collect_tcp;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStream::get_collect_tcp
//       Access: Published
//  Description: Returns the current setting of "collect-tcp" mode.
//               See set_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE bool SocketStream::
get_collect_tcp() const {
  return _collect_tcp;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStream::set_collect_tcp_interval
//       Access: Published
//  Description: Specifies the interval in time, in seconds, for which
//               to hold TCP packets before sending all of the
//               recently received packets at once.  This only has
//               meaning if "collect-tcp" mode is enabled; see
//               set_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE void SocketStream::
set_collect_tcp_interval(double interval) {
  _collect_tcp_interval = interval;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStream::get_collect_tcp_interval
//       Access: Published
//  Description: Returns the interval in time, in seconds, for which
//               to hold TCP packets before sending all of the
//               recently received packets at once.  This only has
//               meaning if "collect-tcp" mode is enabled; see
//               set_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE double SocketStream::
get_collect_tcp_interval() const {
  return _collect_tcp_interval;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStream::consider_flush
//       Access: Published
//  Description: Sends the most recently queued data if enough time
//               has elapsed.  This only has meaning if
//               set_collect_tcp() has been set to true.
////////////////////////////////////////////////////////////////////
INLINE bool SocketStream::
consider_flush() {
  if (!_collect_tcp) {
    return flush();

  } else {
    double elapsed = 
      ClockObject::get_global_clock()->get_real_time() - _queued_data_start;
    // If the elapsed time is negative, someone must have reset the
    // clock back, so just go ahead and flush.
    if (elapsed < 0.0 || elapsed >= _collect_tcp_interval) {
      return flush();
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStream::flush
//       Access: Published
//  Description: Sends the most recently queued data now.  This only
//               has meaning if set_collect_tcp() has been set to
//               true.
////////////////////////////////////////////////////////////////////
INLINE bool SocketStream::
flush() {
  iostream::flush();
  _queued_data_start = ClockObject::get_global_clock()->get_real_time();
  return !is_closed();
}
