/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomTrianglesAdjacency.cxx
 * @author rdb
 * @date 2018-03-01
 */

#include "geomTrianglesAdjacency.h"
#include "geomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"

TypeHandle GeomTrianglesAdjacency::_type_handle;

/**
 *
 */
GeomTrianglesAdjacency::
GeomTrianglesAdjacency(GeomTrianglesAdjacency::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}

/**
 *
 */
GeomTrianglesAdjacency::
GeomTrianglesAdjacency(const GeomTrianglesAdjacency &copy) :
  GeomPrimitive(copy)
{
}

/**
 *
 */
GeomTrianglesAdjacency::
~GeomTrianglesAdjacency() {
}

/**
 *
 */
PT(GeomPrimitive) GeomTrianglesAdjacency::
make_copy() const {
  return new GeomTrianglesAdjacency(*this);
}

/**
 * Returns the fundamental rendering type of this primitive: whether it is
 * points, lines, or polygons.
 *
 * This is used to set up the appropriate antialiasing settings when
 * AntialiasAttrib::M_auto is in effect; it also implies the type of primitive
 * that will be produced when decompose() is called.
 */
GeomPrimitive::PrimitiveType GeomTrianglesAdjacency::
get_primitive_type() const {
  return PT_polygons;
}

/**
 * Returns the set of GeomRendering bits that represent the rendering
 * properties required to properly render this primitive.
 */
int GeomTrianglesAdjacency::
get_geom_rendering() const {
  return GeomPrimitive::get_geom_rendering() | GR_adjacency;
}

/**
 * If the primitive type is a simple type in which all primitives have the
 * same number of vertices, like triangles, returns the number of vertices per
 * primitive.  If the primitive type is a more complex type in which different
 * primitives might have different numbers of vertices, for instance a
 * triangle strip, returns 0.
 */
int GeomTrianglesAdjacency::
get_num_vertices_per_primitive() const {
  return 6;
}

/**
 * Calls the appropriate method on the GSG to draw the primitive.
 */
bool GeomTrianglesAdjacency::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_triangles_adj(reader, force);
}

/**
 * The virtual implementation of doubleside().
 */
CPT(GeomPrimitive) GeomTrianglesAdjacency::
doubleside_impl() const {
  Thread *current_thread = Thread::get_current_thread();
  PT(GeomTrianglesAdjacency) reversed = new GeomTrianglesAdjacency(*this);

  GeomPrimitivePipelineReader from(this, current_thread);

  // This is like reverse(), except we don't clear the vertices first.  That
  // way we double the vertices up.

  // First, rotate the original copy, if necessary, so the flat-firstflat-last
  // nature of the vertices is consistent throughout the primitive.
  bool needs_rotate = false;
  switch (from.get_shade_model()) {
  case SM_flat_first_vertex:
  case SM_flat_last_vertex:
    reversed = (GeomTrianglesAdjacency *)DCAST(GeomTrianglesAdjacency, reversed->rotate());
    needs_rotate = true;

  default:
    break;
  }

  // Now append all the new vertices, in reverse order.
  for (int i = from.get_num_vertices() - 1; i >= 0; --i) {
    reversed->add_vertex(from.get_vertex(i));
  }

  // Finally, re-rotate the whole thing to get back to the original shade
  // model.
  if (needs_rotate) {
    reversed = (GeomTrianglesAdjacency *)DCAST(GeomTrianglesAdjacency, reversed->rotate());
  }

  return reversed;
}

/**
 * The virtual implementation of reverse().
 */
CPT(GeomPrimitive) GeomTrianglesAdjacency::
reverse_impl() const {
  Thread *current_thread = Thread::get_current_thread();
  PT(GeomTrianglesAdjacency) reversed = new GeomTrianglesAdjacency(*this);

  GeomPrimitivePipelineReader from(this, current_thread);
  reversed->clear_vertices();

  for (int i = from.get_num_vertices() - 1; i >= 0; --i) {
    reversed->add_vertex(from.get_vertex(i));
  }

  switch (from.get_shade_model()) {
  case SM_flat_first_vertex:
    reversed->set_shade_model(SM_flat_last_vertex);
    reversed = (GeomTrianglesAdjacency *)DCAST(GeomTrianglesAdjacency, reversed->rotate());
    break;

  case SM_flat_last_vertex:
    reversed->set_shade_model(SM_flat_first_vertex);
    reversed = (GeomTrianglesAdjacency *)DCAST(GeomTrianglesAdjacency, reversed->rotate());
    break;

  default:
    break;
  }

  return reversed;
}

/**
 * Tells the BamReader how to create objects of type Geom.
 */
void GeomTrianglesAdjacency::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Geom is encountered in the Bam file.  It should create the Geom and
 * extract its information from the file.
 */
TypedWritable *GeomTrianglesAdjacency::
make_from_bam(const FactoryParams &params) {
  GeomTrianglesAdjacency *object = new GeomTrianglesAdjacency(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
