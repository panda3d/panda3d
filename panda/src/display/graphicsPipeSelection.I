// Filename: graphicsPipeSelection.I
// Created by:  drose (15Aug02)
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::get_num_aux_modules
//       Access: Published
//  Description: Returns the number of display modules that are still
//               to be loaded.  If this is nonzero, then calling
//               load_aux_modules() will likely increase the number of
//               GraphicsPipes available.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsPipeSelection::
get_num_aux_modules() const {
  return _display_modules.size();
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
//     Function: GraphicsPipeSelection::load_default_module
//       Access: Private
//  Description: Conditionally calls do_load_default_module(), if it has
//               not been called before.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsPipeSelection::
load_default_module() const {
  if (!_default_module_loaded) {
    ((GraphicsPipeSelection *)this)->do_load_default_module();
  }
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
