// Filename: graphicsThreadingModel.I
// Created by:  drose (27Jan03)
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
//     Function: GraphicsThreadingModel::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GraphicsThreadingModel::
GraphicsThreadingModel(const GraphicsThreadingModel &copy) :
  _cull_name(copy._cull_name),
  _draw_name(copy._draw_name),
  _cull_sorting(copy._cull_sorting)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsThreadingModel::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GraphicsThreadingModel::
operator = (const GraphicsThreadingModel &copy) {
  _cull_name = copy._cull_name;
  _draw_name = copy._draw_name;
  _cull_sorting = copy._cull_sorting;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsThreadingModel::get_cull_name
//       Access: Published
//  Description: Returns the name of the thread that will handle
//               culling in this model.
////////////////////////////////////////////////////////////////////
INLINE const string &GraphicsThreadingModel::
get_cull_name() const {
  return _cull_name;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsThreadingModel::get_draw_name
//       Access: Published
//  Description: Returns the name of the thread that will handle
//               sending the actual graphics primitives to the
//               graphics API in this model.
////////////////////////////////////////////////////////////////////
INLINE const string &GraphicsThreadingModel::
get_draw_name() const {
  return _draw_name;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsThreadingModel::get_cull_sorting
//       Access: Published
//  Description: Returns true if the model involves a separate cull
//               pass, or false if culling happens implicitly, at the
//               same time as draw.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsThreadingModel::
get_cull_sorting() const {
  return _cull_sorting;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsThreadingModel::is_single_threaded
//       Access: Published
//  Description: Returns true if the threading model is a
//               single-threaded model, or false if it involves
//               threads.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsThreadingModel::
is_single_threaded() const {
  return _cull_name.empty() && _draw_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsThreadingModel::is_default
//       Access: Published
//  Description: Returns true if the threading model is the default,
//               cull-then-draw single-threaded model, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsThreadingModel::
is_default() const {
  return is_single_threaded() && _cull_sorting;
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsThreadingModel::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GraphicsThreadingModel::
output(ostream &out) const {
  out << get_model();
}

INLINE ostream &
operator << (ostream &out, const GraphicsThreadingModel &threading_model) {
  threading_model.output(out);
  return out;
}
