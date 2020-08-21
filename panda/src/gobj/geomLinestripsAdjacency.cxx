/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomLinestripsAdjacency.cxx
 * @author rdb
 * @date 2018-03-01
 */

#include "geomLinestripsAdjacency.h"
#include "geomLines.h"
#include "geomLinesAdjacency.h"
#include "geomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"

TypeHandle GeomLinestripsAdjacency::_type_handle;

/**
 *
 */
GeomLinestripsAdjacency::
GeomLinestripsAdjacency(GeomEnums::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}

/**
 *
 */
GeomLinestripsAdjacency::
GeomLinestripsAdjacency(const GeomLinestripsAdjacency &copy) :
  GeomPrimitive(copy)
{
}

/**
 *
 */
GeomLinestripsAdjacency::
~GeomLinestripsAdjacency() {
}

/**
 *
 */
PT(GeomPrimitive) GeomLinestripsAdjacency::
make_copy() const {
  return new GeomLinestripsAdjacency(*this);
}

/**
 * Returns the fundamental rendering type of this primitive: whether it is
 * points, lines, or polygons.
 *
 * This is used to set up the appropriate antialiasing settings when
 * AntialiasAttrib::M_auto is in effect; it also implies the type of primitive
 * that will be produced when decompose() is called.
 */
GeomPrimitive::PrimitiveType GeomLinestripsAdjacency::
get_primitive_type() const {
  return PT_lines;
}

/**
 * Returns the set of GeomRendering bits that represent the rendering
 * properties required to properly render this primitive.
 */
int GeomLinestripsAdjacency::
get_geom_rendering() const {
  if (is_indexed()) {
    if (get_num_primitives() > 1) {
      return GR_line_strip | GR_indexed_other | GR_strip_cut_index | GR_adjacency;
    } else {
      return GR_line_strip | GR_indexed_other | GR_adjacency;
    }
  } else {
    return GR_line_strip | GR_adjacency;
  }
}

/**
 * Returns the minimum number of vertices that must be added before
 * close_primitive() may legally be called.
 */
int GeomLinestripsAdjacency::
get_min_num_vertices_per_primitive() const {
  return 4;
}

/**
 * Returns the number of vertices that are added between primitives that
 * aren't, strictly speaking, part of the primitives themselves.  This is
 * used, for instance, to define degenerate triangles to connect otherwise
 * disconnected triangle strips.
 */
int GeomLinestripsAdjacency::
get_num_unused_vertices_per_primitive() const {
  return 1;
}

/**
 * Calls the appropriate method on the GSG to draw the primitive.
 */
bool GeomLinestripsAdjacency::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_linestrips_adj(reader, force);
}

/**
 * Decomposes a complex primitive type into a simpler primitive type, for
 * instance line strips to lines, and returns a pointer to the new primitive
 * definition.  If the decomposition cannot be performed, this might return
 * the original object.
 *
 * This method is useful for application code that wants to iterate through
 * the set of lines on the primitive without having to write handlers for each
 * possible kind of primitive type.
 */
CPT(GeomPrimitive) GeomLinestripsAdjacency::
decompose_impl() const {
  Thread *current_thread = Thread::get_current_thread();
  PT(GeomLinesAdjacency) lines = new GeomLinesAdjacency(get_usage_hint());
  lines->set_shade_model(get_shade_model());

  GeomPrimitivePipelineReader from(this, current_thread);
  int num_vertices = from.get_num_vertices();
  CPTA_int ends = get_ends();

  const int num_unused = 1;

  int vi = -num_unused;
  int li = 0;
  while (li < (int)ends.size()) {
    // Skip unused vertices between tristrips.
    vi += num_unused;
    int end = ends[li];
    nassertr(vi + 3 <= end, lines);
    int v0 = from.get_vertex(vi++);
    int v1 = from.get_vertex(vi++);
    int v2 = from.get_vertex(vi++);
    while (vi < end) {
      int v3 = from.get_vertex(vi++);
      lines->add_vertex(v0);
      lines->add_vertex(v1);
      lines->add_vertex(v2);
      lines->add_vertex(v3);
      v0 = v1;
      v1 = v2;
      v2 = v3;
    }
    ++li;
  }
  nassertr(vi == num_vertices, nullptr);

  return lines;
}

/**
 * Should be redefined to return true in any primitive that implements
 * append_unused_vertices().
 */
bool GeomLinestripsAdjacency::
requires_unused_vertices() const {
  return true;
}

/**
 * Called when a new primitive is begun (other than the first primitive), this
 * should add some degenerate vertices between primitives, if the primitive
 * type requires that.  The second parameter is the first vertex that begins
 * the new primitive.
 */
void GeomLinestripsAdjacency::
append_unused_vertices(GeomVertexArrayData *vertices, int vertex) {
  GeomVertexWriter to(vertices, 0);
  to.set_row_unsafe(vertices->get_num_rows());
  to.add_data1i(get_strip_cut_index());
}

/**
 * Tells the BamReader how to create objects of type Geom.
 */
void GeomLinestripsAdjacency::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Geom is encountered in the Bam file.  It should create the Geom and
 * extract its information from the file.
 */
TypedWritable *GeomLinestripsAdjacency::
make_from_bam(const FactoryParams &params) {
  GeomLinestripsAdjacency *object = new GeomLinestripsAdjacency(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
