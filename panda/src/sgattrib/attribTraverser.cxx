// Filename: attribTraverser.cxx
// Created by:  mike (16Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "attribTraverser.h"
#include "renderRelation.h"
#include <onOffAttribute.h>
#include <onOffTransition.h>
#include <immediateTransition.h>

#include "textureTransition.h"
#include "textureAttribute.h"

#include <geomNode.h>
#include <dftraverser.h>
#include <allAttributesWrapper.h>
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
reached_node(Node *node, NodeAttributeWrapper &state, NullLevelState &) {

  //Short circuit if we aren't looking for an attribute
  if (_attrib_type == TypeHandle::none())
  {
    return true;
  }

  if (node->is_of_type(GeomNode::get_class_type())) 
  {
    NodeAttribute *attrib = state.get_attrib();
    if (attrib != (NodeAttribute *)NULL) 
    {
      nassertr(attrib->is_of_type(_attrib_type), false);

      if (attrib->is_of_type(OnOffAttribute::get_class_type()))
      {
	if(DCAST(OnOffAttribute, attrib)->is_on())
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
	    NodeAttributeWrapper &, NodeAttributeWrapper &,
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
//		 see if any geometry is textured 
////////////////////////////////////////////////////////////////////
bool
is_textured(Node* root) {
  AttribTraverser trav;

  trav.set_attrib_type(TextureAttribute::get_class_type());

  df_traverse(root, trav, 
	      NodeAttributeWrapper(TextureTransition::get_class_type()), 
	      NullLevelState(), RenderRelation::get_class_type());

  return trav._has_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: is_textured 
//  Description: Recursively checks the tree of nodes from root to
//		 see if any geometry is textured 
////////////////////////////////////////////////////////////////////
bool
is_textured(Node* root, const AllAttributesWrapper &init_state) 
{
  AttribTraverser trav;

  trav.set_attrib_type(TextureAttribute::get_class_type());

  NodeAttributeWrapper state(TextureTransition::get_class_type());
  state.set_attrib(init_state.get_attribute(TextureTransition::get_class_type()));

  df_traverse(root, trav, state, NullLevelState(), RenderRelation::get_class_type());

  return trav._has_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: is_shaded 
//  Description: Recursively checks the tree of nodes from root to
//		 see if any geometry is shaded 
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
	      NodeAttributeWrapper(TypeRegistry::ptr()->find_type("ShaderTransition")), 
	      NullLevelState(), RenderRelation::get_class_type());

  return trav._has_attrib;
}





