// Filename: eggCompositePrimitive.cxx
// Created by:  drose (13Mar05)
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

#include "eggCompositePrimitive.h"
#include "eggGroupNode.h"

TypeHandle EggCompositePrimitive::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::triangulate_in_place
//       Access: Published
//  Description: Subdivides the composite primitive into triangles and
//               adds those triangles to the parent group node in
//               place of the original primitive.  Returns a pointer
//               to the original primitive, which is likely about to
//               be destructed.
//
//               If convex_also is true, both concave and convex
//               polygons will be subdivided into triangles;
//               otherwise, only concave polygons will be subdivided,
//               and convex polygons will be copied unchanged into the
//               container.
////////////////////////////////////////////////////////////////////
PT(EggCompositePrimitive) EggCompositePrimitive::
triangulate_in_place() {
  EggGroupNode *parent = get_parent();
  nassertr(parent != (EggGroupNode *)NULL, this);

  PT(EggCompositePrimitive) save_me = this;
  parent->remove_child(this);
  do_triangulate(parent);

  return save_me;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::prepare_add_vertex
//       Access: Protected, Virtual
//  Description: Marks the vertex as belonging to the primitive.  This
//               is an internal function called by the STL-like
//               functions push_back() and insert(), in preparation
//               for actually adding the vertex.
//
//               i indicates the new position of the vertex in the
//               list; n indicates the new number of vertices after
//               the operation has completed.
////////////////////////////////////////////////////////////////////
void EggCompositePrimitive::
prepare_add_vertex(EggVertex *vertex, int i, int n) {
  EggPrimitive::prepare_add_vertex(vertex, i, n);

  if (n >= 3) {
    i = max(i - 2, 0);
    nassertv(i <= (int)_components.size());
    _components.insert(_components.begin() + i, new EggAttributes(*this));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::prepare_remove_vertex
//       Access: Protected, Virtual
//  Description: Marks the vertex as removed from the primitive.  This
//               is an internal function called by the STL-like
//               functions pop_back() and erase(), in preparation for
//               actually doing the removal.
//
//               i indicates the former position of the vertex in the
//               list; n indicates the current number of vertices
//               before the operation has completed.
//
//               It is an error to attempt to remove a vertex that is
//               not already a vertex of this primitive.
////////////////////////////////////////////////////////////////////
void EggCompositePrimitive::
prepare_remove_vertex(EggVertex *vertex, int i, int n) {
  EggPrimitive::prepare_remove_vertex(vertex, i, n);

  if (n >= 3) {
    i = max(i - 2, 0);
    nassertv(i < (int)_components.size());
    delete _components[i];
    _components.erase(_components.begin() + i);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::write_body
//       Access: Protected
//  Description: Writes the attributes and the vertices referenced by
//               the primitive to the indicated output stream in Egg
//               format.
////////////////////////////////////////////////////////////////////
void EggCompositePrimitive::
write_body(ostream &out, int indent_level) const {
  EggPrimitive::write_body(out, indent_level);

  for (int i = 0; i < get_num_components(); i++) {
    const EggAttributes *attrib = get_component(i);
    if (attrib->compare_to(*this) != 0) {
      indent(out, indent_level)
        << "<Component> " << i << " {\n";
      attrib->write(out, indent_level + 2);
      indent(out, indent_level) << "}\n";
    }
  }
}
