// Filename: eggBase.cxx
// Created by:  drose (14Feb00)
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
