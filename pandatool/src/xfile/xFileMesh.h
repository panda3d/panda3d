// Filename: xFileMesh.h
// Created by:  drose (19Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef XFILEMESH_H
#define XFILEMESH_H

#include "pandatoolbase.h"
#include "pvector.h"
#include "pmap.h"
#include "indirectCompareTo.h"

class XFileMesh;
class XFileVertex;
class XFileNormal;
class XFileFace;
class EggVertex;
class EggPolygon;
class EggPrimitive;
class Datagram;

////////////////////////////////////////////////////////////////////
//       Class : XFileMesh
// Description : This is a collection of polygons; i.e. a polyset.
////////////////////////////////////////////////////////////////////
class XFileMesh {
public:
  XFileMesh();
  ~XFileMesh();

  void add_polygon(EggPolygon *egg_poly);
  int add_vertex(EggVertex *egg_vertex, EggPrimitive *egg_prim);
  int add_normal(EggVertex *egg_vertex, EggPrimitive *egg_prim);

  bool has_normals() const;
  bool has_colors() const;
  bool has_uvs() const;

  void make_mesh_data(Datagram &raw_data);
  void make_normal_data(Datagram &raw_data);
  void make_color_data(Datagram &raw_data);
  void make_uv_data(Datagram &raw_data);

private:
  typedef pvector<XFileVertex *> Vertices;
  typedef pvector<XFileNormal *> Normals;
  typedef pvector<XFileFace *> Faces;

  Vertices _vertices;
  Normals _normals;
  Faces _faces;

private:
  typedef pmap<XFileVertex *, int, IndirectCompareTo<XFileVertex> > UniqueVertices;
  typedef pmap<XFileNormal *, int, IndirectCompareTo<XFileNormal> > UniqueNormals;
  UniqueVertices _unique_vertices;
  UniqueNormals _unique_normals;

  bool _has_normals;
  bool _has_colors;
  bool _has_uvs;
};

#endif

