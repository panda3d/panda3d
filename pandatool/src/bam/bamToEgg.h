// Filename: bamToEgg.h
// Created by:  drose (25Jun01)
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
class GeomNode;
class GeomTri;
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
  void convert_geom_node(GeomNode *node, const WorkingNodePath &node_path, 
                         EggGroupNode *egg_parent, bool has_decal);
  void convert_geom_tri(GeomTri *geom, const RenderState *net_state,
                        const LMatrix4f &net_mat, EggGroupNode *egg_parent);
  void recurse_nodes(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
                     bool has_decal);
  bool apply_node_properties(EggGroup *egg_group, PandaNode *node);

  EggTexture *get_egg_texture(Texture *tex);

  EggVertexPool *_vpool;
  EggTextureCollection _textures;
  EggMaterialCollection _materials;
};

#endif
