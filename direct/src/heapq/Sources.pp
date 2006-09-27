// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib

// This directory strictly contains a Python utility; therefore, only
// build it if we actually have Python.
#define BUILD_DIRECTORY $[HAVE_PYTHON]


#define OTHER_LIBS \
  pandaexpress:m \
  dconfig:c dtoolconfig:m \
  dtoolutil:c dtoolbase:c prc:c dtool:m

#begin metalib_target
  #define TARGET heapq

  // Tell ppremake to treat this file as if it had been generated via
  // interrogate.  On OSX, this will move it into the .so, instead of
  // the .dylib, so that it can be imported into Python.
  #define PYTHON_MODULE_ONLY 1

  #define SOURCES heapq.cxx
#end metalib_target

