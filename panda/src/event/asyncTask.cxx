// Filename: asyncTask.cxx
// Created by:  drose (23Aug06)
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
  out << get_type() << " " << get_name();
}
