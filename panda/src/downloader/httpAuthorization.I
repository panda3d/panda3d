// Filename: httpAuthorization.I
// Created by:  drose (22Oct02)
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
//     Function: HTTPAuthorization::get_realm
//       Access: Public
//  Description: Returns the realm to which this authorization
//               applies.  This is the server-supplied string that may
//               have meaning to the user, and describes the general
//               collection of things protected by this password.
////////////////////////////////////////////////////////////////////
const string &HTTPAuthorization::
get_realm() const {
  return _realm;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPAuthorization::get_domain
//       Access: Public
//  Description: Returns the set of domain strings on which this
//               authorization applies.  This is the set of URL
//               prefixes for which this authorization should be
//               used.
////////////////////////////////////////////////////////////////////
const vector_string &HTTPAuthorization::
get_domain() const {
  return _domain;
}
