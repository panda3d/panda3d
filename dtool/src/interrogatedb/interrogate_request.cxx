// Filename: interrogate_request.cxx
// Created by:  drose (01Aug00)
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

#include "interrogate_request.h"
#include "interrogateDatabase.h"

#include <string.h>  // for strdup

#ifdef TRACK_IN_INTERPRETER
int in_interpreter = false;
#endif

void
interrogate_request_database(const char *database_filename) {
  InterrogateModuleDef *def = new InterrogateModuleDef;
  memset(def, 0, sizeof(InterrogateModuleDef));
  def->database_filename = strdup(database_filename);

  // Don't think of this as a leak; think of it as a one-time database
  // allocation.
  InterrogateDatabase::get_ptr()->request_module(def);
}

void
interrogate_request_module(InterrogateModuleDef *def) {
  InterrogateDatabase::get_ptr()->request_module(def);
}
