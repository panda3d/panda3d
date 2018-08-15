/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsWindowProcCallbackData.cxx
 * @author Walt Destler
 * @date 2010-06
 */

#include "graphicsWindowProcCallbackData.h"
#include "graphicsWindow.h"

TypeHandle GraphicsWindowProcCallbackData::_type_handle;

/**
 *
 */
void GraphicsWindowProcCallbackData::
output(std::ostream &out) const {
#ifdef WIN32
  out << get_type() << "(" << (void*)_graphicsWindow << ", " << _hwnd << ", "
      << _msg << ", " << _wparam << ", " << _lparam << ")";
#else
  out << get_type() << "()";
#endif
}
/**
 * Returns whether the event is a touch event.
 *
 */
bool GraphicsWindowProcCallbackData::
is_touch_event(){
  return _graphicsWindow->is_touch_event(this);
}

/**
 * Returns the current number of touches on the window.
 *
 */
int GraphicsWindowProcCallbackData::
get_num_touches(){
  return _graphicsWindow->get_num_touches();
}

/**
 * Returns the TouchInfo object describing the specified touch.
 *
 */
TouchInfo GraphicsWindowProcCallbackData::
get_touch_info(int index){
  return _graphicsWindow->get_touch_info(index);
}
