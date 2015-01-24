// Filename: nodeCullCallbackData.cxx
// Created by:  drose (13Mar09)
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

#include "nodeCullCallbackData.h"
#include "callbackNode.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "cullableObject.h"
#include "cullHandler.h"

TypeHandle NodeCullCallbackData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NodeCullCallbackData::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void NodeCullCallbackData::
output(ostream &out) const {
  out << get_type() << "(" << (void *)_trav << ", " << (void *)&_data << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: NodeCullCallbackData::upcall
//       Access: Published, Virtual
//  Description: You should make this call during the callback if you
//               want to continue the normal rendering function that
//               would have been done in the absence of a callback.
//
//               Specifically, this method will add this node to the
//               draw queue, and continue the cull traversal for all
//               the nodes below.  If you omit this call, this node
//               and its children will be pruned from the render
//               result.
////////////////////////////////////////////////////////////////////
void NodeCullCallbackData::
upcall() {
  PandaNode *node = _data.node();
  if (node->is_of_type(CallbackNode::get_class_type())) {
    CallbackNode *cbnode = DCAST(CallbackNode, _data.node());

    // OK, render this node.  Rendering a CallbackNode means creating
    // a CullableObject for the draw_callback, if any.  We don't need
    // to pass any Geoms, however.
    CallbackObject *cbobj = cbnode->get_draw_callback();
    if (cbobj != (CallbackObject *)NULL) {
      CullableObject *object =
        new CullableObject(NULL, _data._state,
                           _data.get_internal_transform(_trav));
      object->set_draw_callback(cbobj);
      _trav->get_cull_handler()->record_object(object, _trav);
    }
  }

  // Now traverse below.
  _trav->traverse_below(_data);
}
