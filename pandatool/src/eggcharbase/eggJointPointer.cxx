// Filename: eggJointPointer.cxx
// Created by:  drose (26Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggJointPointer.h"


TypeHandle EggJointPointer::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::add_frame
//       Access: Public, Virtual
//  Description: Appends a new frame onto the end of the data, if
//               possible; returns true if not possible, or false
//               otherwise (e.g. for a static joint).
////////////////////////////////////////////////////////////////////
bool EggJointPointer::
add_frame(const LMatrix4d &) {
  return false;
}
