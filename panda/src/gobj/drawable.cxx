// Filename: drawable.cxx
// Created by:  drose (15Jan99)
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
