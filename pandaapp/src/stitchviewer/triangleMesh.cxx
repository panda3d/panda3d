// Filename: triangleMesh.cxx
// Created by:  drose (06Nov99)
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

#include "triangleMesh.h"

#include "geomTristrip.h"

TriangleMesh::
TriangleMesh(int x_verts, int y_verts) :
  _coords(PTA_Vertexf::empty_array(0)),
  _norms(PTA_Normalf::empty_array(0)),
  _colors(PTA_Colorf::empty_array(0)),
  _texcoords(PTA_TexCoordf::empty_array(0)),
  _x_verts(x_verts),
  _y_verts(y_verts)
{
}

int TriangleMesh::
get_x_verts() const {
  return _x_verts;
}

int TriangleMesh::
get_y_verts() const {
  return _y_verts;
}

int TriangleMesh::
get_num_verts() const {
  return _x_verts * _y_verts;
}

GeomTristrip *TriangleMesh::
build_mesh() const {
  //  int num_verts = _x_verts * _y_verts;
  int num_tstrips = (_y_verts-1);
  int tstrip_length = 2*(_x_verts-1)+2;

  PTA_int lengths = PTA_int::empty_array(num_tstrips);
  PTA_ushort vindex = PTA_ushort::empty_array(num_tstrips * tstrip_length);

  // Set the lengths array.  We are creating num_tstrips T-strips,
  // each of which has t_strip length vertices.
  int n;
  for (n = 0; n < num_tstrips; n++) {
    lengths[n] = tstrip_length;
  }

  // Now fill up the index array into the vertices.  This lays out the
  // order of the vertices in each T-strip.
  n = 0;
  int ti, si;
  for (ti = 1; ti < _y_verts; ti++) {
    vindex[n++] = ti * _x_verts;
    for (si = 1; si < _x_verts; si++) {
      vindex[n++] = (ti - 1) * _x_verts + (si-1);
      vindex[n++] = ti * _x_verts + si;
    }
    vindex[n++] = (ti - 1) * _x_verts + (_x_verts-1);
  }
  assert(n==num_tstrips * tstrip_length);

  GeomTristrip *geom = new GeomTristrip;
  geom->set_num_prims(num_tstrips);
  geom->set_lengths(lengths);

  assert(!_coords.empty());
  geom->set_coords(_coords, G_PER_VERTEX, vindex);

  if (!_norms.empty()) {
    geom->set_normals(_norms, G_PER_VERTEX, vindex);
  }

  if (!_colors.empty()) {
    geom->set_colors(_colors, G_PER_VERTEX, vindex);
  }

  if (!_texcoords.empty()) {
    geom->set_texcoords(_texcoords, G_PER_VERTEX, vindex);
  }

  return geom;
}

