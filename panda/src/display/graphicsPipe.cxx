// Filename: graphicsPipe.cxx
// Created by:  mike (09Jan97)
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

#include "graphicsPipe.h"
#include "config_display.h"
#include "mutexHolder.h"

TypeHandle GraphicsPipe::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsPipe::
GraphicsPipe() {
  // Initially, we assume the GraphicsPipe is valid.  A derived class
  // should set this to false if it determines otherwise.
  _is_valid = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Copy Constructor
//       Access: Private
//  Description: Don't try to copy GraphicsPipes.
////////////////////////////////////////////////////////////////////
GraphicsPipe::
GraphicsPipe(const GraphicsPipe &) {
  _is_valid = false;
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Copy Assignment Operator
//       Access: Private
//  Description: Don't try to copy GraphicsPipes.
////////////////////////////////////////////////////////////////////
void GraphicsPipe::
operator = (const GraphicsPipe &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsPipe::
~GraphicsPipe() {
  // On destruction, we need to clean up our references to all of the
  // windows.
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    GraphicsWindow *window = (*wi);
    window->_pipe = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_num_windows
//       Access: Published
//  Description: Returns the number of windows that have been created
//               on this pipe.
////////////////////////////////////////////////////////////////////
int GraphicsPipe::
get_num_windows() const {
  int result;
  {
    MutexHolder holder(_lock);
    result = _windows.size();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_window
//       Access: Published
//  Description: Returns the nth window that has been created
//               on this pipe.  It is possible for this to return NULL
//               due to a window being removed in another thread.
////////////////////////////////////////////////////////////////////
PT(GraphicsWindow) GraphicsPipe::
get_window(int n) const {
  PT(GraphicsWindow) result;
  {
    MutexHolder holder(_lock);
    if (n >= 0 && n < (int)_windows.size()) {
      result = _windows[n];
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_num_hw_channels
//       Access: Public, Virtual
//  Description: Returns the number of hardware channels available for
//               pipes of this type.  See get_hw_channel().
////////////////////////////////////////////////////////////////////
int GraphicsPipe::
get_num_hw_channels() {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_hw_channel
//       Access: Public, Virtual
//  Description: Creates and returns an accessor to the
//               HardwareChannel at the given index number, which must
//               be in the range 0 <= index < get_num_hw_channels().
//               This function will return NULL if the index number is
//               out of range or the hardware channel at that index is
//               unavailable.
//
//               Most kinds of GraphicsPipes do not have any special
//               hardware channels available, and this function will
//               always return NULL.
////////////////////////////////////////////////////////////////////
HardwareChannel *GraphicsPipe::
get_hw_channel(GraphicsWindow *, int) {
  return (HardwareChannel*)0L;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::add_window
//       Access: Protected
//  Description: This is intended to be called by the derived
//               make_window() function to add the newly-created
//               window to the list of windows owned by this pipe.
////////////////////////////////////////////////////////////////////
void GraphicsPipe::
add_window(GraphicsWindow *window) {
  nassertv(window->_pipe == this);
  MutexHolder holder(_lock);
  _windows.push_back(window);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::remove_window
//       Access: Protected
//  Description: This is intended to be called by the GraphicsEngine
//               when a window is to be removed.  It returns true if
//               the window is removed, or false if it was not found.
////////////////////////////////////////////////////////////////////
bool GraphicsPipe::
remove_window(GraphicsWindow *window) {
  bool removed = false;
  {
    MutexHolder holder(_lock);
    PT(GraphicsWindow) ptwin = window;
    window->_pipe = NULL;
    Windows::iterator wi = find(_windows.begin(), _windows.end(), ptwin);
    if (wi != _windows.end()) {
      _windows.erase(wi);
      removed = true;
    }
  }

  return removed;
}
