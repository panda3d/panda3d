// Filename: lwoPolygons.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoPolygons.h"
#include "lwoInputFile.h"

#include <indent.h>

TypeHandle LwoPolygons::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoPolygons::get_num_polygons
//       Access: Public
//  Description: Returns the number of polygons of this group.
////////////////////////////////////////////////////////////////////
int LwoPolygons::
get_num_polygons() const {
  return _polygons.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LwoPolygons::get_polygon
//       Access: Public
//  Description: Returns the nth polygon of this group.
////////////////////////////////////////////////////////////////////
LwoPolygons::Polygon *LwoPolygons::
get_polygon(int n) const {
  nassertr(n >= 0 && n < (int)_polygons.size(), (Polygon *)NULL);
  return _polygons[n];
}

////////////////////////////////////////////////////////////////////
//     Function: LwoPolygons::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoPolygons::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _polygon_type = lin->get_id();

  while (lin->get_bytes_read() < stop_at && !lin->is_eof()) {
    int nf = lin->get_be_int16();
    int num_vertices = nf & PF_numverts_mask;

    PT(Polygon) poly = new Polygon;
    poly->_flags = nf & ~PF_numverts_mask;

    for (int i = 0; i < num_vertices; i++) {
      poly->_vertices.push_back(lin->get_vx());
    }

    _polygons.push_back(poly);
  }

  return (lin->get_bytes_read() == stop_at);
}

////////////////////////////////////////////////////////////////////
//     Function: LwoPolygons::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LwoPolygons::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { polygon_type = " << _polygon_type
    << ", " << _polygons.size() << " polygons }\n";
}
