/* Filename: interrogate_request.h
 * Created by:  drose (01Aug00)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
 *
 * All use of this software is subject to the terms of the Panda 3d
 * Software license.  You should have received a copy of this license
 * along with this source code; you will also find a current copy of
 * the license at http://www.panda3d.org/license.txt .
 *
 * To contact the maintainers of this program write to
 * panda3d@yahoogroups.com .
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
EXPCL_DTOOLCONFIG void interrogate_request_database(const char *database_filename);



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
EXPCL_DTOOLCONFIG void interrogate_request_module(InterrogateModuleDef *def);

#ifdef TRACK_IN_INTERPRETER
/* 
 * If we're tracking whether we're currently running in Python code
 * (mainly for the purpose of debug logging from memory allocation
 * callbacks), this variable will record that state.  It will be set
 * true whenever we return to Python code, and false whenever we are
 * entering local C or C++ code.  The flag will be toggled off and
 * on within each generated Python wrapper function.
 *
 * This will mis-categorize some code that runs at static
 * initialization time, but it will correctly identify the vast
 * majority of code.
 */
EXPCL_DTOOLCONFIG extern int in_interpreter;
#endif  // TRACK_IN_INTERPRETER

#ifdef __cplusplus
}
#endif

#endif

