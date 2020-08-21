/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socketStream.I
 * @author drose
 * @date 2002-10-15
 */

/**
 * Receives a datagram over the socket by expecting a little-endian 16-bit
 * byte count as a prefix.  If the socket stream is non-blocking, may return
 * false if the data is not available; otherwise, returns false only if the
 * socket closes.
 */
INLINE bool SSReader::
receive_datagram(Datagram &dg) {
#ifdef SIMULATE_NETWORK_DELAY
  if (_delay_active) {
    while (do_receive_datagram(dg)) {
      delay_datagram(dg);
    }
    return get_delayed(dg);
  }

  // Pick up any datagrams that might have been leftover in the queue when we
  // disabled the delay.
  if (get_delayed(dg)) {
    return true;
  }
#endif  // SIMULATE_NETWORK_DELAY

  return do_receive_datagram(dg);
}

/**
 * Sets the header size for datagrams.  At the present, legal values for this
 * are 0, 2, or 4; this specifies the number of bytes to use encode the
 * datagram length at the start of each TCP datagram.  Sender and receiver
 * must independently agree on this.
 */
INLINE void SSReader::
set_tcp_header_size(int tcp_header_size) {
  nassertv(tcp_header_size == 0 || tcp_header_size == 2 || tcp_header_size == 4);
  _tcp_header_size = tcp_header_size;
}

/**
 * Returns the header size for datagrams.  See set_tcp_header_size().
 */
INLINE int SSReader::
get_tcp_header_size() const {
  return _tcp_header_size;
}

/**
 * Enables or disables "collect-tcp" mode.  In this mode, individual TCP
 * packets are not sent immediately, but rather they are collected together
 * and accumulated to be sent periodically as one larger TCP packet.  This
 * cuts down on overhead from the TCP/IP protocol, especially if many small
 * packets need to be sent on the same connection, but it introduces
 * additional latency (since packets must be held before they can be sent).
 *
 * See set_collect_tcp_interval() to specify the interval of time for which to
 * hold packets before sending them.
 *
 * If you enable this mode, you may also need to periodically call
 * consider_flush() to flush the queue if no packets have been sent recently.
 */
INLINE void SSWriter::
set_collect_tcp(bool collect_tcp) {
  _collect_tcp = collect_tcp;
}

/**
 * Returns the current setting of "collect-tcp" mode.  See set_collect_tcp().
 */
INLINE bool SSWriter::
get_collect_tcp() const {
  return _collect_tcp;
}

/**
 * Specifies the interval in time, in seconds, for which to hold TCP packets
 * before sending all of the recently received packets at once.  This only has
 * meaning if "collect-tcp" mode is enabled; see set_collect_tcp().
 */
INLINE void SSWriter::
set_collect_tcp_interval(double interval) {
  _collect_tcp_interval = interval;
}

/**
 * Returns the interval in time, in seconds, for which to hold TCP packets
 * before sending all of the recently received packets at once.  This only has
 * meaning if "collect-tcp" mode is enabled; see set_collect_tcp().
 */
INLINE double SSWriter::
get_collect_tcp_interval() const {
  return _collect_tcp_interval;
}

/**
 * Sets the header size for datagrams.  At the present, legal values for this
 * are 0, 2, or 4; this specifies the number of bytes to use encode the
 * datagram length at the start of each TCP datagram.  Sender and receiver
 * must independently agree on this.
 */
INLINE void SSWriter::
set_tcp_header_size(int tcp_header_size) {
  nassertv(tcp_header_size == 0 || tcp_header_size == 2 || tcp_header_size == 4);
  _tcp_header_size = tcp_header_size;
}

/**
 * Returns the header size for datagrams.  See set_tcp_header_size().
 */
INLINE int SSWriter::
get_tcp_header_size() const {
  return _tcp_header_size;
}

/**
 * Sends the most recently queued data if enough time has elapsed.  This only
 * has meaning if set_collect_tcp() has been set to true.
 */
INLINE bool SSWriter::
consider_flush() {
  if (!_collect_tcp) {
    return flush();

  } else {
    double elapsed =
      TrueClock::get_global_ptr()->get_short_time() - _queued_data_start;
    // If the elapsed time is negative, someone must have reset the clock
    // back, so just go ahead and flush.
    if (elapsed < 0.0 || elapsed >= _collect_tcp_interval) {
      return flush();
    }
  }

  return true;
}

/**
 * Sends the most recently queued data now.  This only has meaning if
 * set_collect_tcp() has been set to true.
 */
INLINE bool SSWriter::
flush() {
  _ostream->flush();
  _queued_data_start = TrueClock::get_global_ptr()->get_short_time();
  return !is_closed();
}

/**
 *
 */
INLINE ISocketStream::
ISocketStream(std::streambuf *buf) : std::istream(buf), SSReader(this) {
  _channel = nullptr;
}

/**
 *
 */
INLINE OSocketStream::
OSocketStream(std::streambuf *buf) : std::ostream(buf), SSWriter(this) {
}

/**
 * Sends the most recently queued data now.  This only has meaning if
 * set_collect_tcp() has been set to true.
 */
INLINE bool OSocketStream::
flush() {
  return SSWriter::flush();
}

/**
 *
 */
INLINE SocketStream::
SocketStream(std::streambuf *buf) : std::iostream(buf), SSReader(this), SSWriter(this) {
}

/**
 * Sets the header size for datagrams.  At the present, legal values for this
 * are 0, 2, or 4; this specifies the number of bytes to use encode the
 * datagram length at the start of each TCP datagram.  Sender and receiver
 * must independently agree on this.
 */
INLINE void SocketStream::
set_tcp_header_size(int tcp_header_size) {
  SSReader::set_tcp_header_size(tcp_header_size);
  SSWriter::set_tcp_header_size(tcp_header_size);
}

/**
 * Returns the header size for datagrams.  See set_tcp_header_size().
 */
INLINE int SocketStream::
get_tcp_header_size() const {
  return SSReader::get_tcp_header_size();
}

/**
 * Sends the most recently queued data now.  This only has meaning if
 * set_collect_tcp() has been set to true.
 */
INLINE bool SocketStream::
flush() {
  return SSWriter::flush();
}
