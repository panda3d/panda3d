// Filename: graphicsPipeSelection.I
// Created by:  drose (15Aug02)
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


////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::resolve_modules
//       Access: Published
//  Description: Conditionally calls do_resolve_modules(), if it has
//               not been called before.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsPipeSelection::
resolve_modules() const {
  if (!_resolved_modules) {
    ((GraphicsPipeSelection *)this)->do_resolve_modules();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::get_global_ptr
//       Access: Published, Static
//  Description: Returns a pointer to the one global
//               GraphicsPipeSelection object.
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipeSelection *GraphicsPipeSelection::
get_global_ptr() {
  if (_global_ptr == (GraphicsPipeSelection *)NULL) {
    _global_ptr = new GraphicsPipeSelection;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::PipeType::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipeSelection::PipeType::
PipeType(TypeHandle type, PipeConstructorFunc *constructor) :
  _type(type),
  _constructor(constructor)
{
}
