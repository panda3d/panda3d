// Filename: interrogate_request.C
// Created by:  drose (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "interrogate_request.h"
#include "interrogateDatabase.h"

#include <string.h>  // for strdup

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
