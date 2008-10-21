// Filename: bamToEgg.h
// Created by:  drose (25Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef BAMTOEGG_H
#define BAMTOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"
#include "luse.h"
#include "eggTextureCollection.h"
#include "eggMaterialCollection.h"

class WorkingNodePath;
class EggGroup;
class EggGroupNode;
class EggVertexPool;
class EggTexture;
class LODNode;
class SequenceNode;
class SwitchNode;
class CollisionNode;
class GeomNode;
class GeomTri;
class GeomVertexData;
class GeomTriangles;
class PandaNode;
class RenderState;
class Texture;

////////////////////////////////////////////////////////////////////
//       Class : BamToEgg
// Description : This program reads a bam file, for instance as
//               written out from a real-time interaction session, and
//               generates a corresponding egg file.
////////////////////////////////////////////////////////////////////
class BamToEgg : public SomethingToEgg {
public:
  BamToEgg();

  void run();

private:
  void convert_node(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
                    bool has_decal);
  void convert_lod_node(LODNode *node, const WorkingNodePath &node_path,
                        EggGroupNode *egg_parent, bool has_decal);
  void convert_sequence_node(SequenceNode *node, const WorkingNodePath &node_path,
                        EggGroupNode *egg_parent, bool has_decal);
  void convert_switch_node(SwitchNode *node, const WorkingNodePath &node_path,
                        EggGroupNode *egg_parent, bool has_decal);
  void convert_collision_node(CollisionNode *node, const WorkingNodePath &node_path,
                        EggGroupNode *egg_parent, bool has_decal);
  void convert_geom_node(GeomNode *node, const WorkingNodePath &node_path, 
                         EggGroupNode *egg_parent, bool has_decal);
  void convert_triangles(const GeomVertexData *vertex_data,
                         const GeomTriangles *primitive, 
                         const RenderState *net_state, 
                         const LMatrix4f &net_mat, EggGroupNode *egg_parent);

  void recurse_nodes(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
                     bool has_decal);
  bool apply_node_properties(EggGroup *egg_group, PandaNode *node, bool allow_backstage = true);

  EggTexture *get_egg_texture(Texture *tex);

  EggVertexPool *_vpool;
  EggTextureCollection _textures;
  EggMaterialCollection _materials;
};

#endif
