/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomTristripsAdjacency.cxx
 * @author rdb
 * @date 2018-03-01
 */

#include "geomTristripsAdjacency.h"
#include "geomTriangles.h"
#include "geomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"

TypeHandle GeomTristripsAdjacency::_type_handle;

/**
 *
 */
GeomTristripsAdjacency::
GeomTristripsAdjacency(GeomTristripsAdjacency::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}

/**
 *
 */
GeomTristripsAdjacency::
GeomTristripsAdjacency(const GeomTristripsAdjacency &copy) :
  GeomPrimitive(copy)
{
}

/**
 *
 */
GeomTristripsAdjacency::
~GeomTristripsAdjacency() {
}

/**
 *
 */
PT(GeomPrimitive) GeomTristripsAdjacency::
make_copy() const {
  return new GeomTristripsAdjacency(*this);
}

/**
 * Returns the fundamental rendering type of this primitive: whether it is
 * points, lines, or polygons.
 *
 * This is used to set up the appropriate antialiasing settings when
 * AntialiasAttrib::M_auto is in effect; it also implies the type of primitive
 * that will be produced when decompose() is called.
 */
GeomPrimitive::PrimitiveType GeomTristripsAdjacency::
get_primitive_type() const {
  return PT_polygons;
}

/**
 * Returns the set of GeomRendering bits that represent the rendering
 * properties required to properly render this primitive.
 */
int GeomTristripsAdjacency::
get_geom_rendering() const {
  if (is_indexed()) {
    if (get_num_primitives() > 1) {
      return GR_triangle_strip | GR_indexed_other | GR_strip_cut_index | GR_adjacency;
    } else {
      return GR_triangle_strip | GR_indexed_other | GR_adjacency;
    }
  } else {
    return GR_triangle_strip | GR_adjacency;
  }
}

/**
 * Returns the minimum number of vertices that must be added before
 * close_primitive() may legally be called.
 */
int GeomTristripsAdjacency::
get_min_num_vertices_per_primitive() const {
  return 6;
}

/**
 * Returns the number of vertices that are added between primitives that
 * aren't, strictly speaking, part of the primitives themselves.  This is
 * used, for instance, to define degenerate triangles to connect otherwise
 * disconnected triangle strips.
 */
int GeomTristripsAdjacency::
get_num_unused_vertices_per_primitive() const {
  return 1;
}

/**
 * Calls the appropriate method on the GSG to draw the primitive.
 */
bool GeomTristripsAdjacency::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_tristrips_adj(reader, force);
}

/**
 * Should be redefined to return true in any primitive that implements
 * append_unused_vertices().
 */
bool GeomTristripsAdjacency::
requires_unused_vertices() const {
  return true;
}

/**
 * Called when a new primitive is begun (other than the first primitive), this
 * should add some degenerate vertices between primitives, if the primitive
 * type requires that.  The second parameter is the first vertex that begins
 * the new primitive.
 */
void GeomTristripsAdjacency::
append_unused_vertices(GeomVertexArrayData *vertices, int vertex) {
  GeomVertexWriter to(vertices, 0);
  to.set_row_unsafe(vertices->get_num_rows());
  to.add_data1i(get_strip_cut_index());
}

/**
 * Tells the BamReader how to create objects of type Geom.
 */
void GeomTristripsAdjacency::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Geom is encountered in the Bam file.  It should create the Geom and
 * extract its information from the file.
 */
TypedWritable *GeomTristripsAdjacency::
make_from_bam(const FactoryParams &params) {
  GeomTristripsAdjacency *object = new GeomTristripsAdjacency(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
