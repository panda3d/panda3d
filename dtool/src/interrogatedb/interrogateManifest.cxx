// Filename: interrogateManifest.cxx
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

#include "interrogateManifest.h"
#include "indexRemapper.h"
#include "interrogate_datafile.h"

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::output
//       Access: Public
//  Description: Formats the InterrogateManifest data for output to a data
//               file.
////////////////////////////////////////////////////////////////////
void InterrogateManifest::
output(ostream &out) const {
  InterrogateComponent::output(out);
  out << _flags << " "
      << _int_value << " "
      << _type << " "
      << _getter << " ";
  idf_output_string(out, _definition);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::input
//       Access: Public
//  Description: Reads the data file as previously formatted by
//               output().
////////////////////////////////////////////////////////////////////
void InterrogateManifest::
input(istream &in) {
  InterrogateComponent::input(in);
  in >> _flags >> _int_value >> _type >> _getter;
  idf_input_string(in, _definition);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::remap_indices
//       Access: Public
//  Description: Remaps all internal index numbers according to the
//               indicated map.  This called from
//               InterrogateDatabase::remap_indices().
////////////////////////////////////////////////////////////////////
void InterrogateManifest::
remap_indices(const IndexRemapper &remap) {
  _type = remap.map_from(_type);
  _getter = remap.map_from(_getter);
}
