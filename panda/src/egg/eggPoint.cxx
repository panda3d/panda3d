// Filename: eggPoint.cxx
// Created by:  drose (15Dec99)
// 
////////////////////////////////////////////////////////////////////

#include "eggPoint.h"

#include <indent.h>

#include <algorithm>

TypeHandle EggPoint::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggPoint::cleanup
//       Access: Public, Virtual
//  Description: Cleans up modeling errors in whatever context this
//               makes sense.  For instance, for a polygon, this calls
//               remove_doubled_verts(true).  For a point, it calls
//               remove_nonunique_verts().
////////////////////////////////////////////////////////////////////
void EggPoint::
cleanup() {
  remove_nonunique_verts();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::write
//       Access: Public, Virtual
//  Description: Writes the point to the indicated output stream in
//               Egg format.
////////////////////////////////////////////////////////////////////
void EggPoint::
write(ostream &out, int indent_level) const {
  write_header(out, indent_level, "<PointLight>");
  write_body(out, indent_level+2);
  indent(out, indent_level) << "}\n";
}
