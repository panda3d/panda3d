// Filename: xFileMesh.h
// Created by:  drose (19Jun01)
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

#ifndef XFILEMESH_H
#define XFILEMESH_H

#include "pandatoolbase.h"
#include "pvector.h"
#include "pmap.h"
#include "indirectCompareTo.h"
#include "namable.h"
#include "coordinateSystem.h"

class XFileMesh;
class XFileVertex;
class XFileNormal;
class XFileMaterial;
class XFileFace;
class XFileToEggConverter;
class EggGroupNode;
class EggVertex;
class EggPolygon;
class EggPrimitive;
class Datagram;

////////////////////////////////////////////////////////////////////
//       Class : XFileMesh
// Description : This is a collection of polygons; i.e. a polyset.
////////////////////////////////////////////////////////////////////
class XFileMesh : public Namable {
public:
  XFileMesh(CoordinateSystem cs = CS_yup_left);
  ~XFileMesh();

  void clear();

  void add_polygon(EggPolygon *egg_poly);
  int add_vertex(EggVertex *egg_vertex, EggPrimitive *egg_prim);
  int add_normal(EggVertex *egg_vertex, EggPrimitive *egg_prim);
  int add_material(EggPrimitive *egg_prim);

  int add_vertex(XFileVertex *vertex);
  int add_normal(XFileNormal *normal);
  int add_material(XFileMaterial *material);

  bool create_polygons(EggGroupNode *egg_parent, 
                       XFileToEggConverter *converter);

  bool has_normals() const;
  bool has_colors() const;
  bool has_uvs() const;
  bool has_materials() const;

  int get_num_materials() const;
  XFileMaterial *get_material(int n) const;

  void make_mesh_data(Datagram &raw_data);
  void make_normal_data(Datagram &raw_data);
  void make_color_data(Datagram &raw_data);
  void make_uv_data(Datagram &raw_data);
  void make_material_list_data(Datagram &raw_data);

  bool read_mesh_data(const Datagram &raw_data);
  bool read_normal_data(const Datagram &raw_data);
  bool read_color_data(const Datagram &raw_data);
  bool read_uv_data(const Datagram &raw_data);
  bool read_material_list_data(const Datagram &raw_data);

private:
  typedef pvector<XFileVertex *> Vertices;
  typedef pvector<XFileNormal *> Normals;
  typedef pvector<XFileMaterial *> Materials;
  typedef pvector<XFileFace *> Faces;

  CoordinateSystem _cs;

  Vertices _vertices;
  Normals _normals;
  Materials _materials;
  Faces _faces;

  typedef pmap<XFileVertex *, int, IndirectCompareTo<XFileVertex> > UniqueVertices;
  typedef pmap<XFileNormal *, int, IndirectCompareTo<XFileNormal> > UniqueNormals;
  typedef pmap<XFileMaterial *, int, IndirectCompareTo<XFileMaterial> > UniqueMaterials;
  UniqueVertices _unique_vertices;
  UniqueNormals _unique_normals;
  UniqueMaterials _unique_materials;

  bool _has_normals;
  bool _has_colors;
  bool _has_uvs;
  bool _has_materials;
};

#endif

