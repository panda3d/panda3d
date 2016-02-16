/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file run_p3dpython.cxx
 * @author drose
 * @date 2009-08-29
 */

#include "p3dPythonRun.h"
#include "run_p3dpython.h"

////////////////////////////////////////////////////////////////////
//     Function: run_p3dpython
//  Description: This externally-visible function is the main entry
//               point to this DLL, and it starts the whole thing
//               running.  Returns the exit status, which will be
//               0 on success, 1 or otherwise on failure.
////////////////////////////////////////////////////////////////////
int
run_p3dpython(const char *program_name, const char *archive_file,
              FHandle input_handle, FHandle output_handle,
              const char *log_pathname, bool interactive_console) {
  P3DPythonRun::_global_ptr =
    new P3DPythonRun(program_name, archive_file, input_handle, output_handle,
                     log_pathname, interactive_console);
  int result = P3DPythonRun::_global_ptr->run_python();
  delete P3DPythonRun::_global_ptr;
  P3DPythonRun::_global_ptr = NULL;
  return result;
}
