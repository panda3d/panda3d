// Filename: eggNamedObject.cxx
// Created by:  drose (16Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "eggNamedObject.h"
#include "eggMiscFuncs.h"

#include <indent.h>

TypeHandle EggNamedObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggNamedObject::write_header
//       Access: Public
//  Description: Writes the first line of the egg object,
//               e.g. "<Group> group_name {" or some such.  It
//               automatically enquotes the name if it contains any
//               special characters.  egg_keyword is the keyword that
//               begins the line, e.g. "<Group>".
////////////////////////////////////////////////////////////////////
void EggNamedObject::
write_header(ostream &out, int indent_level, const char *egg_keyword) const {
  indent(out, indent_level) << egg_keyword << " ";

  if (has_name()) {
    enquote_string(out, get_name()) << " {\n";
  } else {
    out << "{\n";
  }
}
