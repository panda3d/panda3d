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
//     Function: XFileTemplate::is_standard
//       Access: Public
//  Description: Returns true if this particular template is one of
//               the "standard" templates defined by
//               standardTemplates.x in this directory (and compiled
//               into the binary), or false if it is a user-custom
//               template.
////////////////////////////////////////////////////////////////////
INLINE bool XFileTemplate::
is_standard() const {
  return _is_standard;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::set_open
//       Access: Public
//  Description: Sets whether the template is considered "open" or
//               not.  If it is open (this flag is true), the set of
//               options is ignored and the instances of this
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
//               options is ignored and the instances of this
//               template may include any types of children.  If it is
//               closed (false), only the named types may be added.
////////////////////////////////////////////////////////////////////
INLINE bool XFileTemplate::
get_open() const {
  return _open;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::add_option
//       Access: Public
//  Description: Adds a new type to the list of allowable types of
//               child nodes for an instance of this template.
////////////////////////////////////////////////////////////////////
INLINE void XFileTemplate::
add_option(XFileTemplate *option) {
  _options.push_back(option);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::get_num_options
//       Access: Public
//  Description: Returns the number of templates on the options
//               list.
////////////////////////////////////////////////////////////////////
INLINE int XFileTemplate::
get_num_options() const {
  return _options.size();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::get_option
//       Access: Public
//  Description: Returns the nth template on the options list.
////////////////////////////////////////////////////////////////////
INLINE XFileTemplate *XFileTemplate::
get_option(int n) const {
  nassertr(n >= 0 && n < (int)_options.size(), NULL);
  return _options[n];
}
