// Filename: graphicsPipeSelection.I
// Created by:  drose (15Aug02)
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
