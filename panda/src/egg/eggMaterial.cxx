// Filename: eggMaterial.cxx
// Created by:  drose (29Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "eggMaterial.h"

#include <indent.h>

TypeHandle EggMaterial::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggMaterial::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggMaterial::
EggMaterial(const string &mref_name)
  : EggNode(mref_name)
{
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMaterial::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggMaterial::
EggMaterial(const EggMaterial &copy) 
  : EggNode(copy),
    _diff(copy._diff),
    _flags(copy._flags)
{
}


////////////////////////////////////////////////////////////////////
//     Function: EggMaterial::write
//       Access: Public, Virtual
//  Description: Writes the material definition to the indicated output
//               stream in Egg format.
////////////////////////////////////////////////////////////////////
void EggMaterial::
write(ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Material>");

  if (has_diff()) {
    indent(out, indent_level + 2)
      << "<Scalar> diffr { " << get_diff()[0] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> diffg { " << get_diff()[1] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> diffb { " << get_diff()[2] << " }\n";
  }

  indent(out, indent_level) << "}\n";
}
