// Filename: assimpLoader.h
// Created by:  rdb (29Mar11)
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

#ifndef ASSIMPLOADER_H
#define ASSIMPLOADER_H

#include "config_assimp.h"
#include "filename.h"
#include "modelRoot.h"
#include "texture.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"

////////////////////////////////////////////////////////////////////
//       Class : AssimpLoader
// Description : Class that interfaces with Assimp and builds Panda
//               nodes to represent the Assimp structures.
//               The loader should be reusable.
////////////////////////////////////////////////////////////////////
class AssimpLoader : public TypedReferenceCount {
public:
  AssimpLoader();
  virtual ~AssimpLoader();

  void get_extensions(string &ext) const;

  bool read(const Filename &filename);
  void build_graph();

public:
  bool _error;
  PT(ModelRoot) _root;
  Filename _filename;
  Mutex _lock;

private:
  Assimp::Importer _importer;
  const aiScene *_scene;

  // These arrays are temporarily used during the build_graph run.
  PT(Texture) *_textures;
  CPT(RenderState) *_mat_states;
  PT(Geom) *_geoms;
  unsigned int *_geom_matindices;

  void load_texture(size_t index);
  void load_texture_stage(const aiMaterial &mat, const aiTextureType &ttype, CPT(TextureAttrib) &tattr);
  void load_material(size_t index);
  void load_mesh(size_t index);
  void load_node(const aiNode &node, PandaNode *parent);
  void load_light(const aiLight &light);
};

#include "assimpLoader.I"

#endif
