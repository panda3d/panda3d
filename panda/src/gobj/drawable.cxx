// Filename: drawable.cxx
// Created by:  drose (15Jan99)
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

#include "drawable.h"

TypeHandle dDrawable::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Drawable::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
dDrawable::
dDrawable() : WritableConfigurable() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Drawable::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
dDrawable::
~dDrawable() {
}

////////////////////////////////////////////////////////////////////
//     Function: Drawable::draw
//       Access: Public, Virtual
//  Description: Actually draws the Drawable with the indicated GSG.
//               At this level, this doesn't do very much.
////////////////////////////////////////////////////////////////////
void dDrawable::
draw(GraphicsStateGuardianBase *, const qpGeomVertexData *) const { 
  if (is_dirty()) {
    ((dDrawable *)this)->config(); 
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Drawable::draw
//       Access: Public, Virtual
//  Description: Returns true if the Drawable has any dynamic
//               properties that are expected to change from one frame
//               to the next, or false if the Drawable is largely
//               static.
////////////////////////////////////////////////////////////////////
bool dDrawable::
is_dynamic() const {
  return false;
}

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
