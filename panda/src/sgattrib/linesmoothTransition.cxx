// Filename: linesmoothTransition.cxx
// Created by:  mike (08Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "linesmoothTransition.h"
#include "linesmoothAttribute.h"

TypeHandle LinesmoothTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LinesmoothTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated LinesmoothTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *LinesmoothTransition::
make_copy() const {
  return new LinesmoothTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LinesmoothTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated LinesmoothAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *LinesmoothTransition::
make_attrib() const {
  return new LinesmoothAttribute;
}
