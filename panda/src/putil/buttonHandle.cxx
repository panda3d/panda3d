// Filename: buttonHandle.cxx
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "buttonHandle.h"
#include "buttonRegistry.h"

// This is initialized to zero by static initialization.
ButtonHandle ButtonHandle::_none;


////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::get_name
//       Access: Public
//  Description: Returns the name of the button.
////////////////////////////////////////////////////////////////////
string ButtonHandle::
get_name() const {
  if ((*this) == ButtonHandle::none()) {
    return "none";
  } else {
    return ButtonRegistry::ptr()->get_name(*this);
  }
}
