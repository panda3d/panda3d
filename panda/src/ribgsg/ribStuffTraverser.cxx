// Filename: ribStuffTraverser.cxx
// Created by:  drose (16Feb99)
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

#include "ribStuffTraverser.h"
#include "ribGraphicsStateGuardian.h"

#include "textureTransition.h"
#include "lightTransition.h"
#include "geomNode.h"

////////////////////////////////////////////////////////////////////
//     Function: RibStuffTraverser::reached_node
//       Access: Public
//  Description: Called for each node of the scene graph; this does
//               the work of identifying new things and registering
//               them in the RIB file.
////////////////////////////////////////////////////////////////////
bool RibStuffTraverser::
reached_node(Node *node, AllTransitionsWrapper &state, NullLevelState &) {
  if (node->is_of_type(GeomNode::get_class_type())) {
    const TextureTransition *tex_attrib;
    if (!get_transition_into(tex_attrib, state)) {
      if (tex_attrib->is_on()) {
        _gsg->define_texture(tex_attrib->get_texture());
      }
    }

    const LightTransition *light_attrib;
    if (!get_transition_into(light_attrib, state)) {
      LightTransition::const_iterator li;
      for (li = light_attrib->begin(); li != light_attrib->end(); ++li) {
        _gsg->define_light(*li);
      }
    }
  }
  return true;
}
