// Filename: onAttribute.cxx
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "onAttribute.h"
#include "onTransition.h"

#include <indent.h>

TypeHandle OnAttribute::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: OnAttribute::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void OnAttribute::
output(ostream &out) const {
  output_value(out);
}

////////////////////////////////////////////////////////////////////
//     Function: OnAttribute::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void OnAttribute::
write(ostream &out, int indent_level) const {
  write_value(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: OnAttribute::internal_compare_to
//       Access: Protected, Virtual
//  Description: Returns a number < 0 if this attribute sorts before
//               the other attribute, > 0 if it sorts after, 0 if
//               they are equivalent (except for priority).
////////////////////////////////////////////////////////////////////
int OnAttribute::
internal_compare_to(const NodeAttribute *other) const {
  const OnAttribute *ot;
  DCAST_INTO_R(ot, other, false);

  return compare_values(ot);
}
