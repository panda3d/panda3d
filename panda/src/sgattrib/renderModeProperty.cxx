// Filename: renderModeProperty.cxx
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "renderModeProperty.h"

ostream &
operator << (ostream &out, RenderModeProperty::Mode mode) {
  switch (mode) {
  case RenderModeProperty::M_filled:
    return out << "filled";

  case RenderModeProperty::M_wireframe:
    return out << "wireframe";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeProperty::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderModeProperty::
output(ostream &out) const {
  out << _mode << ":" << _line_width;
}
