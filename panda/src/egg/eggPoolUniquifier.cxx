// Filename: eggPoolUniquifier.cxx
// Created by:  drose (09Nov00)
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

#include "eggPoolUniquifier.h"
#include "eggNode.h"
#include "eggTexture.h"
#include "eggMaterial.h"
#include "eggVertexPool.h"

#include "notify.h"

TypeHandle EggPoolUniquifier::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggPoolUniquifier::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggPoolUniquifier::
EggPoolUniquifier() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoolUniquifier::get_category
//       Access: Public
//  Description: Returns the category name into which the given node
//               should be collected, or the empty string if the
//               node's name should be left alone.
////////////////////////////////////////////////////////////////////
string EggPoolUniquifier::
get_category(EggNode *node) {
  if (node->is_of_type(EggTexture::get_class_type())) {
    return "tex";
  } else if (node->is_of_type(EggMaterial::get_class_type())) {
    return "mat";
  } else if (node->is_of_type(EggVertexPool::get_class_type())) {
    return "vpool";
  }

  return string();
}
