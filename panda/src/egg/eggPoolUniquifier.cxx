// Filename: eggPoolUniquifier.cxx
// Created by:  drose (09Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "eggPoolUniquifier.h"
#include "eggNode.h"
#include "eggTexture.h"
#include "eggMaterial.h"
#include "eggVertexPool.h"

#include <notify.h>

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
