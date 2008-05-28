// Filename: eggSingleBase.cxx
// Created by:  drose (21Jul03)
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

#include "eggSingleBase.h"

#include "eggGroupNode.h"
#include "eggTexture.h"
#include "eggFilenameNode.h"
#include "eggComment.h"
#include "dcast.h"
#include "string_utils.h"

////////////////////////////////////////////////////////////////////
//     Function: EggSingleBase::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggSingleBase::
EggSingleBase() :
  _data(new EggData)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggSingleBase::as_reader
//       Access: Public, Virtual
//  Description: Returns this object as an EggReader pointer, if it is
//               in fact an EggReader, or NULL if it is not.
//
//               This is intended to work around the C++ limitation
//               that prevents downcasts past virtual inheritance.
//               Since both EggReader and EggWriter inherit virtually
//               from EggSingleBase, we need functions like this to downcast
//               to the appropriate pointer.
////////////////////////////////////////////////////////////////////
EggReader *EggSingleBase::
as_reader() {
  return (EggReader *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSingleBase::as_writer
//       Access: Public, Virtual
//  Description: Returns this object as an EggWriter pointer, if it is
//               in fact an EggWriter, or NULL if it is not.
//
//               This is intended to work around the C++ limitation
//               that prevents downcasts past virtual inheritance.
//               Since both EggReader and EggWriter inherit virtually
//               from EggSingleBase, we need functions like this to downcast
//               to the appropriate pointer.
////////////////////////////////////////////////////////////////////
EggWriter *EggSingleBase::
as_writer() {
  return (EggWriter *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSingleBase::post_command_line
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool EggSingleBase::
post_command_line() {
  if (_got_coordinate_system) {
    _data->set_coordinate_system(_coordinate_system);
  }

  return EggBase::post_command_line();
}

