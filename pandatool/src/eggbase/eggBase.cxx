// Filename: eggBase.cxx
// Created by:  drose (14Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "eggBase.h"

#include <eggComment.h>

////////////////////////////////////////////////////////////////////
//     Function: EggBase::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggBase::
EggBase() {
  add_option
    ("cs", "coordinate-system", 80,
     "Specify the coordinate system to operate in.  This may be one of "
     "'y-up', 'z-up', 'y-up-left', or 'z-up-left'.",
     &EggBase::dispatch_coordinate_system,
     &_got_coordinate_system, &_coordinate_system);

  _coordinate_system = CS_yup_right;
}

////////////////////////////////////////////////////////////////////
//     Function: EggBase::as_reader
//       Access: Public, Virtual
//  Description: Returns this object as an EggReader pointer, if it is
//               in fact an EggReader, or NULL if it is not.
//
//               This is intended to work around the C++ limitation
//               that prevents downcasts past virtual inheritance.
//               Since both EggReader and EggWriter inherit virtually
//               from EggBase, we need functions like this to downcast
//               to the appropriate pointer.
////////////////////////////////////////////////////////////////////
EggReader *EggBase::
as_reader() {
  return (EggReader *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggBase::as_writer
//       Access: Public, Virtual
//  Description: Returns this object as an EggWriter pointer, if it is
//               in fact an EggWriter, or NULL if it is not.
//
//               This is intended to work around the C++ limitation
//               that prevents downcasts past virtual inheritance.
//               Since both EggReader and EggWriter inherit virtually
//               from EggBase, we need functions like this to downcast
//               to the appropriate pointer.
////////////////////////////////////////////////////////////////////
EggWriter *EggBase::
as_writer() {
  return (EggWriter *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggBase::post_command_line
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool EggBase::
post_command_line() {
  if (_got_coordinate_system) {
    _data.set_coordinate_system(_coordinate_system);
  }

  return ProgramBase::post_command_line();
}


////////////////////////////////////////////////////////////////////
//     Function: EggBase::append_command_comment
//       Access: Protected
//  Description: Inserts a comment into the beginning of the indicated
//               egg file corresponding to the command line that
//               invoked this program.
//
//               Normally this function is called automatically when
//               appropriate by EggWriter, and it's not necessary to
//               call it explicitly.
////////////////////////////////////////////////////////////////////
void EggBase::
append_command_comment(EggData &data) {
  string comment;

  comment = _program_name;
  Args::const_iterator ai;
  for (ai = _program_args.begin(); ai != _program_args.end(); ++ai) {
    const string &arg = (*ai);

    // First, check to see if the string is shell-acceptable.
    bool legal = true;
    string::const_iterator si;
    for (si = arg.begin(); legal && si != arg.end(); ++si) {
      switch (*si) {
      case ' ':
      case '\n':
      case '\t':
      case '*':
      case '?':
      case '\\':
      case '(':
      case ')':
      case '|':
      case '&':
      case '<':
      case '>':
      case '"':
      case ';':
      case '$':
        legal = false;
      }
    }

    if (legal) {
      comment += " " + arg;
    } else {
      comment += " '" + arg + "'";
    }
  }

  data.insert(data.begin(), new EggComment("", comment));
}
