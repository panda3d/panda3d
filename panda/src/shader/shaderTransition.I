// Filename: shaderTransition.I
// Created by:  drose (24Mar00)
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

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: ShaderTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderTransition::
ShaderTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: set_shader
//  Description: An external function, handy for setting shaders
//               directly onto arcs.  This will add the indicated
//               shader to an existing ShaderTransition, or create a
//               new ShaderTransition if necessary.
////////////////////////////////////////////////////////////////////
INLINE bool
set_shader(NodeRelation *arc, Shader *shader) {
  ShaderTransition *st;
  if (get_transition_into(st, arc)) {
    return st->set_shader(shader);
  }
  st = new ShaderTransition;
  st->set_shader(shader);
  arc->set_transition(st);
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: clear_shader
//  Description: An external function, handy for setting shaders
//               directly onto arcs.  This will remove the indicated
//               shader from the existing ShaderTransition, if there
//               is one.
////////////////////////////////////////////////////////////////////
INLINE bool
clear_shader(NodeRelation *arc, Shader *shader) {
  ShaderTransition *st;
  if (get_transition_into(st, arc)) {
    return st->clear_shader(shader);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: has_shader
//  Description: An external function, handy for setting shaders
//               directly onto arcs.  This returns true if there
//               exists a ShaderTransition and it contains the
//               indicated shader, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool
has_shader(const NodeRelation *arc, Shader *shader) {
  const ShaderTransition *st;
  if (get_transition_into(st, arc)) {
    return st->has_shader(shader);
  }
  return false;
}
