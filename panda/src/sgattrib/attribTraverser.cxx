// Filename: attribTraverser.cxx
// Created by:  mike (16Feb99)
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

#include "attribTraverser.h"
#include "renderRelation.h"
#include <onOffTransition.h>
#include <onOffTransition.h>
#include <immediateTransition.h>

#include "textureTransition.h"
#include "textureTransition.h"

#include <geomNode.h>
#include <dftraverser.h>
#include <allTransitionsWrapper.h>
#include <typedObject.h>

////////////////////////////////////////////////////////////////////
//     Function: AttribTraverser::constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AttribTraverser::
AttribTraverser()
     : _has_attrib(false), _has_transition(false)
{
  _attrib_type = TypeHandle::none();
  _transition_type = TypeHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: AttribTraverser::reached_node
//       Access: Public
//  Description: Called for each node of the scene graph
////////////////////////////////////////////////////////////////////
bool AttribTraverser::
reached_node(Node *node, NodeTransitionWrapper &state, NullLevelState &) {

  //Short circuit if we aren't looking for an attribute
  if (_attrib_type == TypeHandle::none())
  {
    return true;
  }

  if (node->is_of_type(GeomNode::get_class_type()))
  {
    NodeTransition *attrib = state.get_trans();
    if (attrib != (NodeTransition *)NULL)
    {
      nassertr(attrib->is_of_type(_attrib_type), false);

      if (attrib->is_of_type(OnOffTransition::get_class_type()))
      {
        if(DCAST(OnOffTransition, attrib)->is_on())
        {
          _has_attrib = true;
        }
      }
      else
      {
        _has_attrib = true;
      }

    }
  }

  // Prune the search if we've discovered that we have the attribute
  // being searched for anywhere.
  return !_has_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AttribTraverser::forward_arc
//       Access: Public
//  Description: Called for each forward arc of the scene graph
////////////////////////////////////////////////////////////////////
bool AttribTraverser::
forward_arc(NodeRelation *, TransitionWrapper &trans,
            NodeTransitionWrapper &, NodeTransitionWrapper &,
            NullLevelState &)
{
  //Short circuit if we aren't looking for a transition
  if (_transition_type == TypeHandle::none())
  {
    return true;
  }

  NodeTransition *transition = trans.get_trans();

  if (transition != (NodeTransition *)NULL)
  {
    nassertr(transition->is_of_type(_transition_type), false);
    _has_transition = true;
  }
  // Prune the search if we've discovered that we have the transition
  // being searched for anywhere.
  return !_has_transition;
}
////////////////////////////////////////////////////////////////////
//     Function: AttribTraverser::set_attrib_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void AttribTraverser::
set_attrib_type(TypeHandle type)
{
  _attrib_type = type;
}

////////////////////////////////////////////////////////////////////
//     Function: AttribTraverser::set_transition_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void AttribTraverser::
set_transition_type(TypeHandle type)
{
  _transition_type = type;
  nassertv(_transition_type.is_derived_from(ImmediateTransition::get_class_type()));
}

////////////////////////////////////////////////////////////////////
//     Function: is_textured
//  Description: Recursively checks the tree of nodes from root to
//               see if any geometry is textured
////////////////////////////////////////////////////////////////////
bool
is_textured(Node* root) {
  AttribTraverser trav;

  trav.set_attrib_type(TextureTransition::get_class_type());

  df_traverse(root, trav,
              NodeTransitionWrapper(TextureTransition::get_class_type()),
              NullLevelState(), RenderRelation::get_class_type());

  return trav._has_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: is_textured
//  Description: Recursively checks the tree of nodes from root to
//               see if any geometry is textured
////////////////////////////////////////////////////////////////////
bool
is_textured(Node* root, const AllTransitionsWrapper &init_state)
{
  AttribTraverser trav;

  trav.set_attrib_type(TextureTransition::get_class_type());

  NodeTransitionWrapper state(TextureTransition::get_class_type());
  state.set_trans(init_state.get_transition(TextureTransition::get_class_type()));

  df_traverse(root, trav, state, NullLevelState(), RenderRelation::get_class_type());

  return trav._has_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: is_shaded
//  Description: Recursively checks the tree of nodes from root to
//               see if any geometry is shaded
////////////////////////////////////////////////////////////////////
bool
is_shaded(Node* root) {
  AttribTraverser trav;

  //We use the TypeRegistry here because attribTraverser is defined in
  //the package sgattrib.  And this package knows nothing about
  //Shaders.  So we can just include shaderTransition.h, as it may not
  //have been compiled yet.  So use this workaround to get the type
  //handle of a ShaderTransition
  trav.set_transition_type(TypeRegistry::ptr()->find_type("ShaderTransition"));

  df_traverse(root, trav,
              NodeTransitionWrapper(TypeRegistry::ptr()->find_type("ShaderTransition")),
              NullLevelState(), RenderRelation::get_class_type());

  return trav._has_attrib;
}





