// Filename: collisionLine.cxx
// Created by:  drose (05Jan05)
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

#include "collisionLine.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "config_collide.h"
#include "geom.h"
#include "geomNode.h"
#include "geomLinestrip.h"
#include "boundingLine.h"
#include "lensNode.h"
#include "lens.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle CollisionLine::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionLine::
make_copy() {
  return new CollisionLine(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::test_intersection
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionLine::
test_intersection(const CollisionEntry &entry) const {
  return entry.get_into()->test_intersection_from_line(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionLine::
output(ostream &out) const {
  out << "line, o (" << get_origin() << "), d (" << get_direction() << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
void CollisionLine::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }

  GeomLinestrip *line = new GeomLinestrip;
  PTA_Vertexf verts;
  PTA_Colorf colors;
  PTA_int lengths;

  static const int num_points = 100;
  static const double scale = 100.0;

  verts.reserve(num_points);
  colors.reserve(num_points);

  for (int i = 0; i < num_points; i++) {
    double t = ((double)i / (double)num_points - 0.5) * 2.0;
    verts.push_back(get_origin() + t * scale * get_direction());

    colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f) +
                     fabs(t) * Colorf(0.0f, 0.0f, 0.0f, -1.0f));
  }
  line->set_coords(verts);
  line->set_colors(colors, G_PER_VERTEX);

  lengths.push_back(num_points-1);
  line->set_lengths(lengths);

  line->set_num_prims(1);

  _viz_geom->add_geom(line, get_other_viz_state());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               CollisionLine.
////////////////////////////////////////////////////////////////////
void CollisionLine::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CollisionLine::
write_datagram(BamWriter *manager, Datagram &dg) {
  CollisionRay::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type CollisionLine is encountered
//               in the Bam file.  It should create the CollisionLine
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *CollisionLine::
make_from_bam(const FactoryParams &params) {
  CollisionLine *node = new CollisionLine();
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLine::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new CollisionLine.
////////////////////////////////////////////////////////////////////
void CollisionLine::
fillin(DatagramIterator &scan, BamReader *manager) {
  CollisionRay::fillin(scan, manager);
}
