/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTriangleFan.cxx
 * @author drose
 * @date 2005-03-23
 */

#include "eggTriangleFan.h"
#include "eggGroupNode.h"
#include "eggPolygon.h"
#include "indent.h"

TypeHandle EggTriangleFan::_type_handle;

/**
 *
 */
EggTriangleFan::
~EggTriangleFan() {
  clear();
}

/**
 * Makes a copy of this object.
 */
EggTriangleFan *EggTriangleFan::
make_copy() const {
  return new EggTriangleFan(*this);
}

/**
 * Writes the triangle fan to the indicated output stream in Egg format.
 */
void EggTriangleFan::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<TriangleFan>");
  write_body(out, indent_level+2);
  indent(out, indent_level) << "}\n";
}

/**
 * Sets the first vertex of the triangle (or each component) to the primitive
 * normal and/or color, if the primitive is flat-shaded.  This reflects the
 * DirectX convention of storing flat-shaded properties on the first vertex,
 * although it is not usually a convention in Egg.
 *
 * This may introduce redundant vertices to the vertex pool.
 */
void EggTriangleFan::
apply_first_attribute() {
  // In the case of a triangle fan, the first vertex of the fan is the common
  // vertex, so we consider the second vertex to be the key vertex of the
  // first triangle, and move from there.
  for (size_t i = 0; i < get_num_components(); ++i) {
    EggAttributes *component = get_component(i);
    do_apply_flat_attribute(i + 1, component);
  }
}

/**
 * Returns the number of initial vertices that are not used in defining any
 * component; the first component is defined by the (n + 1)th vertex, and then
 * a new component at each vertex thereafter.
 */
int EggTriangleFan::
get_num_lead_vertices() const {
  return 2;
}

/**
 * Fills the container up with EggPolygons that represent the component
 * triangles of this triangle fan.
 *
 * It is assumed that the EggTriangleFan is not already a child of any other
 * group when this function is called.
 *
 * Returns true if the triangulation is successful, or false if there was some
 * error (in which case the container may contain some partial triangulation).
 */
bool EggTriangleFan::
do_triangulate(EggGroupNode *container) const {
  if (size() < 3) {
    return false;
  }
  const_iterator vi = begin();
  EggVertex *v0 = (*vi);
  ++vi;
  EggVertex *v1 = (*vi);
  ++vi;

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

    poly->add_vertex(v0);
    poly->add_vertex(v1);
    poly->add_vertex(*vi);
    v1 = *vi;
    container->add_child(poly);
    ++vi;
  }

  return true;
}
