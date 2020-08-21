/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPoolUniquifier.cxx
 * @author drose
 * @date 2000-11-09
 */

#include "eggPoolUniquifier.h"
#include "eggNode.h"
#include "eggTexture.h"
#include "eggMaterial.h"
#include "eggVertexPool.h"

#include "pnotify.h"

TypeHandle EggPoolUniquifier::_type_handle;


/**
 *
 */
EggPoolUniquifier::
EggPoolUniquifier() {
}

/**
 * Returns the category name into which the given node should be collected, or
 * the empty string if the node's name should be left alone.
 */
std::string EggPoolUniquifier::
get_category(EggNode *node) {
  if (node->is_of_type(EggTexture::get_class_type())) {
    return "tex";
  } else if (node->is_of_type(EggMaterial::get_class_type())) {
    return "mat";
  } else if (node->is_of_type(EggVertexPool::get_class_type())) {
    return "vpool";
  }

  return std::string();
}
