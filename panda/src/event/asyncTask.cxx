// Filename: asyncTask.cxx
// Created by:  drose (23Aug06)
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

#include "asyncTask.h"

TypeHandle AsyncTask::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
AsyncTask::
~AsyncTask() {
  nassertv(_state != S_active && _manager == NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AsyncTask::
output(ostream &out) const {
  out << get_type();
}
