// Filename: geomQuad.cxx
// Created by:  charles (13Jul00)
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

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "ioPtaDatagramShort.h"
#include "ioPtaDatagramInt.h"
#include "ioPtaDatagramLinMath.h"
#include "graphicsStateGuardianBase.h"

#include "geomTri.h"
#include "geomQuad.h"

TypeHandle GeomQuad::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomQuad::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *GeomQuad::
make_copy() const {
  return new GeomQuad(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomQuad::print_draw_immediate
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GeomQuad::
print_draw_immediate(void) const {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomQuad::draw_immediate
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GeomQuad::
draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc) {
  gsg->draw_quad(this, gc);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomQuad::get_tris
//       Access: Public, Virtual
//  Description: This is similar in principle to explode(), except it
//               returns only a list of triangle vertex indices, with
//               no information about color or whatever.  The array
//               returned is a set of indices into the geom's _coords
//               array, as retrieve by get_coords(); there will be 3*n
//               elements in the array, where n is the number of
//               triangles described by the geometry.  This is useful
//               when it's important to determine the physical
//               structure of the geometry, without necessarily
//               worrying about its rendering properties, and when
//               performance considerations are not overwhelming.
////////////////////////////////////////////////////////////////////
PTA_ushort GeomQuad::
get_tris() const {
  int num_tris = _numprims * 2;
  PTA_ushort tris;
  tris.reserve(num_tris * 3);

  int k = 0;

  for (int i = 0; i < _numprims; i++) {
    ushort indices[4];
    if (_vindex.empty()) {
      for (int j = 0; j < 4; j++) {
        indices[j] = k++;
      }
    } else {
      for (int j = 0; j < 4; j++) {
        indices[j] = _vindex[k++];
      }
    }

    // First tri.  Vertices 0, 1, 2.
    tris.push_back(indices[0]);
    tris.push_back(indices[1]);
    tris.push_back(indices[2]);

    // Second tri.  Vertices 0, 2, 3.
    tris.push_back(indices[0]);
    tris.push_back(indices[2]);
    tris.push_back(indices[3]);
  }

  nassertr((int)tris.size() == num_tris * 3, PTA_ushort());
  return tris;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomQuad::make_GeomQuad
//       Access: Protected
//  Description: Factory method to generate a GeomQuad object
////////////////////////////////////////////////////////////////////
TypedWritable* GeomQuad::
make_GeomQuad(const FactoryParams &params) {
  GeomQuad *me = new GeomQuad;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomQuad::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomQuad object
////////////////////////////////////////////////////////////////////
void GeomQuad::
register_with_read_factory(void) {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomQuad);
}
