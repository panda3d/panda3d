// Filename: interrogateComponent.cxx
// Created by:  drose (08Aug00)
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

#include "interrogateComponent.h"
#include "interrogate_datafile.h"


////////////////////////////////////////////////////////////////////
//     Function: InterrogateComponent::output
//       Access: Public
//  Description: Formats the component for output to a data file.
////////////////////////////////////////////////////////////////////
void InterrogateComponent::
output(ostream &out) const {
  idf_output_string(out, _name);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateComponent::input
//       Access: Public
//  Description: Reads the data file as previously formatted by
//               output().
////////////////////////////////////////////////////////////////////
void InterrogateComponent::
input(istream &in) {
  idf_input_string(in, _name);
}
