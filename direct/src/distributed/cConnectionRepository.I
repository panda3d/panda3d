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
#endif  // HAVE_NSPR

  INLINE ConnectionWriter &get_cw();
  INLINE QueuedConnectionReader &get_qcr();

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
