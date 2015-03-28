// Filename: eggSaver.h
// Created by:  drose (19Dec12)
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

#ifndef EGGSAVER_H
#define EGGSAVER_H

#include "pandabase.h"

#include "luse.h"
#include "eggData.h"
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
class AnimBundleNode;
class AnimGroup;
class Character;
class PartGroup;
class CollisionNode;
class GeomNode;
class GeomVertexData;
class GeomPrimitive;
class PandaNode;
class RenderState;
class Material;
class Texture;
class CharacterJoint;
class EggVertex;

////////////////////////////////////////////////////////////////////
//       Class : EggSaver
// Description : Converts the scene graph beginning at the indicated
//               node into an EggData structure, for writing to an egg
//               file.  The conversion is not necessarily complete
//               (some Panda or egg constructs are not fully supported
//               by this class).
////////////////////////////////////////////////////////////////////
class EggSaver {
PUBLISHED:
  EggSaver(EggData *data = NULL);

  void add_node(PandaNode *node);
  INLINE EggData *get_egg_data() const;

private:
  typedef pmap<const CharacterJoint*, pvector<pair<EggVertex*,PN_stdfloat> > > CharacterJointMap;

  void convert_node(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
                    bool has_decal);
  void convert_lod_node(LODNode *node, const WorkingNodePath &node_path,
                        EggGroupNode *egg_parent, bool has_decal);
  void convert_sequence_node(SequenceNode *node, const WorkingNodePath &node_path,
                        EggGroupNode *egg_parent, bool has_decal);
  void convert_switch_node(SwitchNode *node, const WorkingNodePath &node_path,
                        EggGroupNode *egg_parent, bool has_decal);
  EggGroupNode *convert_animGroup_node(AnimGroup *animGroup, double fps );
  void convert_anim_node(AnimBundleNode *node, const WorkingNodePath &node_path,
                        EggGroupNode *egg_parent, bool has_decal);
  void convert_character_node(Character *node, const WorkingNodePath &node_path,
                        EggGroupNode *egg_parent, bool has_decal);
  void convert_character_bundle(PartGroup *bundleNode, EggGroupNode *egg_parent, CharacterJointMap *jointMap);
  void convert_collision_node(CollisionNode *node, const WorkingNodePath &node_path,
                        EggGroupNode *egg_parent, bool has_decal);
  void convert_geom_node(GeomNode *node, const WorkingNodePath &node_path, 
                         EggGroupNode *egg_parent, bool has_decal, CharacterJointMap *jointMap=NULL);
  void convert_primitive(const GeomVertexData *vertex_data,
                         const GeomPrimitive *primitive, 
                         const RenderState *net_state, 
                         const LMatrix4 &net_mat, EggGroupNode *egg_parent,
                         CharacterJointMap *jointMap);

  void recurse_nodes(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
                     bool has_decal);
  bool apply_node_properties(EggGroup *egg_group, PandaNode *node, bool allow_backstage = true);
  bool apply_tags(EggGroup *egg_group, PandaNode *node);
  bool apply_tag(EggGroup *egg_group, PandaNode *node, const string &tag);

  EggMaterial *get_egg_material(Material *tex);
  EggTexture *get_egg_texture(Texture *tex);

  static EggPrimitive *make_egg_polygon();
  static EggPrimitive *make_egg_patch();
  static EggPrimitive *make_egg_point();
  static EggPrimitive *make_egg_line();

  PT(EggData) _data;

  PT(EggVertexPool) _vpool;
  EggMaterialCollection _materials;
  EggTextureCollection _textures;
};

#include "eggSaver.I"

#endif
