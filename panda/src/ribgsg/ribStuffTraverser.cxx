// Filename: ribStuffTraverser.cxx
// Created by:  drose (16Feb99)
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

#include "ribStuffTraverser.h"
#include "ribGraphicsStateGuardian.h"

#include <textureTransition.h>
#include <textureAttribute.h>
#include <lightTransition.h>
#include <lightAttribute.h>
#include <geomNode.h>

////////////////////////////////////////////////////////////////////
//     Function: RibStuffTraverser::reached_node
//       Access: Public
//  Description: Called for each node of the scene graph; this does
//               the work of identifying new things and registering
//               them in the RIB file.
////////////////////////////////////////////////////////////////////
bool RibStuffTraverser::
reached_node(Node *node, AllAttributesWrapper &state, NullLevelState &) {
  if (node->is_of_type(GeomNode::get_class_type())) {
    const TextureAttribute *tex_attrib;
    if (!get_attribute_into(tex_attrib, state,
                            TextureTransition::get_class_type())) {
      if (tex_attrib->is_on()) {
        _gsg->define_texture(tex_attrib->get_texture());
      }
    }

    const LightAttribute *light_attrib;
    if (!get_attribute_into(light_attrib, state,
                            LightTransition::get_class_type())) {
      LightAttribute::const_iterator li;
      for (li = light_attrib->begin(); li != light_attrib->end(); ++li) {
        _gsg->define_light(*li);
      }
    }
  }
  return true;
}
