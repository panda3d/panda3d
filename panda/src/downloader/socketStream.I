// Filename: socketStream.I
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


////////////////////////////////////////////////////////////////////
//     Function: SSReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SSReader::
SSReader(istream *stream) : _istream(stream) {
  _data_expected = 0;

#ifdef SIMULATE_NETWORK_DELAY
  _delay_active = false;
  _min_delay = 0.0;
  _delay_variance = 0.0;
#endif  // SIMULATE_NETWORK_DELAY
}

////////////////////////////////////////////////////////////////////
//     Function: SSReader::receive_datagram
//       Access: Published
//  Description: Receives a datagram over the socket by expecting a
//               little-endian 16-bit byte count as a prefix.  If the
//               socket stream is non-blocking, may return false if
//               the data is not available; otherwise, returns false
//               only if the socket closes.
////////////////////////////////////////////////////////////////////
bool SSReader::
receive_datagram(Datagram &dg) {
#ifdef SIMULATE_NETWORK_DELAY
  if (_delay_active) {
    while (do_receive_datagram(dg)) {
      delay_datagram(dg);
    }
    return get_delayed(dg);
  }

  // Pick up any datagrams that might have been leftover in the queue
  // when we disabled the delay.
  if (get_delayed(dg)) {
    return true;
  }
#endif  // SIMULATE_NETWORK_DELAY

  return do_receive_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: SSWriter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SSWriter::
SSWriter(ostream *stream) : _ostream(stream) {
  _collect_tcp = collect_tcp;
  _collect_tcp_interval = collect_tcp_interval;
  _queued_data_start = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: SSWriter::set_collect_tcp
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
INLINE void SSWriter::
set_collect_tcp(bool collect_tcp) {
  _collect_tcp = collect_tcp;
}

////////////////////////////////////////////////////////////////////
//     Function: SSWriter::get_collect_tcp
//       Access: Published
//  Description: Returns the current setting of "collect-tcp" mode.
//               See set_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE bool SSWriter::
get_collect_tcp() const {
  return _collect_tcp;
}

////////////////////////////////////////////////////////////////////
//     Function: SSWriter::set_collect_tcp_interval
//       Access: Published
//  Description: Specifies the interval in time, in seconds, for which
//               to hold TCP packets before sending all of the
//               recently received packets at once.  This only has
//               meaning if "collect-tcp" mode is enabled; see
//               set_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE void SSWriter::
set_collect_tcp_interval(double interval) {
  _collect_tcp_interval = interval;
}

////////////////////////////////////////////////////////////////////
//     Function: SSWriter::get_collect_tcp_interval
//       Access: Published
//  Description: Returns the interval in time, in seconds, for which
//               to hold TCP packets before sending all of the
//               recently received packets at once.  This only has
//               meaning if "collect-tcp" mode is enabled; see
//               set_collect_tcp().
////////////////////////////////////////////////////////////////////
INLINE double SSWriter::
get_collect_tcp_interval() const {
  return _collect_tcp_interval;
}

////////////////////////////////////////////////////////////////////
//     Function: SSWriter::consider_flush
//       Access: Published
//  Description: Sends the most recently queued data if enough time
//               has elapsed.  This only has meaning if
//               set_collect_tcp() has been set to true.
////////////////////////////////////////////////////////////////////
INLINE bool SSWriter::
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
//     Function: SSWriter::flush
//       Access: Published
//  Description: Sends the most recently queued data now.  This only
//               has meaning if set_collect_tcp() has been set to
//               true.
////////////////////////////////////////////////////////////////////
INLINE bool SSWriter::
flush() {
  _ostream->flush();
  _queued_data_start = ClockObject::get_global_clock()->get_real_time();
  return !is_closed();
}

////////////////////////////////////////////////////////////////////
//     Function: ISocketStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ISocketStream::
ISocketStream(streambuf *buf) : istream(buf), SSReader(this) {
}

////////////////////////////////////////////////////////////////////
//     Function: OSocketStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OSocketStream::
OSocketStream(streambuf *buf) : ostream(buf), SSWriter(this) {
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
  return SSWriter::flush();
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SocketStream::
SocketStream(streambuf *buf) : iostream(buf), SSReader(this), SSWriter(this) {
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
  return SSWriter::flush();
}
