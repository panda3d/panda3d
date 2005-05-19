// Filename: sceneGraphAnalyzer.h
// Created by:  drose (02Jul00)
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

#ifndef SCENEGRAPHANALYZER_H
#define SCENEGRAPHANALYZER_H

#include "pandabase.h"

#include "typedObject.h"
#include "luse.h"

#include "pmap.h"

class PandaNode;
class GeomNode;
class Geom;
class Texture;

////////////////////////////////////////////////////////////////////
//       Class : SceneGraphAnalyzer
// Description : A handy class that can scrub over a scene graph and
//               collect interesting statistics on it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SceneGraphAnalyzer {
public:
  SceneGraphAnalyzer();
  ~SceneGraphAnalyzer();

  void clear();
  void add_node(PandaNode *node);

  void write(ostream &out, int indent_level = 0) const;

private:
  void collect_statistics(PandaNode *node, bool under_instance);
  void collect_statistics(GeomNode *geom_node);
  void collect_statistics(const Geom *geom);
  void collect_statistics(Texture *texture);

  void consider_normals(const Normalf *norms, const unsigned short *nindex,
                        int num);

  typedef pmap<PandaNode *, int> Nodes;
  typedef pmap<Texture *, int> Textures;

  Nodes _nodes;
  Textures _textures;

public:
  int _num_nodes;
  int _num_instances;
  int _num_transforms;
  int _num_nodes_with_attribs;
  int _num_geom_nodes;
  int _num_geoms;

  int _num_vertices;
  int _num_normals;
  int _num_texcoords;
  int _num_tris;
  int _num_quads;
  int _num_polys;
  int _num_lines;
  int _num_points;
  int _num_spheres;

  int _num_individual_tris;
  int _num_tristrips;
  int _num_triangles_in_strips;
  int _num_trifans;
  int _num_triangles_in_fans;

  int _texture_bytes;

  int _num_long_normals;
  int _num_short_normals;
  float _total_normal_length;
};

#endif
