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

class XFileNode;
class XFileDataNode;
class XFileMesh;
class XFileVertex;
class XFileNormal;
class XFileMaterial;
class XFileFace;
class XFileToEggConverter;
class XFileDataNode;
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

  void set_egg_parent(EggGroupNode *egg_parent);

  bool create_polygons(XFileToEggConverter *converter);

  bool has_normals() const;
  bool has_colors() const;
  bool has_uvs() const;
  bool has_materials() const;

  int get_num_materials() const;
  XFileMaterial *get_material(int n) const;

  XFileDataNode *make_x_mesh(XFileNode *x_parent, const string &suffix);
  XFileDataNode *make_x_normals(XFileNode *x_mesh, const string &suffix);
  XFileDataNode *make_x_colors(XFileNode *x_mesh, const string &suffix);
  XFileDataNode *make_x_uvs(XFileNode *x_mesh, const string &suffix);
  XFileDataNode *make_x_material_list(XFileNode *x_mesh, const string &suffix);

  bool fill_mesh(XFileDataNode *obj);
  bool fill_mesh_child(XFileDataNode *obj);
  bool fill_normals(XFileDataNode *obj);
  bool fill_colors(XFileDataNode *obj);
  bool fill_uvs(XFileDataNode *obj);
  bool fill_skin_weights(XFileDataNode *obj);
  bool fill_material_list(XFileDataNode *obj);

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

  typedef pmap<int, float> WeightMap;

  class SkinWeightsData {
  public:
    string _joint_name;
    WeightMap _weight_map;
    LMatrix4d _matrix_offset;
  };
  typedef pvector<SkinWeightsData> SkinWeights;
  SkinWeights _skin_weights;

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

  EggGroupNode *_egg_parent;
};

#endif

