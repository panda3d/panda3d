// Filename: clipPlaneAttrib.I
// Created by:  drose (11Jul02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::Constructor
//       Access: Protected
//  Description: Use ClipPlaneAttrib::make() to construct a new
//               ClipPlaneAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ClipPlaneAttrib::
ClipPlaneAttrib() {
  _off_all_planes = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::Copy Constructor
//       Access: Protected
//  Description: Use ClipPlaneAttrib::make() to construct a new
//               ClipPlaneAttrib object.  The copy constructor is only
//               defined to facilitate methods like add_on_plane().
////////////////////////////////////////////////////////////////////
INLINE ClipPlaneAttrib::
ClipPlaneAttrib(const ClipPlaneAttrib &copy) :
  _on_planes(copy._on_planes),
  _off_planes(copy._off_planes),
  _off_all_planes(copy._off_all_planes)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_num_on_planes
//       Access: Published
//  Description: Returns the number of planes that are enabled by
//               the attribute.
////////////////////////////////////////////////////////////////////
INLINE int ClipPlaneAttrib::
get_num_on_planes() const {
  return _on_planes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_on_plane
//       Access: Published
//  Description: Returns the nth plane enabled by the attribute,
//               sorted in render order.
////////////////////////////////////////////////////////////////////
INLINE NodePath ClipPlaneAttrib::
get_on_plane(int n) const {
  nassertr(n >= 0 && n < (int)_on_planes.size(), NodePath::fail());
  return _on_planes[n];
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::has_on_plane
//       Access: Published
//  Description: Returns true if the indicated plane is enabled by
//               the attrib, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneAttrib::
has_on_plane(const NodePath &plane) const {
  return _on_planes.find(plane) != _on_planes.end();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_num_off_planes
//       Access: Published
//  Description: Returns the number of planes that are disabled by
//               the attribute.
////////////////////////////////////////////////////////////////////
INLINE int ClipPlaneAttrib::
get_num_off_planes() const {
  return _off_planes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_off_plane
//       Access: Published
//  Description: Returns the nth plane disabled by the attribute,
//               sorted in arbitrary (pointer) order.
////////////////////////////////////////////////////////////////////
INLINE NodePath ClipPlaneAttrib::
get_off_plane(int n) const {
  nassertr(n >= 0 && n < (int)_off_planes.size(), NodePath::fail());
  return _off_planes[n];
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::has_off_plane
//       Access: Published
//  Description: Returns true if the indicated plane is disabled by
//               the attrib, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneAttrib::
has_off_plane(const NodePath &plane) const {
  return _off_planes.find(plane) != _off_planes.end() ||
    (_off_all_planes && !has_on_plane(plane));
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::has_all_off
//       Access: Published
//  Description: Returns true if this attrib disables all planes
//               (although it may also enable some).
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneAttrib::
has_all_off() const {
  return _off_all_planes;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::is_identity
//       Access: Published
//  Description: Returns true if this is an identity attrib: it does
//               not change the set of planes in use.
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneAttrib::
is_identity() const {
  return _on_planes.empty() && _off_planes.empty() && !_off_all_planes;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::check_filtered
//       Access: Private
//  Description: Confirms whether the _filtered table is still valid.
//               It may become invalid if someone calls
//               PlaneNode::set_priority().
//
//               If the table is invalid, transparently empties it
//               before returning.
////////////////////////////////////////////////////////////////////
INLINE void ClipPlaneAttrib::
check_filtered() const {
  if (_sort_seq != PlaneNode::get_sort_seq()) {
    ((ClipPlaneAttrib *)this)->sort_on_planes();
  }
}
