/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indexedFaceSet.h
 * @author drose
 * @date 1999-06-24
 */

#ifndef INDEXEDFACESET_H
#define INDEXEDFACESET_H

#include "pandatoolbase.h"
#include "pvector.h"
#include "epvector.h"
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

/**
 * Decodes the vertices and faces in a VRML indexed face set, and creates the
 * corresponding egg geometry.
 */
class IndexedFaceSet {
public:
  IndexedFaceSet(const VrmlNode *geometry, const VRMLAppearance &appearance);

  void convert_to_egg(EggGroup *group, const LMatrix4d &net_transform);

private:
  void get_coord_values();
  void get_polys();
  void get_vrml_colors(const VrmlNode *color_node, double transparency,
                       pvector<UnalignedLVecBase4> &color_list);
  void get_vrml_normals(const VrmlNode *normal_node,
                        pvector<LNormald> &normal_list);
  void get_vrml_uvs(const VrmlNode *texCoord_node,
                    pvector<LTexCoordd> &uv_list);

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
    LVertexd _pos;
    EggVertex _attrib;
  };
  class VrmlPolygon {
  public:
    EggPolygon _attrib;
    epvector<VrmlVertex> _verts;
  };
  pvector<LVertexd> _coord_values;
  epvector<VrmlPolygon> _polys;
  pvector<LTexCoordd> _per_vertex_uvs;
  pvector<LNormald> _per_vertex_normals;

  bool _has_normals;

  const VrmlNode *_geometry;
  const VRMLAppearance &_appearance;
};

#endif
