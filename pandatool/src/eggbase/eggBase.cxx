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

#include "eggGroupNode.h"
#include "eggTexture.h"
#include "eggFilenameNode.h"
#include "eggComment.h"
#include "dcast.h"

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
//     Function: EggBase::convert_paths
//       Access: Public, Static
//  Description: Recursively walks the egg hierarchy.  Any filenames
//               encountered are replaced according to the indicated
//               PathReplace.
////////////////////////////////////////////////////////////////////
void EggBase::
convert_paths(EggNode *node, PathReplace *path_replace,
              const DSearchPath &additional_path) {
  if (node->is_of_type(EggTexture::get_class_type())) {
    EggTexture *egg_tex = DCAST(EggTexture, node);
    Filename fullpath = 
      path_replace->match_path(egg_tex->get_filename(), additional_path);
    egg_tex->set_filename(path_replace->store_path(fullpath));
    egg_tex->set_fullpath(fullpath);

    if (egg_tex->has_alpha_filename()) {
      Filename alpha_fullpath = 
        path_replace->match_path(egg_tex->get_alpha_filename(), additional_path);
      egg_tex->set_alpha_filename(path_replace->store_path(alpha_fullpath));
      egg_tex->set_alpha_fullpath(alpha_fullpath);
    }

  } else if (node->is_of_type(EggFilenameNode::get_class_type())) {
    EggFilenameNode *egg_fnode = DCAST(EggFilenameNode, node);

    Filename fullpath = 
      path_replace->match_path(egg_fnode->get_filename(), additional_path);
    egg_fnode->set_filename(path_replace->store_path(fullpath));
    egg_fnode->set_fullpath(fullpath);

  } else if (node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *egg_group = DCAST(EggGroupNode, node);
    EggGroupNode::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      convert_paths(*ci, path_replace, additional_path);
    }
  }
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
  string comment = get_exec_command();
  data.insert(data.begin(), new EggComment("", comment));
}
