/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomLinestrips.cxx
 * @author drose
 * @date 2005-03-22
 */

#include "geomLinestrips.h"
#include "geomLines.h"
#include "geomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"
#include "geomLinestripsAdjacency.h"

using std::map;

TypeHandle GeomLinestrips::_type_handle;

/**
 *
 */
GeomLinestrips::
GeomLinestrips(GeomLinestrips::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}

/**
 *
 */
GeomLinestrips::
GeomLinestrips(const GeomLinestrips &copy) :
  GeomPrimitive(copy)
{
}

/**
 *
 */
GeomLinestrips::
~GeomLinestrips() {
}

/**
 *
 */
PT(GeomPrimitive) GeomLinestrips::
make_copy() const {
  return new GeomLinestrips(*this);
}

/**
 * Returns the fundamental rendering type of this primitive: whether it is
 * points, lines, or polygons.
 *
 * This is used to set up the appropriate antialiasing settings when
 * AntialiasAttrib::M_auto is in effect; it also implies the type of primitive
 * that will be produced when decompose() is called.
 */
GeomPrimitive::PrimitiveType GeomLinestrips::
get_primitive_type() const {
  return PT_lines;
}

/**
 * Returns the set of GeomRendering bits that represent the rendering
 * properties required to properly render this primitive.
 */
int GeomLinestrips::
get_geom_rendering() const {
  if (is_indexed()) {
    if (get_num_primitives() > 1) {
      return GR_line_strip | GR_indexed_other | GR_strip_cut_index;
    } else {
      return GR_line_strip | GR_indexed_other;
    }
  } else {
    return GR_line_strip;
  }
}

/**
 * Adds adjacency information to this primitive.  May return null if this type
 * of geometry does not support adjacency information.
 */
CPT(GeomPrimitive) GeomLinestrips::
make_adjacency() const {
  Thread *current_thread = Thread::get_current_thread();
  PT(GeomLinestripsAdjacency) adj = new GeomLinestripsAdjacency(get_usage_hint());

  GeomPrimitivePipelineReader from(this, current_thread);
  int num_vertices = from.get_num_vertices();
  CPTA_int ends = get_ends();

  const int num_unused = 1;

  // First, build a map of each vertex to its next vertex, and another map
  // doing the exact reverse.
  map<int, int> forward_map, reverse_map;
  int vi = -num_unused;
  int li = 0;
  while (li < (int)ends.size()) {
    // Skip unused vertices between linestrips.
    vi += num_unused;
    int end = ends[li];
    nassertr(vi + 1 <= end, nullptr);
    int v0 = from.get_vertex(vi++);
    while (vi < end) {
      int v1 = from.get_vertex(vi++);
      forward_map[v0] = v1;
      reverse_map[v1] = v0;
      v0 = v1;
    }
    ++li;
  }
  nassertr(vi == num_vertices, nullptr);

  // Now build up the new vertex data.  For each linestrip, we prepend and
  // append the appropriate connecting vertices.
  vi = -num_unused;
  li = 0;
  while (li < (int)ends.size()) {
    // Skip unused vertices between linestrips.
    vi += num_unused;
    int end = ends[li];
    nassertr(vi + 1 <= end, nullptr);

    // Look for the line segment connected to the beginning of this strip.
    int v0 = from.get_vertex(vi++);
    auto it = reverse_map.find(v0);
    if (it != reverse_map.end()) {
      adj->add_vertex(it->second);
    } else {
      // Um, no adjoining line segment?  Just repeat the vertex, I guess.
      adj->add_vertex(v0);
    }

    // Add the actual vertices in the strip.
    adj->add_vertex(v0);
    int v1 = v0;
    while (vi < end) {
      v1 = from.get_vertex(vi++);
      adj->add_vertex(v1);
    }

    // Do the same for the last vertex in the strip.
    it = forward_map.find(v1);
    if (it != forward_map.end()) {
      adj->add_vertex(it->second);
    } else {
      adj->add_vertex(v1);
    }

    adj->close_primitive();
    ++li;
  }
  nassertr(vi == num_vertices, nullptr);

  return adj;
}

/**
 * Returns the minimum number of vertices that must be added before
 * close_primitive() may legally be called.
 */
int GeomLinestrips::
get_min_num_vertices_per_primitive() const {
  return 2;
}

/**
 * Returns the number of vertices that are added between primitives that
 * aren't, strictly speaking, part of the primitives themselves.  This is
 * used, for instance, to define degenerate triangles to connect otherwise
 * disconnected triangle strips.
 */
int GeomLinestrips::
get_num_unused_vertices_per_primitive() const {
  return 1;
}

/**
 * Calls the appropriate method on the GSG to draw the primitive.
 */
bool GeomLinestrips::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_linestrips(reader, force);
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
CPT(GeomPrimitive) GeomLinestrips::
decompose_impl() const {
  PT(GeomLines) lines = new GeomLines(get_usage_hint());
  lines->set_shade_model(get_shade_model());
  CPTA_int ends = get_ends();

  int num_unused = get_num_unused_vertices_per_primitive();

  int vi = -num_unused;
  int li = 0;
  while (li < (int)ends.size()) {
    // Skip unused vertices between tristrips.
    vi += num_unused;
    int end = ends[li];
    nassertr(vi + 1 <= end, lines);
    int v0 = get_vertex(vi);
    ++vi;
    while (vi < end) {
      int v1 = get_vertex(vi);
      ++vi;
      lines->add_vertex(v0);
      lines->add_vertex(v1);
      v0 = v1;
      lines->close_primitive();
    }
    ++li;
  }
  nassertr(vi == get_num_vertices(), nullptr);

  return lines;
}

/**
 * The virtual implementation of do_rotate().
 */
CPT(GeomVertexArrayData) GeomLinestrips::
rotate_impl() const {
  // To rotate a line strip, we just reverse the vertices.
  CPTA_int ends = get_ends();
  PT(GeomVertexArrayData) new_vertices = make_index_data();
  new_vertices->set_num_rows(get_num_vertices());

  if (is_indexed()) {
    CPT(GeomVertexArrayData) vertices = get_vertices();
    GeomVertexReader from(vertices, 0);
    GeomVertexWriter to(new_vertices, 0);

    int begin = 0;
    CPTA_int::const_iterator ei;
    for (ei = ends.begin(); ei != ends.end(); ++ei) {
      int end = (*ei);
      for (int vi = end - 1; vi >= begin; --vi) {
        from.set_row_unsafe(vi);
        to.set_data1i(from.get_data1i());
      }
      begin = end;
    }

    nassertr(to.is_at_end(), nullptr);

  } else {
    // Nonindexed case.
    int first_vertex = get_first_vertex();
    GeomVertexWriter to(new_vertices, 0);

    int begin = 0;
    CPTA_int::const_iterator ei;
    for (ei = ends.begin(); ei != ends.end(); ++ei) {
      int end = (*ei);
      for (int vi = end - 1; vi >= begin; --vi) {
        to.set_data1i(vi + first_vertex);
      }
      begin = end;
    }

    nassertr(to.is_at_end(), nullptr);
  }
  return new_vertices;
}

/**
 * Should be redefined to return true in any primitive that implements
 * append_unused_vertices().
 */
bool GeomLinestrips::
requires_unused_vertices() const {
  return true;
}

/**
 * Called when a new primitive is begun (other than the first primitive), this
 * should add some degenerate vertices between primitives, if the primitive
 * type requires that.  The second parameter is the first vertex that begins
 * the new primitive.
 */
void GeomLinestrips::
append_unused_vertices(GeomVertexArrayData *vertices, int vertex) {
  GeomVertexWriter to(vertices, 0);
  to.set_row_unsafe(vertices->get_num_rows());
  to.add_data1i(get_strip_cut_index());
}

/**
 * Tells the BamReader how to create objects of type Geom.
 */
void GeomLinestrips::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Geom is encountered in the Bam file.  It should create the Geom and
 * extract its information from the file.
 */
TypedWritable *GeomLinestrips::
make_from_bam(const FactoryParams &params) {
  GeomLinestrips *object = new GeomLinestrips(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
