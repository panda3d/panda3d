// Filename: triangleMesh.cxx
// Created by:  drose (06Nov99)
// 
////////////////////////////////////////////////////////////////////

#include "triangleMesh.h"

#include <geomTristrip.h>

TriangleMesh::
TriangleMesh(int x_verts, int y_verts) :
  _coords(0), _norms(0), _colors(0), _texcoords(0),
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

  PTA(int) lengths(num_tstrips);
  PTA(ushort) vindex(num_tstrips * tstrip_length);

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

