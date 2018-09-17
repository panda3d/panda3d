/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomLines.cxx
 * @author drose
 * @date 2005-03-22
 */

#include "geomLines.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "geomLinesAdjacency.h"

using std::map;

TypeHandle GeomLines::_type_handle;

/**
 *
 */
GeomLines::
GeomLines(GeomLines::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}

/**
 *
 */
GeomLines::
GeomLines(const GeomLines &copy) :
  GeomPrimitive(copy)
{
}

/**
 *
 */
GeomLines::
~GeomLines() {
}

/**
 *
 */
PT(GeomPrimitive) GeomLines::
make_copy() const {
  return new GeomLines(*this);
}

/**
 * Returns the fundamental rendering type of this primitive: whether it is
 * points, lines, or polygons.
 *
 * This is used to set up the appropriate antialiasing settings when
 * AntialiasAttrib::M_auto is in effect; it also implies the type of primitive
 * that will be produced when decompose() is called.
 */
GeomPrimitive::PrimitiveType GeomLines::
get_primitive_type() const {
  return PT_lines;
}

/**
 * Adds adjacency information to this primitive.  May return null if this type
 * of geometry does not support adjacency information.
 */
CPT(GeomPrimitive) GeomLines::
make_adjacency() const {
 Thread *current_thread = Thread::get_current_thread();
  PT(GeomLinesAdjacency) adj = new GeomLinesAdjacency(get_usage_hint());

  GeomPrimitivePipelineReader from(this, current_thread);
  int num_vertices = from.get_num_vertices();

  PT(GeomVertexArrayData) new_vertices = adj->make_index_data();
  new_vertices->set_num_rows(num_vertices * 2);

  // First, build a map of each vertex to its next vertex, and another map
  // doing the exact reverse.  Note that this only considers lines that are
  // ordered the same way as being connected; we may need to change this.
  map<int, int> forward_map, reverse_map;
  for (int i = 0; i < num_vertices; i += 2) {
    int v0 = from.get_vertex(i);
    int v1 = from.get_vertex(i + 1);
    forward_map[v0] = v1;
    reverse_map[v1] = v0;
  }

  // Now build up the new vertex data.  For each line, we insert the
  // appropriate connecting vertex.
  {
    GeomVertexWriter to(new_vertices, 0);
    for (int i = 0; i < num_vertices; i += 2) {
      int v0 = from.get_vertex(i);
      int v1 = from.get_vertex(i + 1);

      auto it = reverse_map.find(v0);
      if (it != reverse_map.end()) {
        to.set_data1i(it->second);
      } else {
        // Um, no adjoining line segment?  Just repeat the vertex, I guess.
        to.set_data1i(v0);
      }

      to.set_data1i(v0);
      to.set_data1i(v1);

      // Do the same for the second vertex in the line.
      it = forward_map.find(v1);
      if (it != forward_map.end()) {
        to.set_data1i(it->second);
      } else {
        to.set_data1i(v1);
      }
    }

    nassertr(to.is_at_end(), nullptr);
  }

  adj->set_vertices(std::move(new_vertices));
  return adj;
}

/**
 * If the primitive type is a simple type in which all primitives have the
 * same number of vertices, like lines, returns the number of vertices per
 * primitive.  If the primitive type is a more complex type in which different
 * primitives might have different numbers of vertices, for instance a line
 * strip, returns 0.
 */
int GeomLines::
get_num_vertices_per_primitive() const {
  return 2;
}

/**
 * Returns the minimum number of vertices that must be added before
 * close_primitive() may legally be called.
 */
int GeomLines::
get_min_num_vertices_per_primitive() const {
  return 2;
}

/**
 * Calls the appropriate method on the GSG to draw the primitive.
 */
bool GeomLines::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_lines(reader, force);
}

/**
 * The virtual implementation of do_rotate().
 */
CPT(GeomVertexArrayData) GeomLines::
rotate_impl() const {
  // To rotate lines, we just move reverse the pairs of vertices.
  int num_vertices = get_num_vertices();

  PT(GeomVertexArrayData) new_vertices = make_index_data();
  new_vertices->set_num_rows(num_vertices);

  if (is_indexed()) {
    CPT(GeomVertexArrayData) vertices = get_vertices();
    GeomVertexReader from(vertices, 0);
    GeomVertexWriter to(new_vertices, 0);

    for (int begin = 0; begin < num_vertices; begin += 2) {
      from.set_row_unsafe(begin + 1);
      to.set_data1i(from.get_data1i());
      from.set_row_unsafe(begin);
      to.set_data1i(from.get_data1i());
    }

    nassertr(to.is_at_end(), nullptr);

  } else {
    // Nonindexed case.
    int first_vertex = get_first_vertex();
    GeomVertexWriter to(new_vertices, 0);

    for (int begin = 0; begin < num_vertices; begin += 2) {
      to.set_data1i(begin + 1 + first_vertex);
      to.set_data1i(begin + first_vertex);
    }

    nassertr(to.is_at_end(), nullptr);
  }

  return new_vertices;
}

/**
 * Tells the BamReader how to create objects of type Geom.
 */
void GeomLines::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Geom is encountered in the Bam file.  It should create the Geom and
 * extract its information from the file.
 */
TypedWritable *GeomLines::
make_from_bam(const FactoryParams &params) {
  GeomLines *object = new GeomLines(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
