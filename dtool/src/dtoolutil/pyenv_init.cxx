/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pyenv_init.cxx
 * @author rdb
 * @date 2025-02-02
 */

#include "pyenv_init.h"
#include "py_panda.h"
#include "executionEnvironment.h"

/**
 * Called when panda3d.core is initialized, does some initialization specific
 * to the Python environment.
 */
void
pyenv_init() {
  // MAIN_DIR needs to be set very early; this seems like a convenient place
  // to do that.  Perhaps we'll find a better place for this in the future.
  static bool initialized_main_dir = false;
  if (!initialized_main_dir) {
    /*if (interrogatedb_cat.is_debug()) {
      // Good opportunity to print this out once, at startup.
      interrogatedb_cat.debug()
        << "Python " << version << "\n";
    }*/

    if (!ExecutionEnvironment::has_environment_variable("MAIN_DIR")) {
      // Grab the __main__ module and extract its __file__ attribute.
      Filename main_dir;
      PyObject *main_module = PyImport_ImportModule("__main__");
      PyObject *file_attr = nullptr;
      if (main_module != nullptr) {
        file_attr = PyObject_GetAttrString(main_module, "__file__");
      } else {
        std::cerr << "Warning: unable to import __main__\n";
      }
      if (file_attr == nullptr) {
        // Must be running in the interactive interpreter.  Use the CWD.
        main_dir = ExecutionEnvironment::get_cwd();
      } else {
#if PY_MAJOR_VERSION >= 3
        Py_ssize_t length;
        wchar_t *buffer = PyUnicode_AsWideCharString(file_attr, &length);
        if (buffer != nullptr) {
          main_dir = Filename::from_os_specific_w(std::wstring(buffer, length));
          main_dir.make_absolute();
          main_dir = main_dir.get_dirname();
          PyMem_Free(buffer);
        }
#else
        char *buffer;
        Py_ssize_t length;
        if (PyString_AsStringAndSize(file_attr, &buffer, &length) != -1) {
          main_dir = Filename::from_os_specific(std::string(buffer, length));
          main_dir.make_absolute();
          main_dir = main_dir.get_dirname();
        }
#endif
        else {
          std::cerr << "Invalid string for __main__.__file__\n";
        }
      }
      ExecutionEnvironment::shadow_environment_variable("MAIN_DIR", main_dir.to_os_specific());
      PyErr_Clear();
    }
    initialized_main_dir = true;
  }

  // Also, while we are at it, initialize the thread swap hook.
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
  global_thread_state_swap = PyThreadState_Swap;
#endif
}
