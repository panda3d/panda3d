// Filename: cConnectionRepository.I
// Created by:  drose (17May04)
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
//     Function: CConnectionRepository::get_dc_file
//       Access: Published
//  Description: Returns the DCFile object associated with this
//               repository.
////////////////////////////////////////////////////////////////////
INLINE DCFile &CConnectionRepository::
get_dc_file() {
  return _dc_file;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::set_client_datagram
//       Access: Published
//  Description: Sets the client_datagram flag.  If this is true,
//               incoming datagrams are not expected to be prefixed
//               with the server routing information like message
//               sender, channel number, etc.; otherwise, these server
//               fields are parsed and removed from each incoming
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE void CConnectionRepository::
set_client_datagram(bool client_datagram) {
  _client_datagram = client_datagram;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_client_datagram
//       Access: Published
//  Description: Returns the client_datagram flag.
////////////////////////////////////////////////////////////////////
INLINE bool CConnectionRepository::
get_client_datagram() const {
  return _client_datagram;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::set_python_repository
//       Access: Published
//  Description: Records the pointer to the Python class that derives
//               from CConnectionRepository.  This allows the C++
//               implementation to directly manipulation some python
//               structures on the repository.
////////////////////////////////////////////////////////////////////
INLINE void CConnectionRepository::
set_python_repository(PyObject *python_repository) {
  _python_repository = python_repository;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_NSPR
////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_qcm
//       Access: Published
//  Description: Returns the QueuedConnectionManager object associated
//               with the repository.
////////////////////////////////////////////////////////////////////
INLINE QueuedConnectionManager &CConnectionRepository::
get_qcm() {
  return _qcm;
}
#endif  // HAVE_NSPR

#ifdef HAVE_NSPR
////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_cw
//       Access: Published
//  Description: Returns the ConnectionWriter object associated
//               with the repository.
////////////////////////////////////////////////////////////////////
INLINE ConnectionWriter &CConnectionRepository::
get_cw() {
  return _cw;
}
  INLINE ConnectionWriter &get_cw();
#endif  // HAVE_NSPR

#ifdef HAVE_NSPR
////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_qcr
//       Access: Published
//  Description: Returns the QueuedConnectionReader object associated
//               with the repository.
////////////////////////////////////////////////////////////////////
INLINE QueuedConnectionReader &CConnectionRepository::
get_qcr() {
  return _qcr;
}
  INLINE QueuedConnectionReader &get_qcr();
#endif  // HAVE_NSPR

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_datagram
//       Access: Published
//  Description: Fills the datagram object with the datagram most
//               recently retrieved by check_datagram().
////////////////////////////////////////////////////////////////////
INLINE void CConnectionRepository::
get_datagram(Datagram &dg) {
  dg = _dg;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_datagram_iterator
//       Access: Published
//  Description: Fills the DatagramIterator object with the iterator
//               for the datagram most recently retrieved by
//               check_datagram().  This iterator has already read
//               past the datagram header and the message type, and is
//               positioned at the beginning of data.
////////////////////////////////////////////////////////////////////
INLINE void CConnectionRepository::
get_datagram_iterator(DatagramIterator &di) {
  di = _di;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_msg_channel
//       Access: Published
//  Description: Returns the channel from which the current message
//               was sent, according to the datagram headers.  This
//               information is not available to the client.
////////////////////////////////////////////////////////////////////
INLINE unsigned int CConnectionRepository::
get_msg_channel() const {
  return _msg_channel;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_msg_sender
//       Access: Published
//  Description: Returns the sender ID of the current message,
//               according to the datagram headers.  This information
//               is not available to the client.
////////////////////////////////////////////////////////////////////
INLINE unsigned int CConnectionRepository::
get_msg_sender() const {
  return _msg_sender;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_sec_code
//       Access: Published
//  Description: Returns the security code associated with the current
//               message, according to the datagram headers.  This
//               information is not available to the client.
////////////////////////////////////////////////////////////////////
INLINE unsigned char CConnectionRepository::
get_sec_code() const {
  return _sec_code;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_msg_type
//       Access: Published
//  Description: Returns the type ID of the current message,
//               according to the datagram headers.
////////////////////////////////////////////////////////////////////
INLINE unsigned int CConnectionRepository::
get_msg_type() const {
  return _msg_type;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_overflow_event_name
//       Access: Published
//  Description: Returns event string that will be thrown if the
//               datagram reader queue overflows.
////////////////////////////////////////////////////////////////////
INLINE const string &CConnectionRepository::
get_overflow_event_name() {
  return _overflow_event_name;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::set_simulated_disconnect
//       Access: Published
//  Description: Sets the simulated disconnect flag.  While this is
//               true, no datagrams will be retrieved from or sent to
//               the server.  The idea is to simulate a temporary
//               network outage.
////////////////////////////////////////////////////////////////////
INLINE void CConnectionRepository::
set_simulated_disconnect(bool simulated_disconnect) {
  _simulated_disconnect = simulated_disconnect;
}

////////////////////////////////////////////////////////////////////
//     Function: CConnectionRepository::get_simulated_disconnect
//       Access: Published
//  Description: Returns the simulated disconnect flag.  While this is
//               true, no datagrams will be retrieved from or sent to
//               the server.  The idea is to simulate a temporary
//               network outage.
////////////////////////////////////////////////////////////////////
INLINE bool CConnectionRepository::
get_simulated_disconnect() const {
  return _simulated_disconnect;
}
