// Filename: interrogateElement.cxx
// Created by:  drose (11Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "interrogateElement.h"
#include "indexRemapper.h"
#include "interrogate_datafile.h"

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::output
//       Access: Public
//  Description: Formats the InterrogateElement data for output to a data
//               file.
////////////////////////////////////////////////////////////////////
void InterrogateElement::
output(ostream &out) const {
  InterrogateComponent::output(out);
  out << _flags << " "
      << _type << " "
      << _getter << " "
      << _setter << " ";
  idf_output_string(out, _scoped_name);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::input
//       Access: Public
//  Description: Reads the data file as previously formatted by
//               output().
////////////////////////////////////////////////////////////////////
void InterrogateElement::
input(istream &in) {
  InterrogateComponent::input(in);
  in >> _flags >> _type >> _getter >> _setter;
  idf_input_string(in, _scoped_name);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::remap_indices
//       Access: Public
//  Description: Remaps all internal index numbers according to the
//               indicated map.  This called from
//               InterrogateDatabase::remap_indices().
////////////////////////////////////////////////////////////////////
void InterrogateElement::
remap_indices(const IndexRemapper &remap) {
  _type = remap.map_from(_type);
  _getter = remap.map_from(_getter);
  _setter = remap.map_from(_setter);
}
