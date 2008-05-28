// Filename: functionWriters.cxx
// Created by:  drose (14Sep01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "functionWriters.h"

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriters::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FunctionWriters::
FunctionWriters() {
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriters::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FunctionWriters::
~FunctionWriters() {
  Writers::iterator wi;
  for (wi = _writers.begin(); wi != _writers.end(); ++wi) {
    delete (*wi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriters::add_writer
//       Access: Public
//  Description: Adds the indicated FunctionWriter to the set of
//               functions to be written, unless there is already a
//               matching FunctionWriter.
//
//               The return value is the FunctionWriter pointer that
//               was added to the set, which may be the same pointer
//               or a previously-allocated (but equivalent) pointer.
////////////////////////////////////////////////////////////////////
FunctionWriter *FunctionWriters::
add_writer(FunctionWriter *writer) {
  pair<Writers::iterator, bool> result = _writers.insert(writer);
  if (!result.second) {
    // Already there; delete the pointer.
    delete writer;
  }

  // Return the pointer that is in the set.
  return *result.first;
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriters::write_prototypes
//       Access: Public
//  Description: Generates prototypes for all of the functions.
////////////////////////////////////////////////////////////////////
void FunctionWriters::
write_prototypes(ostream &out) {
  Writers::iterator wi;
  for (wi = _writers.begin(); wi != _writers.end(); ++wi) {
    FunctionWriter *writer = (*wi);
    writer->write_prototype(out);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriters::write_code
//       Access: Public
//  Description: Generates all of the functions.
////////////////////////////////////////////////////////////////////
void FunctionWriters::
write_code(ostream &out) {
  Writers::iterator wi;
  for (wi = _writers.begin(); wi != _writers.end(); ++wi) {
    FunctionWriter *writer = (*wi);
    writer->write_code(out);
  }
}
