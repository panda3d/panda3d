// Filename: switchNodeOne.cxx
// Created by:  drose (15May01)
// 
////////////////////////////////////////////////////////////////////

#include "switchNodeOne.h"

TypeHandle SwitchNodeOne::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: SwitchNodeOne::is_child_visible
//       Access: Public, Virtual
//  Description: Returns true if the indicated child is one that
//               should be considered visible (rendered), false if it
//               should be suppressed.
////////////////////////////////////////////////////////////////////
bool SwitchNodeOne::
is_child_visible(TypeHandle, int index) {
  return (index == _selected_child_index);
}
