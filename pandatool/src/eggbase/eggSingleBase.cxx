/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSingleBase.cxx
 * @author drose
 * @date 2003-07-21
 */

#include "eggSingleBase.h"

#include "eggGroupNode.h"
#include "eggTexture.h"
#include "eggFilenameNode.h"
#include "eggComment.h"
#include "dcast.h"
#include "string_utils.h"

/**
 *
 */
EggSingleBase::
EggSingleBase() :
  _data(new EggData)
{
}

/**
 * Returns this object as an EggReader pointer, if it is in fact an EggReader,
 * or NULL if it is not.
 *
 * This is intended to work around the C++ limitation that prevents downcasts
 * past virtual inheritance.  Since both EggReader and EggWriter inherit
 * virtually from EggSingleBase, we need functions like this to downcast to
 * the appropriate pointer.
 */
EggReader *EggSingleBase::
as_reader() {
  return nullptr;
}

/**
 * Returns this object as an EggWriter pointer, if it is in fact an EggWriter,
 * or NULL if it is not.
 *
 * This is intended to work around the C++ limitation that prevents downcasts
 * past virtual inheritance.  Since both EggReader and EggWriter inherit
 * virtually from EggSingleBase, we need functions like this to downcast to
 * the appropriate pointer.
 */
EggWriter *EggSingleBase::
as_writer() {
  return nullptr;
}

/**
 *
 */
bool EggSingleBase::
post_command_line() {
  if (_got_coordinate_system) {
    _data->set_coordinate_system(_coordinate_system);
  }

  return EggBase::post_command_line();
}
