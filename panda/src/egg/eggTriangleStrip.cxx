// Filename: eggTriangleStrip.cxx
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

#include "eggTriangleStrip.h"
#include "eggGroupNode.h"
#include "eggPolygon.h"
#include "plane.h"

#include "indent.h"

#include <algorithm>

TypeHandle EggTriangleStrip::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggTriangleStrip::apply_last_attribute
//       Access: Published, Virtual
//  Description: Sets the last vertex of the triangle (or each
//               component) to the primitive normal and/or color, if
//               the primitive is flat-shaded.  This reflects the
//               OpenGL convention of storing flat-shaded properties on
//               the last vertex, although it is not usually a
//               convention in Egg.
//
//               This may introduce redundant vertices to the vertex
//               pool.
////////////////////////////////////////////////////////////////////
void EggTriangleStrip::
apply_last_attribute() {
  // The first component gets applied to the third vertex, and so on
  // from there.
  for (int i = 0; i < get_num_components(); i++) {
    EggAttributes *component = get_component(i);
    do_apply_flat_attribute(i + 2, component);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggTriangleStrip::apply_first_attribute
//       Access: Published, Virtual
//  Description: Sets the first vertex of the triangle (or each
//               component) to the primitive normal and/or color, if
//               the primitive is flat-shaded.  This reflects the
//               DirectX convention of storing flat-shaded properties
//               on the first vertex, although it is not usually a
//               convention in Egg.
//
//               This may introduce redundant vertices to the vertex
//               pool.
////////////////////////////////////////////////////////////////////
void EggTriangleStrip::
apply_first_attribute() {
  // The first component gets applied to the first vertex, and so on
  // from there.
  for (int i = 0; i < get_num_components(); i++) {
    EggAttributes *component = get_component(i);
    do_apply_flat_attribute(i, component);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggTriangleStrip::write
//       Access: Published, Virtual
//  Description: Writes the triangle strip to the indicated output
//               stream in Egg format.
////////////////////////////////////////////////////////////////////
void EggTriangleStrip::
write(ostream &out, int indent_level) const {
  write_header(out, indent_level, "<TriangleStrip>");
  write_body(out, indent_level+2);
  indent(out, indent_level) << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: EggTriangleStrip::triangulate_poly
//       Access: Protected, Virtual
//  Description: Fills the container up with EggPolygons that
//               represent the component triangles of this triangle
//               strip.
//
//               It is assumed that the EggTriangleStrip is not
//               already a child of any other group when this function
//               is called.
//
//               Returns true if the triangulation is successful, or
//               false if there was some error (in which case the
//               container may contain some partial triangulation).
////////////////////////////////////////////////////////////////////
bool EggTriangleStrip::
do_triangulate(EggGroupNode *container) const {
  if (size() < 3) {
    return false;
  }
  const_iterator vi = begin();
  EggVertex *v0 = (*vi);
  ++vi;
  EggVertex *v1 = (*vi);
  ++vi;
  bool reversed = false;

  for (int i = 0; i < (int)size() - 2; i++) {
    PT(EggPolygon) poly = new EggPolygon;
    poly->copy_attributes(*this);
    const EggAttributes *attrib = get_component(i);
    if (attrib->has_color()) {
      poly->set_color(attrib->get_color());
    }
    if (attrib->has_normal()) {
      poly->set_normal(attrib->get_normal());
    }

    if (reversed) {
      poly->add_vertex(v1);
      poly->add_vertex(v0);
      reversed = false;
    } else {
      poly->add_vertex(v0);
      poly->add_vertex(v1);
      reversed = true;
    }
    poly->add_vertex(*vi);
    v0 = v1;
    v1 = *vi;
    container->add_child(poly);
    ++vi;
  }

  return true;
}
