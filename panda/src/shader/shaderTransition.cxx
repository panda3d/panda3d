// Filename: shaderTransition.cxx
// Created by:  mike (06Feb99)
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

#include "shaderTransition.h"

#include "indent.h"
#include "graphicsStateGuardian.h"
#include "textureTransition.h"
#include "attribTraverser.h"
#include "directRenderTraverser.h"
#include "renderRelation.h"

#include "config_shader.h"

TypeHandle ShaderTransition::_type_handle;
ShaderTransition::ShaderOrder* ShaderTransition::_shader_order = (ShaderTransition::ShaderOrder *)NULL;
ShaderTransition::ShaderBlend* ShaderTransition::_shader_always_blend = (ShaderTransition::ShaderBlend *)NULL;

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ShaderTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *ShaderTransition::
make_copy() const {
  return new ShaderTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::set_shader_order
//       Access: Public, Static
//  Description: Stores the order "priority" of a shader. (Note, there
//               is nothing to prevent you from adding a none shader
//               handle type, but it won't really do you any good).
//               This order value is used to make sure that multiple
//               shaders are ordered in a defined, hopefully
//               intelligent order.
////////////////////////////////////////////////////////////////////
void ShaderTransition::
set_shader_order(TypeHandle shader, int order) {
  if (_shader_order == (ShaderOrder *)NULL)
  {
    _shader_order = new ShaderOrder;
  }
  (*_shader_order)[shader] = order;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::set_shader_blend
//       Access: Public, Static
//  Description: Some shaders (i.e. spotlight and highlight) need to
//               blend with the scene no matter what.
////////////////////////////////////////////////////////////////////
void ShaderTransition::
set_shader_always_blend(TypeHandle shader) {
  if (_shader_always_blend == (ShaderBlend *)NULL)
  {
    _shader_always_blend = new ShaderBlend;
  }
  _shader_always_blend->insert(shader);
}


////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::clear
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ShaderTransition::
clear() {
  _shaders.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::is_empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool ShaderTransition::
is_empty() const {
  return _shaders.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::insert
//       Access: Public
//  Description: Inserts the shader into the list, based
//               on the set order of the list.  If there is no order set for
//               the shader, it is put at the end of the list.
////////////////////////////////////////////////////////////////////
void ShaderTransition::
insert(Shader *shader) {
  PT(Shader) sp(shader);
  Shaders::iterator si = _shaders.begin();
  int order = -1;

  if ((_shader_order == NULL) && (_overrides.find(sp) == _overrides.end()))
  {
    si = _shaders.end();
  }

  if (_overrides.find(sp) != _overrides.end())
  {
    order = _overrides[sp];
  }
  else if (_shader_order->find(sp->get_type()) != _shader_order->end())
  {
    order = (*_shader_order)[sp->get_type()];
  }

  if (order == -1)
  {
    si = _shaders.end();
  }

  while(si != _shaders.end())
  {
    if (_overrides.find((*si)) != _overrides.end())
    {
      if (order < _overrides[(*si)])
      {
        break;
      }
    }
    else if (order < (*_shader_order)[(*si)->get_type()])
    {
      break;
    }
    si++;
  }

  _shaders.insert(si, shader);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::set_shader
//       Access: Public
//  Description: Adds the shader to the transition, if it is not
//               already there.  Returns true if the shader was added,
//               false if it was there already.  If the shader *was*
//               already on the list, it is moved to the end of the
//               list.
////////////////////////////////////////////////////////////////////
bool ShaderTransition::
set_shader(Shader *shader, int override) {
  // We need to search for a PT(Shader), not a Shader* .
  PT(Shader) sp(shader);

  if (override != -1)
  {
    _overrides[sp] = override;
  }

  Shaders::iterator si = find(_shaders.begin(), _shaders.end(), sp);
  if (si != _shaders.end()) {
    // The shader was there already; move it to the end.
    return false;
  }
  insert(shader);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::clear_shader
//       Access: Public
//  Description: Removes the first occurrence of the indicated shader
//               from the list, and returns true if anything was
//               removed, false if there were no occurrences.
////////////////////////////////////////////////////////////////////
bool ShaderTransition::
clear_shader(Shader *shader) {
  // We need to search for a PT(Shader), not a Shader* .
  PT(Shader) sp(shader);
  Shaders::iterator si = find(_shaders.begin(), _shaders.end(), sp);
  if (si != _shaders.end()) {
    _shaders.erase(si);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::has_shader
//       Access: Public
//  Description: Returns true if the indicated shader appears on the
//               list, false otherwise.
////////////////////////////////////////////////////////////////////
bool ShaderTransition::
has_shader(Shader *shader) const {
  // We need to search for a PT(Shader), not a Shader* .
  PT(Shader) sp(shader);
  Shaders::const_iterator si = find(_shaders.begin(), _shaders.end(), sp);
  return (si != _shaders.end());
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ShaderTransition::const_iterator ShaderTransition::
begin() const {
  return _shaders.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ShaderTransition::const_iterator ShaderTransition::
end() const {
  return _shaders.end();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::must_blend
//       Access: Public
//  Description: Checks to see if one of the shaders on the shaderTransition
//               is a kind that requires blending
////////////////////////////////////////////////////////////////////
bool ShaderTransition::
must_blend()
{
  const_iterator si;
  for(si = begin(); si != end(); si++)
  {
    if (_shader_always_blend->find((*si)->get_type()) != _shader_always_blend->end())
    {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::sub_render
//       Access: Public, Virtual
//  Description: This is called by the RenderTraverser to tell the
//               shader to do its thing.
////////////////////////////////////////////////////////////////////
bool ShaderTransition::
sub_render(NodeRelation *arc, const AllAttributesWrapper &attrib,
           AllTransitionsWrapper &trans, RenderTraverser *trav) {
  Node *node = arc->get_child();
  GraphicsStateGuardian *gsg = trav->get_gsg();
  bool multipass_on = false;

  // No shaders; never mind.
  if (is_empty()) {
    return true;
  }

  AllAttributesWrapper node_attrib;
  node_attrib.apply_from(attrib, trans);

  multipass_on = must_blend() | is_textured(node, node_attrib) |
                 is_shaded(node) | (_shaders.size() > 1);

  const_iterator si;
  // In order to avoid strange nasty looking artifacts due to(?) the
  // depth buffer not being restored properly when we employ scratch
  // display regions.  Call pre-apply before the scene has been
  // rendered.  Any shader that uses a scratch region will do it's
  // thing and then when the scene is actually rendered, it will blow
  // those artifacts away.  Those shaders will store their work to be
  // actually applied later when we call apply.
  for (si = begin(); si != end(); ++si)
  {
    // We'll go ahead and tell each shader in turn if multipass is
    // needed or not now.
    (*si)->set_multipass(multipass_on);
    (*si)->pre_apply(node, node_attrib, AllTransitionsWrapper(), gsg);
  }

  if ( multipass_on )
  {
    // Multipass is needed at this point.  Render the node with its normal
    // attributes (including texturing).
    DirectRenderTraverser drt(gsg, RenderRelation::get_class_type());
    gsg->render_subgraph(&drt, node, node_attrib, AllTransitionsWrapper());
  }

  // Now apply each shader in turn.
  for (si = begin(); si != end(); ++si)
  {
    (*si)->apply(node, node_attrib, AllTransitionsWrapper(), gsg);
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::has_sub_render
//       Access: Public, Virtual
//  Description: Should be redefined to return true if the function
//               sub_render(), above, expects to be called during
//               traversal.
////////////////////////////////////////////////////////////////////
bool ShaderTransition::
has_sub_render() const {
  return true;
}



