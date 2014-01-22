// Filename: graphicsWindowProcCallbackData.cxx
// Created by:  Walt Destler (June 2010)
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

#include "graphicsWindowProcCallbackData.h"
#include "graphicsWindow.h"

TypeHandle GraphicsWindowProcCallbackData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindowProcCallbackData::
output(ostream &out) const {
#ifdef WIN32
  out << get_type() << "(" << (void*)_graphicsWindow << ", " << _hwnd << ", "
      << _msg << ", " << _wparam << ", " << _lparam << ")";
#else
  out << get_type() << "()";
#endif
}
////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::is_touch_event
//       Access: Public, Virtual
//  Description: Returns whether the event is a touch event.
//               
////////////////////////////////////////////////////////////////////
bool GraphicsWindowProcCallbackData::
is_touch_event(){
  return _graphicsWindow->is_touch_event(this);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::get_num_touches
//       Access: Public, Virtual
//  Description: Returns the current number of touches on the window.
//               
////////////////////////////////////////////////////////////////////
int GraphicsWindowProcCallbackData::
get_num_touches(){
  return _graphicsWindow->get_num_touches();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::get_touch_info
//       Access: Public, Virtual
//  Description: Returns the TouchInfo object describing the specified touch.
//               
////////////////////////////////////////////////////////////////////
TouchInfo GraphicsWindowProcCallbackData::
get_touch_info(int index){
  return _graphicsWindow->get_touch_info(index);
}
