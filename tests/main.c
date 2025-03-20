/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file main.c
 * @author rdb
 * @date 2024-11-03
 */

/**
 * This script embeds the Python interpreter and runs the unit testing suite.
 * It is designed for use with a statically built Panda3D, where the Panda3D
 * modules are linked directly into the interpreter.
 */

#include <Python.h>

#include "pandabase.h"

#ifdef LINK_ALL_STATIC
extern PyObject *PyInit_core();

#ifdef HAVE_DIRECT
extern PyObject *PyInit_direct();
#endif

#ifdef HAVE_PHYSICS
extern PyObject *PyInit_physics();
#endif

#ifdef HAVE_EGG
extern PyObject *PyInit_egg();
extern EXPCL_PANDAEGG void init_libpandaegg();
#endif

#ifdef HAVE_BULLET
extern PyObject *PyInit_bullet();
#endif

extern EXPCL_PANDA_PNMIMAGETYPES void init_libpnmimagetypes();
#endif  // LINK_ALL_STATIC


int main(int argc, char **argv) {
  PyStatus status;
  PyConfig config;
  PyConfig_InitPythonConfig(&config);

  PyConfig_SetBytesString(&config, &config.run_module, "pytest");
  config.parse_argv = 0;

  status = PyConfig_SetBytesArgv(&config, argc, argv);
  if (PyStatus_Exception(status)) {
    goto exception;
  }

  status = Py_InitializeFromConfig(&config);
  if (PyStatus_Exception(status)) {
    goto exception;
  }
  PyConfig_Clear(&config);

#ifdef LINK_ALL_STATIC
#ifdef HAVE_EGG
  init_libpandaegg();
#endif

  init_libpnmimagetypes();

  {
    PyObject *panda3d_module = PyImport_ImportModule("panda3d");
    PyObject *panda3d_dict = PyModule_GetDict(panda3d_module);
    PyObject *sys_modules = PySys_GetObject("modules");

    PyObject *core_module = PyInit_core();
    PyDict_SetItemString(panda3d_dict, "core", core_module);
    PyDict_SetItemString(sys_modules, "panda3d.core", core_module);

#ifdef HAVE_DIRECT
    PyObject *direct_module = PyInit_direct();
    PyDict_SetItemString(panda3d_dict, "direct", direct_module);
    PyDict_SetItemString(sys_modules, "panda3d.direct", direct_module);
#endif

#ifdef HAVE_PHYSICS
    PyObject *physics_module = PyInit_physics();
    PyDict_SetItemString(panda3d_dict, "physics", physics_module);
    PyDict_SetItemString(sys_modules, "panda3d.physics", physics_module);
#endif

#ifdef HAVE_EGG
    PyObject *egg_module = PyInit_egg();
    PyDict_SetItemString(panda3d_dict, "egg", egg_module);
    PyDict_SetItemString(sys_modules, "panda3d.egg", egg_module);
#endif

#ifdef HAVE_BULLET
    PyObject *bullet_module = PyInit_bullet();
    PyDict_SetItemString(panda3d_dict, "bullet", bullet_module);
    PyDict_SetItemString(sys_modules, "panda3d.bullet", bullet_module);
#endif
  }
#endif  // LINK_ALL_STATIC

#ifdef __EMSCRIPTEN__
  // Default fd capturing doesn't work on emscripten
  PyRun_SimpleString("import sys; sys.argv.insert(1, '--capture=sys')");
#endif

  return Py_RunMain();

exception:
  PyConfig_Clear(&config);
  if (PyStatus_IsExit(status)) {
    return status.exitcode;
  }
  Py_ExitStatusException(status);
}
