/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTriangleStrip.cxx
 * @author drose
 * @date 2005-03-13
 */

#include "eggTriangleStrip.h"
#include "eggGroupNode.h"
#include "eggPolygon.h"
#include "indent.h"

TypeHandle EggTriangleStrip::_type_handle;

/**
 *
 */
EggTriangleStrip::
~EggTriangleStrip() {
  clear();
}

/**
 * Makes a copy of this object.
 */
EggTriangleStrip *EggTriangleStrip::
make_copy() const {
  return new EggTriangleStrip(*this);
}

/**
 * Writes the triangle strip to the indicated output stream in Egg format.
 */
void EggTriangleStrip::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<TriangleStrip>");
  write_body(out, indent_level+2);
  indent(out, indent_level) << "}\n";
}

/**
 * Returns the number of initial vertices that are not used in defining any
 * component; the first component is defined by the (n + 1)th vertex, and then
 * a new component at each vertex thereafter.
 */
int EggTriangleStrip::
get_num_lead_vertices() const {
  return 2;
}

/**
 * Fills the container up with EggPolygons that represent the component
 * triangles of this triangle strip.
 *
 * It is assumed that the EggTriangleStrip is not already a child of any other
 * group when this function is called.
 *
 * Returns true if the triangulation is successful, or false if there was some
 * error (in which case the container may contain some partial triangulation).
 */
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
