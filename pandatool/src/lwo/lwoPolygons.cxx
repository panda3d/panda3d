/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoPolygons.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoPolygons.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoPolygons::_type_handle;

/**
 * Returns the number of polygons of this group.
 */
int LwoPolygons::
get_num_polygons() const {
  return _polygons.size();
}

/**
 * Returns the nth polygon of this group.
 */
LwoPolygons::Polygon *LwoPolygons::
get_polygon(int n) const {
  nassertr(n >= 0 && n < (int)_polygons.size(), nullptr);
  return _polygons[n];
}

/**
 * Reads the data of the chunk in from the given input file, if possible.  The
 * ID and length of the chunk have already been read.  stop_at is the byte
 * position of the file to stop at (based on the current position at
 * in->get_bytes_read()).  Returns true on success, false otherwise.
 */
bool LwoPolygons::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  if (lin->get_lwo_version() >= 6.0) {
    // 6.x style syntax: POLS { type[ID4], ( numvert+flags[U2], vert[VX] #
    // numvert )* }

    _polygon_type = lin->get_id();

    while (lin->get_bytes_read() < stop_at && !lin->is_eof()) {
      int nf = lin->get_be_uint16();
      int num_vertices = nf & PF_numverts_mask;

      PT(Polygon) poly = new Polygon;
      poly->_flags = nf & ~PF_numverts_mask;
      poly->_surface_index = -1;

      for (int i = 0; i < num_vertices; i++) {
        int vindex = lin->get_vx();
        poly->_vertices.push_back(vindex);
      }

      _polygons.push_back(poly);
    }

  } else {
    // 5.x style syntax: POLS { ( numvert[U2], vert[VX] # numvert,
    // +-(surf+1)[I2], numdetail[U2]? )* }
    _polygon_type = IffId("FACE");

    int num_decals = 0;
    while (lin->get_bytes_read() < stop_at && !lin->is_eof()) {
      int num_vertices = lin->get_be_uint16();

      PT(Polygon) poly = new Polygon;
      poly->_flags = 0;

      for (int i = 0; i < num_vertices; i++) {
        int vindex = lin->get_vx();
        poly->_vertices.push_back(vindex);
      }

      int surface = lin->get_be_int16();

      if (num_decals > 0) {
        // This is a decal polygon of a previous polygon.
        num_decals--;
        poly->_flags |= PF_decal;

      } else {
        if (surface < 0) {
          num_decals = lin->get_be_int16();
          surface = -surface;
        }
      }

      // The surface index is stored +1 to allow signedness to be examined.
      poly->_surface_index = surface - 1;

      _polygons.push_back(poly);
    }
  }

  return true;
}

/**
 *
 */
void LwoPolygons::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { polygon_type = " << _polygon_type
    << ", " << _polygons.size() << " polygons }\n";
}
