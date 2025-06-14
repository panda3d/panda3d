/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogate_request.cxx
 * @author drose
 * @date 2000-08-01
 */

#include "interrogate_request.h"
#include "interrogateDatabase.h"

#include <string.h>  // for strdup

void
interrogate_request_database(const char *database_filename) {
  InterrogateModuleDef *def = new InterrogateModuleDef;
  memset(def, 0, sizeof(InterrogateModuleDef));
#ifdef _WIN32
  def->database_filename = _strdup(database_filename);
#else
  def->database_filename = strdup(database_filename);
#endif

  // Don't think of this as a leak; think of it as a one-time database
  // allocation.
  InterrogateDatabase::get_ptr()->request_module(def);
}

void
interrogate_request_module(InterrogateModuleDef *def) {
  InterrogateDatabase::get_ptr()->request_module(def);
}
