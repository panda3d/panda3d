// Filename: lwoGroupChunk.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoGroupChunk.h"
#include "lwoInputFile.h"

#include <notify.h>

TypeHandle LwoGroupChunk::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoGroupChunk::get_num_children
//       Access: Public
//  Description: Returns the number of child chunks of this group.
////////////////////////////////////////////////////////////////////
int LwoGroupChunk::
get_num_children() const {
  return _children.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LwoGroupChunk::get_child
//       Access: Public
//  Description: Returns the nth child chunk of this group.
////////////////////////////////////////////////////////////////////
IffChunk *LwoGroupChunk::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), (IffChunk *)NULL);
  return _children[n];
}

////////////////////////////////////////////////////////////////////
//     Function: LwoGroupChunk::read_children_iff
//       Access: Public
//  Description: Reads a sequence of child chunks, until byte stop_at
//               has been been reached, and stores them as the
//               children.  Returns true if successful (and exactly
//               the correct number of bytes were read), or false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool LwoGroupChunk::
read_children_iff(IffInputFile *in, size_t stop_at) {
  while (in->get_bytes_read() < stop_at && !in->is_eof()) {
    PT(IffChunk) chunk = in->get_chunk();
    if (chunk == (IffChunk *)NULL) {
      return false;
    }
    _children.push_back(chunk);
  }

  return (in->get_bytes_read() == stop_at);
}

////////////////////////////////////////////////////////////////////
//     Function: LwoGroupChunk::write_children
//       Access: Public
//  Description: Formats the list of children for output to the user
//               (primarily for debugging), one per line.
////////////////////////////////////////////////////////////////////
void LwoGroupChunk::
write_children(ostream &out, int indent_level) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write(out, indent_level);
  }
}
