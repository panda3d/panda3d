/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogate_request.h
 * @author drose
 * @date 2000-08-01
 */

#ifndef INTERROGATE_REQUEST
#define INTERROGATE_REQUEST

#include "dtoolbase.h"

/*
 * The functions here are simple functions that are intended to be
 * called during static init time for the various libraries that
 * contain interrogate data.  They provide interfaces to add the
 * module's interrogate data to the main interrogate database.
 *
 * The interface is entirely C here--no C++--so that it may be called
 * from C modules if required.
 */


#ifdef __cplusplus
extern "C" {
#endif


/*
 * This is the simplest interface.  It just requests that the given
 * database filename (*.in) be read in.  This makes the interrogate
 * data available, but doesn't allow matching the database information
 * up with any compiled-in function wrappers or anything.
 */
EXPCL_INTERROGATEDB void interrogate_request_database(const char *database_filename);



/* The more sophisticated interface uses these structures. */

typedef struct {
  const char *name;
  int index_offset;
} InterrogateUniqueNameDef;

typedef struct {
  int file_identifier;

  const char *library_name;
  const char *library_hash_name;
  const char *module_name;
  const char *database_filename;

  InterrogateUniqueNameDef *unique_names;
  int num_unique_names;

  void **fptrs;
  int num_fptrs;

  int first_index;
  int next_index;
} InterrogateModuleDef;


/*
 * This requests that the given module be loaded and made available.
 * This includes all of the function pointers and/or unique names that
 * might be compiled in.
 */
EXPCL_INTERROGATEDB void interrogate_request_module(InterrogateModuleDef *def);

#ifdef __cplusplus
}
#endif

#endif
