// Filename: eggPolygon.cxx
// Created by:  drose (16Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "eggPolygon.h"

#include <indent.h>

#include <algorithm>

TypeHandle EggPolygon::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggPolygon::cleanup
//       Access: Public, Virtual
//  Description: Cleans up modeling errors in whatever context this
//               makes sense.  For instance, for a polygon, this calls
//               remove_doubled_verts(true).  For a point, it calls
//               remove_nonunique_verts().
////////////////////////////////////////////////////////////////////
void EggPolygon::
cleanup() {
  remove_doubled_verts(true);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPolygon::write
//       Access: Public, Virtual
//  Description: Writes the polygon to the indicated output stream in
//               Egg format.
////////////////////////////////////////////////////////////////////
void EggPolygon::
write(ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Polygon>");
  write_body(out, indent_level+2);
  indent(out, indent_level) << "}\n";
}
