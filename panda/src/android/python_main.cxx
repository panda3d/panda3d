/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file python_main.cxx
 * @author rdb
 * @date 2018-02-12
 */

#include "dtoolbase.h"
#include "config_android.h"
#include "executionEnvironment.h"

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#include <Python.h>
#if PY_MAJOR_VERSION >= 3
#include <wchar.h>
#endif

#include <dlfcn.h>

/**
 * The main entry point for the Python activity.  Called by android_main.
 */
int main(int argc, char *argv[]) {
  if (argc <= 1) {
    return 1;
  }

  // Help out Python by telling it which encoding to use
  Py_FileSystemDefaultEncoding = "utf-8";

  Py_SetProgramName(Py_DecodeLocale("ppython", nullptr));

  // Set PYTHONHOME to the location of the .apk file.
  std::string apk_path = ExecutionEnvironment::get_binary_name();
  Py_SetPythonHome(Py_DecodeLocale(apk_path.c_str(), nullptr));

  // We need to make zlib available to zipimport, but I don't know how
  // we could inject our import hook before Py_Initialize, so instead
  // load it as though it were a built-in module.
  void *zlib = dlopen("libpy.zlib.so", RTLD_NOW);
  if (zlib != nullptr) {
    void *init = dlsym(zlib, "PyInit_zlib");
    if (init != nullptr) {
      PyImport_AppendInittab("zlib", (PyObject *(*)())init);
    }
  }

  Py_Initialize();

  // This is used by the import hook to locate the module libraries.
  Filename dtool_name = ExecutionEnvironment::get_dtool_name();
  std::string native_dir = dtool_name.get_dirname();
  PyObject *py_native_dir = PyUnicode_FromStringAndSize(native_dir.c_str(), native_dir.size());
  PySys_SetObject("_native_library_dir", py_native_dir);
  Py_DECREF(py_native_dir);

  int sts = 1;
  FILE *fp = fopen(argv[1], "r");
  if (fp != nullptr) {
    int res = PyRun_AnyFile(fp, argv[1]);
    if (res > 0) {
      sts = 0;
    } else {
      android_cat.error() << "Error running " << argv[1] << "\n";
      PyErr_Print();
    }
  } else {
    android_cat.error() << "Unable to open " << argv[1] << "\n";
  }

  Py_Finalize();
  return sts;
}
