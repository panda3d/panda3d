// Filename: directRenderTransition.cxx
// Created by:  drose (17Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "directRenderTransition.h"

#include <indent.h>

TypeHandle DirectRenderTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DirectRenderTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DirectRenderTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *DirectRenderTransition::
make_copy() const {
  return new DirectRenderTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DirectRenderTransition::has_sub_render
//       Access: Public, Virtual
//  Description: DirectRenderTransition doesn't actually have a
//               sub_render() function, but it might as well, because
//               it's treated as a special case.  We set this function
//               to return true so GraphReducer will behave correctly.
////////////////////////////////////////////////////////////////////
bool DirectRenderTransition::
has_sub_render() const {
  return true;
}
