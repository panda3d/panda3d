/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionLine.cxx
 * @author drose
 * @date 2005-01-05
 */

#include "collisionLine.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "config_collide.h"
#include "geom.h"
#include "geomNode.h"
#include "boundingLine.h"
#include "lensNode.h"
#include "lens.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "geom.h"
#include "geomLinestrips.h"
#include "geomVertexWriter.h"

TypeHandle CollisionLine::_type_handle;


/**
 *
 */
CollisionSolid *CollisionLine::
make_copy() {
  return new CollisionLine(*this);
}

/**
 *
 */
PT(CollisionEntry) CollisionLine::
test_intersection(const CollisionEntry &entry) const {
  return entry.get_into()->test_intersection_from_line(entry);
}

/**
 *
 */
void CollisionLine::
output(std::ostream &out) const {
  out << "line, o (" << get_origin() << "), d (" << get_direction() << ")";
}

/**
 * Fills the _viz_geom GeomNode up with Geoms suitable for rendering this
 * solid.
 */
void CollisionLine::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }

  static const int num_points = 100;
  static const double scale = 100.0;

  PT(GeomVertexData) vdata = new GeomVertexData
    ("collision", GeomVertexFormat::get_v3cp(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter color(vdata, InternalName::get_color());

  for (int i = 0; i < num_points; i++) {
    double t = ((double)i / (double)num_points - 0.5) * 2.0;
    vertex.add_data3(get_origin() + t * scale * get_direction());

    color.add_data4(LColor(1.0f, 1.0f, 1.0f, 1.0f) +
                    fabs(t) * LColor(0.0f, 0.0f, 0.0f, -1.0f));
  }

  PT(GeomLinestrips) line = new GeomLinestrips(Geom::UH_static);
  line->add_next_vertices(num_points);
  line->close_primitive();

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(line);

  _viz_geom->add_geom(geom, get_other_viz_state());
  _bounds_viz_geom->add_geom(geom, get_other_bounds_viz_state());
}

/**
 * Tells the BamReader how to create objects of type CollisionLine.
 */
void CollisionLine::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void CollisionLine::
write_datagram(BamWriter *manager, Datagram &dg) {
  CollisionRay::write_datagram(manager, dg);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type CollisionLine is encountered in the Bam file.  It should create the
 * CollisionLine and extract its information from the file.
 */
TypedWritable *CollisionLine::
make_from_bam(const FactoryParams &params) {
  CollisionLine *node = new CollisionLine();
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new CollisionLine.
 */
void CollisionLine::
fillin(DatagramIterator &scan, BamReader *manager) {
  CollisionRay::fillin(scan, manager);
}
