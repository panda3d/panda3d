// Filename: xFileTemplate.I
// Created by:  drose (03Oct04)
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
//     Function: XFileTemplate::get_guid
//       Access: Public
//  Description: Returns the GUID associated with this template.
////////////////////////////////////////////////////////////////////
INLINE const WindowsGuid &XFileTemplate::
get_guid() const {
  return _guid;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::set_open
//       Access: Public
//  Description: Sets whether the template is considered "open" or
//               not.  If it is open (this flag is true), the set of
//               restrictions is ignored and the instances of this
//               template may include any types of children.  If it is
//               closed (false), only the named types may be added.
////////////////////////////////////////////////////////////////////
INLINE void XFileTemplate::
set_open(bool open) {
  _open = open;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::get_open
//       Access: Public
//  Description: Returns whether the template is considered "open" or
//               not.  If it is open (this flag is true), the set of
//               restrictions is ignored and the instances of this
//               template may include any types of children.  If it is
//               closed (false), only the named types may be added.
////////////////////////////////////////////////////////////////////
INLINE bool XFileTemplate::
get_open() const {
  return _open;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::add_restriction
//       Access: Public
//  Description: Adds a new type to the list of allowable types of
//               child nodes for an instance of this template.  The
//               method is a little misnamed; we are actually removing
//               a restriction by adding this type to the list.
////////////////////////////////////////////////////////////////////
INLINE void XFileTemplate::
add_restriction(XFileTemplate *restriction) {
  _restrictions.push_back(restriction);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::get_num_restrictions
//       Access: Public
//  Description: Returns the number of templates on the restrictions
//               list.
////////////////////////////////////////////////////////////////////
INLINE int XFileTemplate::
get_num_restrictions() const {
  return _restrictions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::get_restriction
//       Access: Public
//  Description: Returns the nth template on the restrictions list.
////////////////////////////////////////////////////////////////////
INLINE XFileTemplate *XFileTemplate::
get_restriction(int n) const {
  nassertr(n >= 0 && n < (int)_restrictions.size(), NULL);
  return _restrictions[n];
}
