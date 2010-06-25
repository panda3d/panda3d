// Filename: graphicsWindowProcCallbackData.I
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GraphicsWindowProcCallbackData::
GraphicsWindowProcCallbackData(GraphicsWindow* graphicsWindow){
  _graphicsWindow = graphicsWindow;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::get_hwnd
//       Access: Published
//  Description: Returns the GraphicsWindow that generated this event.
////////////////////////////////////////////////////////////////////
INLINE GraphicsWindow* GraphicsWindowProcCallbackData::
get_graphics_window() const {
  return _graphicsWindow;
}

#ifdef WIN32

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::get_hwnd
//       Access: Published
//  Description: Returns the Windows proc hwnd parameter.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindowProcCallbackData::
get_hwnd() const {
  return _hwnd;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::get_msg
//       Access: Published
//  Description: Returns the Windows proc msg parameter.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindowProcCallbackData::
get_msg() const {
  return _msg;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::get_wparam
//       Access: Published
//  Description: Returns the Windows proc wparam parameter.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindowProcCallbackData::
get_wparam() const {
  return _wparam;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::get_lparam
//       Access: Published
//  Description: Returns the Windows proc lparam parameter.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindowProcCallbackData::
get_lparam() const {
  return _lparam;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::set_hwnd
//       Access: Published
//  Description: Sets the Windows proc hwnd parameter.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowProcCallbackData::
set_hwnd(int hwnd) {
  _hwnd = hwnd;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::set_msg
//       Access: Published
//  Description: Sets the Windows proc msg parameter.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowProcCallbackData::
set_msg(int msg) {
  _msg = msg;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::set_wparam
//       Access: Published
//  Description: Sets the Windows proc wparam parameter.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowProcCallbackData::
set_wparam(int wparam) {
  _wparam = wparam;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowProcCallbackData::set_lparam
//       Access: Published
//  Description: Sets the Windows proc lparam parameter.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowProcCallbackData::
set_lparam(int lparam) {
  _lparam = lparam;
}

#endif
