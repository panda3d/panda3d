// Filename: drawable.cxx
// Created by:  drose (15Jan99)
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

#include "drawable.h"

TypeHandle dDrawable::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Drawable::propagate_stale_bound
//       Access: Protected, Virtual
//  Description: Called by BoundedObject::mark_bound_stale(), this
//               should make sure that all bounding volumes that
//               depend on this one are marked stale also.
////////////////////////////////////////////////////////////////////
void dDrawable::
propagate_stale_bound() {
  // Unforunately, we don't have a pointer to the GeomNode that
  // includes us, so we can't propagate the bounding volume change
  // upwards.  Need to address this.
}

////////////////////////////////////////////////////////////////////
//     Function: dDrawable::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void dDrawable::
write_datagram(BamWriter *manager, Datagram &me)
{
  //dDrawable contains nothing, but it has to define write_datagram
}
