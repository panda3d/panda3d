// Filename: eggMultiBase.cxx
// Created by:  drose (02Nov00)
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

#include "eggMultiBase.h"
#include "eggBase.h"
#include "eggData.h"
#include "eggComment.h"
#include "filename.h"
#include "dSearchPath.h"

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

  _coordinate_system = CS_yup_right;
}


////////////////////////////////////////////////////////////////////
//     Function: EggMultiBase::append_command_comment
//       Access: Protected
//  Description: Inserts a comment into the beginning of the indicated
//               egg file corresponding to the command line that
//               invoked this program.
//
//               Normally this function is called automatically when
//               appropriate by EggWriter, and it's not necessary to
//               call it explicitly.
////////////////////////////////////////////////////////////////////
void EggMultiBase::
append_command_comment(EggData &data) {
  append_command_comment(data, get_exec_command());
}

////////////////////////////////////////////////////////////////////
//     Function: EggMultiBase::append_command_comment
//       Access: Protected, Static
//  Description: Inserts a comment into the beginning of the indicated
//               egg file corresponding to the command line that
//               invoked this program.
//
//               Normally this function is called automatically when
//               appropriate by EggWriter, and it's not necessary to
//               call it explicitly.
////////////////////////////////////////////////////////////////////
void EggMultiBase::
append_command_comment(EggData &data, const string &comment) {
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
PT(EggData) EggMultiBase::
read_egg(const Filename &filename) {
  PT(EggData) data = new EggData;

  if (!data->read(filename)) {
    // Failure reading.
    return (EggData *)NULL;
  }

  DSearchPath file_path;
  file_path.append_directory(filename.get_dirname());

  // We always resolve filenames first based on the source egg
  // filename, since egg files almost always store relative paths.
  // This is a temporary kludge around integrating the path_replace
  // system with the EggData better.
  data->resolve_filenames(file_path);

  if (_force_complete) {
    if (!data->load_externals()) {
      return (EggData *)NULL;
    }
  }

  // Now resolve the filenames again according to the user's
  // specified _path_replace.
  EggBase::convert_paths(data, _path_replace, file_path);

  if (_got_coordinate_system) {
    data->set_coordinate_system(_coordinate_system);
  } else {
    _coordinate_system = data->get_coordinate_system();
    _got_coordinate_system = true;
  }

  return data;
}
