// Filename: eggCompositePrimitive.I
// Created by:  drose (13Mar05)
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
//     Function: EggCompositePrimitive::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggCompositePrimitive::
EggCompositePrimitive(const string &name) : EggPrimitive(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::Copy constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggCompositePrimitive::
EggCompositePrimitive(const EggCompositePrimitive &copy) : EggPrimitive(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::Copy assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggCompositePrimitive &EggCompositePrimitive::
operator = (const EggCompositePrimitive &copy) {
  EggPrimitive::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::get_num_components
//       Access: Published
//  Description: Returns the number of individual component triangles
//               within the composite.  Each one of these might have a
//               different set of attributes.
////////////////////////////////////////////////////////////////////
INLINE int EggCompositePrimitive::
get_num_components() const {
  return _components.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::get_component
//       Access: Published
//  Description: Returns the attributes for the nth component
//               triangle.
////////////////////////////////////////////////////////////////////
INLINE const EggAttributes *EggCompositePrimitive::
get_component(int i) const {
  nassertr(i >= 0 && i < (int)_components.size(), NULL);
  return _components[i];
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::get_component
//       Access: Published
//  Description: Returns the attributes for the nth component
//               triangle.
////////////////////////////////////////////////////////////////////
INLINE EggAttributes *EggCompositePrimitive::
get_component(int i) {
  nassertr(i >= 0 && i < (int)_components.size(), NULL);
  return _components[i];
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::set_component
//       Access: Published
//  Description: Changes the attributes for the nth component
//               triangle.
////////////////////////////////////////////////////////////////////
INLINE void EggCompositePrimitive::
set_component(int i, const EggAttributes *attrib) {
  nassertv(i >= 0 && i < (int)_components.size());
  _components[i] = new EggAttributes(*attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::triangulate_into
//       Access: Published
//  Description: Subdivides the composite primitive into triangles and
//               adds those triangles to the indicated container.
//               Does not remove the primitive from its existing
//               parent or modify it in any way.
//
//               Returns true if the triangulation is successful, or
//               false if there was some error (in which case the
//               container may contain some partial triangulation).
////////////////////////////////////////////////////////////////////
INLINE bool EggCompositePrimitive::
triangulate_into(EggGroupNode *container) const {
  return do_triangulate(container);
}
