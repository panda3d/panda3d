/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomLinesAdjacency.cxx
 * @author rdb
 * @date 2018-03-01
 */

#include "geomLinesAdjacency.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"

TypeHandle GeomLinesAdjacency::_type_handle;

/**
 *
 */
GeomLinesAdjacency::
GeomLinesAdjacency(GeomEnums::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}

/**
 *
 */
GeomLinesAdjacency::
GeomLinesAdjacency(const GeomLinesAdjacency &copy) :
  GeomPrimitive(copy)
{
}

/**
 *
 */
GeomLinesAdjacency::
~GeomLinesAdjacency() {
}

/**
 *
 */
PT(GeomPrimitive) GeomLinesAdjacency::
make_copy() const {
  return new GeomLinesAdjacency(*this);
}

/**
 * Returns the fundamental rendering type of this primitive: whether it is
 * points, lines, or polygons.
 *
 * This is used to set up the appropriate antialiasing settings when
 * AntialiasAttrib::M_auto is in effect; it also implies the type of primitive
 * that will be produced when decompose() is called.
 */
GeomPrimitive::PrimitiveType GeomLinesAdjacency::
get_primitive_type() const {
  return PT_lines;
}

/**
 * Returns the set of GeomRendering bits that represent the rendering
 * properties required to properly render this primitive.
 */
int GeomLinesAdjacency::
get_geom_rendering() const {
  return GeomPrimitive::get_geom_rendering() | GR_adjacency;
}

/**
 * If the primitive type is a simple type in which all primitives have the
 * same number of vertices, like lines, returns the number of vertices per
 * primitive.  If the primitive type is a more complex type in which different
 * primitives might have different numbers of vertices, for instance a line
 * strip, returns 0.
 */
int GeomLinesAdjacency::
get_num_vertices_per_primitive() const {
  return 4;
}

/**
 * Returns the minimum number of vertices that must be added before
 * close_primitive() may legally be called.
 */
int GeomLinesAdjacency::
get_min_num_vertices_per_primitive() const {
  return 4;
}

/**
 * Calls the appropriate method on the GSG to draw the primitive.
 */
bool GeomLinesAdjacency::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_lines_adj(reader, force);
}

/**
 * Tells the BamReader how to create objects of type Geom.
 */
void GeomLinesAdjacency::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Geom is encountered in the Bam file.  It should create the Geom and
 * extract its information from the file.
 */
TypedWritable *GeomLinesAdjacency::
make_from_bam(const FactoryParams &params) {
  GeomLinesAdjacency *object = new GeomLinesAdjacency(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
