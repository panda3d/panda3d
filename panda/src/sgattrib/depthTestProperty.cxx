// Filename: depthTestProperty.cxx
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "depthTestProperty.h"

ostream &
operator << (ostream &out, DepthTestProperty::Mode mode) {
  switch (mode) {
  case DepthTestProperty::M_none:
    return out << "none";

  case DepthTestProperty::M_never:
    return out << "never";

  case DepthTestProperty::M_less:
    return out << "less";

  case DepthTestProperty::M_equal:
    return out << "equal";

  case DepthTestProperty::M_less_equal:
    return out << "less_equal";

  case DepthTestProperty::M_greater:
    return out << "greater";

  case DepthTestProperty::M_not_equal:
    return out << "not_equal";
 	
  case DepthTestProperty::M_greater_equal:
    return out << "greater_equal";
	
  case DepthTestProperty::M_always:
    return out << "always";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestProperty::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void DepthTestProperty::
output(ostream &out) const {
  out << _mode;
}
