// Filename: indexedFaceSet.h
// Created by:  drose (24Jun99)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

#ifndef INDEXEDFACESET_H
#define INDEXEDFACESET_H

#include "pandatoolbase.h"
#include "pvector.h"
#include "pset.h"
#include "eggPolygon.h"
#include "eggVertex.h"
#include "eggAttributes.h"

class VrmlNode;
class EggData;
class EggGroup;
class EggVertexPool;
class VRMLAppearance;
class LMatrix4d;

////////////////////////////////////////////////////////////////////
//       Class : IndexedFaceSet
// Description : Decodes the vertices and faces in a VRML indexed face
//               set, and creates the corresponding egg geometry.
////////////////////////////////////////////////////////////////////
class IndexedFaceSet {
public:
  IndexedFaceSet(const VrmlNode *geometry, const VRMLAppearance &appearance);

  void convert_to_egg(EggGroup *group, const LMatrix4d &net_transform);

private:
  void get_coord_values();
  void get_polys();
  void get_vrml_colors(const VrmlNode *color_node, double transparency,
		       pvector<Colorf> &color_list);
  void get_vrml_normals(const VrmlNode *normal_node, 
			pvector<Normald> &normal_list);
  void get_vrml_uvs(const VrmlNode *texCoord_node, 
		    pvector<TexCoordd> &uv_list);

  bool get_colors();
  bool get_normals();
  void assign_per_vertex_normals();
  bool get_uvs();
  void assign_per_vertex_uvs();
  void make_polys(EggVertexPool *vpool, EggGroup *group,
		  const LMatrix4d &net_transform);
  void compute_normals(EggGroup *group);

  class VrmlVertex {
  public:
    int _index;
    Vertexd _pos;
    EggVertex _attrib;
  };
  class VrmlPolygon {
  public:
    EggPolygon _attrib;
    pvector<VrmlVertex> _verts;
  };
  pvector<Vertexd> _coord_values;
  pvector<VrmlPolygon> _polys;
  pvector<TexCoordd> _per_vertex_uvs;
  pvector<Normald> _per_vertex_normals;

  bool _has_normals;

  const VrmlNode *_geometry;
  const VRMLAppearance &_appearance;
};

#endif
