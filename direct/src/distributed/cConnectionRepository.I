/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cConnectionRepository.I
 * @author drose
 * @date 2004-05-17
 */

/**
 * Returns the DCFile object associated with this repository.
 */
INLINE DCFile &CConnectionRepository::
get_dc_file() {
  return _dc_file;
}

/**
 * Returns true if this repository can have 'owner' views of distributed
 * objects.
 */
INLINE bool CConnectionRepository::
has_owner_view() const {
  return _has_owner_view;
}

/**
 * Set true to specify this repository should process distributed updates
 * internally in C++ code, or false if it should return them to Python.
 */
INLINE void CConnectionRepository::
set_handle_c_updates(bool handle_c_updates) {
  _handle_c_updates = handle_c_updates;
}

/**
 * Returns true if this repository will process distributed updates internally
 * in C++ code, or false if it will return them to Python.
 */
INLINE bool CConnectionRepository::
get_handle_c_updates() const {
  return _handle_c_updates;
}

/**
 * Sets the client_datagram flag.  If this is true, incoming datagrams are not
 * expected to be prefixed with the server routing information like message
 * sender, channel number, etc.; otherwise, these server fields are parsed and
 * removed from each incoming datagram.
 */
INLINE void CConnectionRepository::
set_client_datagram(bool client_datagram) {
  _client_datagram = client_datagram;
}

/**
 * Returns the client_datagram flag.
 */
INLINE bool CConnectionRepository::
get_client_datagram() const {
  return _client_datagram;
}

/**
 * Sets the handle_datagrams_internally flag.  When true, certain message
 * types can be handled by the C++ code in in this module.  When false, all
 * datagrams, regardless of message type, are passed up to Python for
 * processing.
 *
 * The CMU distributed-object implementation requires this to be set false.
 */
INLINE void CConnectionRepository::
set_handle_datagrams_internally(bool handle_datagrams_internally) {
  _handle_datagrams_internally = handle_datagrams_internally;
}

/**
 * Returns the handle_datagrams_internally flag.
 */
INLINE bool CConnectionRepository::
get_handle_datagrams_internally() const {
  return _handle_datagrams_internally;
}

/**
 * Returns the current setting of TCP header size.  See set_tcp_header_size().
 */
INLINE int CConnectionRepository::
get_tcp_header_size() const {
  return _tcp_header_size;
}

#ifdef HAVE_PYTHON
/**
 * Records the pointer to the Python class that derives from
 * CConnectionRepository.  This allows the C++ implementation to directly
 * manipulation some python structures on the repository.
 */
INLINE void CConnectionRepository::
set_python_repository(PyObject *python_repository) {
  _python_repository = python_repository;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_NET
/**
 * Returns the QueuedConnectionManager object associated with the repository.
 */
INLINE QueuedConnectionManager &CConnectionRepository::
get_qcm() {
  return _qcm;
}
#endif  // HAVE_NET

#ifdef HAVE_NET
/**
 * Returns the ConnectionWriter object associated with the repository.
 */
INLINE ConnectionWriter &CConnectionRepository::
get_cw() {
  return _cw;
}
  INLINE ConnectionWriter &get_cw();
#endif  // HAVE_NET

#ifdef HAVE_NET
/**
 * Returns the QueuedConnectionReader object associated with the repository.
 */
INLINE QueuedConnectionReader &CConnectionRepository::
get_qcr() {
  return _qcr;
}
  INLINE QueuedConnectionReader &get_qcr();
#endif  // HAVE_NET

#ifdef WANT_NATIVE_NET
/**
 * Returns the Buffered_DatagramConnection object associated with the
 * repository.
 */
INLINE Buffered_DatagramConnection &CConnectionRepository::
get_bdc() {
  return _bdc;
}
  INLINE Buffered_DatagramConnection &get_bdc();
#endif

/**
 * Fills the datagram object with the datagram most recently retrieved by
 * check_datagram().
 */
INLINE void CConnectionRepository::
get_datagram(Datagram &dg) {
  ReMutexHolder holder(_lock);
  dg = _dg;
}

/**
 * Fills the DatagramIterator object with the iterator for the datagram most
 * recently retrieved by check_datagram().  This iterator has already read
 * past the datagram header and the message type, and is positioned at the
 * beginning of data.
 */
INLINE void CConnectionRepository::
get_datagram_iterator(DatagramIterator &di) {
  ReMutexHolder holder(_lock);
  di = _di;
}

/**
 * Returns the channel(s) to which the current message was sent, according to
 * the datagram headers.  This information is not available to the client.
 */
INLINE CHANNEL_TYPE CConnectionRepository::
get_msg_channel(int offset) const {
  ReMutexHolder holder(_lock);
  nassertr(offset < (int)_msg_channels.size(),0);
  return _msg_channels[offset];
}

INLINE int CConnectionRepository::
get_msg_channel_count() const {
  ReMutexHolder holder(_lock);
  return _msg_channels.size();
}

/**
 * Returns the sender ID of the current message, according to the datagram
 * headers.  This information is not available to the client.
 */
INLINE CHANNEL_TYPE CConnectionRepository::
get_msg_sender() const {
  ReMutexHolder holder(_lock);
  return _msg_sender;
}

/**
 * Returns the security code associated with the current message, according to
 * the datagram headers.  This information is not available to the client.
 * INLINE unsigned char CConnectionRepository:: get_sec_code() const { return
 * _sec_code; }
 */

/**
 * Returns the type ID of the current message, according to the datagram
 * headers.
 */
INLINE unsigned int CConnectionRepository::
get_msg_type() const {
  ReMutexHolder holder(_lock);
  return _msg_type;
}

/**
 * Returns event string that will be thrown if the datagram reader queue
 * overflows.
 */
INLINE const std::string &CConnectionRepository::
get_overflow_event_name() {
  return _overflow_event_name;
}

/**
 * Returns true if repository is queueing outgoing messages into a message
 * bundle
 */
INLINE bool CConnectionRepository::
is_bundling_messages() const {
  ReMutexHolder holder(_lock);
  return _bundling_msgs > 0;
}

/**
 * Enable/disable outbound message bundling
 */
INLINE void CConnectionRepository::
set_want_message_bundling(bool flag) {
  ReMutexHolder holder(_lock);
  // don't allow enabledisable while bundling
  nassertv(_bundling_msgs == 0);
  _want_message_bundling = flag;
}

/**
 * Returns true if message bundling enabled
 */
INLINE bool CConnectionRepository::
get_want_message_bundling() const {
  ReMutexHolder holder(_lock);
  return _want_message_bundling;
}

/**
 * Enables/disables quiet zone mode
 */
INLINE void CConnectionRepository::
set_in_quiet_zone(bool flag) {
  ReMutexHolder holder(_lock);
  _in_quiet_zone = flag;
}

/**
 * Returns true if repository is in quiet zone mode
 */
INLINE bool CConnectionRepository::
get_in_quiet_zone() const {
  ReMutexHolder holder(_lock);
  return _in_quiet_zone;
}

/**
 * Sets the simulated disconnect flag.  While this is true, no datagrams will
 * be retrieved from or sent to the server.  The idea is to simulate a
 * temporary network outage.
 */
INLINE void CConnectionRepository::
set_simulated_disconnect(bool simulated_disconnect) {
  _simulated_disconnect = simulated_disconnect;
}

/**
 * Returns the simulated disconnect flag.  While this is true, no datagrams
 * will be retrieved from or sent to the server.  The idea is to simulate a
 * temporary network outage.
 */
INLINE bool CConnectionRepository::
get_simulated_disconnect() const {
  return _simulated_disconnect;
}

/**
 * Toggles the current setting of the verbose flag.  When true, this describes
 * every message going back and forth on the wire.
 */
INLINE void CConnectionRepository::
toggle_verbose() {
  _verbose = !_verbose;
}

/**
 * Directly sets the verbose flag.  When true, this describes every message
 * going back and forth on the wire.
 */
INLINE void CConnectionRepository::
set_verbose(bool verbose) {
  _verbose = verbose;
}

/**
 * Returns the current setting of the verbose flag.  When true, this describes
 * every message going back and forth on the wire.
 */
INLINE bool CConnectionRepository::
get_verbose() const {
  return _verbose;
}

/**
 * Directly sets the time_warning field.  When non zero, this describes every
 * message going back and forth on the wire when the msg handling time is over
 * it
 */
INLINE void CConnectionRepository::
set_time_warning(float time_warning) {
  _time_warning = time_warning;
}

/**
 * Returns the current setting of the time_warning field.
 */
INLINE float CConnectionRepository::
get_time_warning() const {
  return _time_warning;
}
