// Filename: eggMultiBase.cxx
// Created by:  drose (02Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "eggMultiBase.h"

#include <eggData.h>
#include <eggComment.h>
#include <filename.h>

////////////////////////////////////////////////////////////////////
//     Function: EggMultiBase::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggMultiBase::
EggMultiBase() {
  add_option
    ("cs", "coordinate-system", 80, 
     "Specify the coordinate system to operate in.  This may be one of "
     "'y-up', 'z-up', 'y-up-left', or 'z-up-left'.",
     &EggMultiBase::dispatch_coordinate_system, 
     &_got_coordinate_system, &_coordinate_system);

  add_option
    ("f", "", 80, 
     "Force complete loading: load up the egg file along with all of its "
     "external references.",
     &EggMultiBase::dispatch_none, &_force_complete);
}


////////////////////////////////////////////////////////////////////
//     Function: EggMultiBase::append_command_comment
//       Access: Protected
//  Description: Inserts a comment into the beginning of the indicated
//               egg file corresponding to the command line that
//               invoked this program.
//
//               Normally this function is called automatically when
//               appropriate by EggMultiFilter, and it's not necessary
//               to call it explicitly.
////////////////////////////////////////////////////////////////////
void EggMultiBase::
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

////////////////////////////////////////////////////////////////////
//     Function: EggMultiBase::read_egg
//       Access: Protected, Virtual
//  Description: Allocates and returns a new EggData structure that
//               represents the indicated egg file.  If the egg file
//               cannot be read for some reason, returns NULL. 
//
//               This can be overridden by derived classes to control
//               how the egg files are read, or to extend the
//               information stored with each egg structure, by
//               deriving from EggData.
////////////////////////////////////////////////////////////////////
EggData *EggMultiBase::
read_egg(const Filename &filename) {
  EggData *data = new EggData;

  // First, we always try to resolve a filename from the current
  // directory.  This means a local filename will always be found
  // before the model path is searched.
  Filename local_filename = filename;
  DSearchPath local_path(".");
  local_filename.resolve_filename(local_path);

  if (!data->read(local_filename)) {
    // Failure reading.
    delete data;
    return (EggData *)NULL;
  }

  if (_force_complete) {
    if (!data->resolve_externals()) {
      delete data;
      return (EggData *)NULL;
    }
  }
   
  return data;
}
