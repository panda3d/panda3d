// Filename: fltVertex.I
// Created by:  drose (30Aug00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: FltVertex::has_color
//       Access: Public
//  Description: Returns true if the vertex has a primary color
//               indicated, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FltVertex::
has_color() const {
  // Even if the no_color bit is not set, if the color_index is -1,
  // the vertex doesn't have a color (unless we've got packed color).
  return ((_flags & F_no_color) == 0 &&
	  (_color_index != -1 || ((_flags & F_packed_color) != 0)));
}
