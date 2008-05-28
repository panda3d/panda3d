// Filename: eggUtilities.cxx
// Created by:  drose (28Jan99)
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

#include "eggUtilities.h"
#include "eggPrimitive.h"
#include "eggGroupNode.h"
#include "pt_EggTexture.h"
#include "dcast.h"


////////////////////////////////////////////////////////////////////
//     Function: get_textures_by_filename
//  Description: Extracts from the egg subgraph beginning at the
//               indicated node a set of all the texture objects
//               referenced, grouped together by filename.  Texture
//               objects that share a common filename (but possibly
//               differ in other properties) are returned together in
//               the same element of the map.
////////////////////////////////////////////////////////////////////
void
get_textures_by_filename(const EggNode *node, EggTextureFilenames &result) {
  if (node->is_of_type(EggPrimitive::get_class_type())) {
    const EggPrimitive *prim = DCAST(EggPrimitive, node);

    int num_textures = prim->get_num_textures();
    for (int i = 0; i < num_textures; i++) {
      PT_EggTexture tex = prim->get_texture(i);
      result[tex->get_filename()].insert(tex);
    }

  } else if (node->is_of_type(EggGroupNode::get_class_type())) {
    const EggGroupNode *group = DCAST(EggGroupNode, node);

    EggGroupNode::const_iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      get_textures_by_filename(*ci, result);
    }
  }
}

