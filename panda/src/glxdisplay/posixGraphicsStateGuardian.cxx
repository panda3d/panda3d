// Filename: posixGraphicsStateGuardian.cxx
// Created by:  drose (14Jan12)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "posixGraphicsStateGuardian.h"
#include "config_glxdisplay.h"
#include <dlfcn.h>

TypeHandle PosixGraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PosixGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PosixGraphicsStateGuardian::
PosixGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe) :
  GLGraphicsStateGuardian(engine, pipe)
{
  _libgl_handle = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PosixGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PosixGraphicsStateGuardian::
~PosixGraphicsStateGuardian() {
  if (_libgl_handle != (void *)NULL) {
    dlclose(_libgl_handle);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PosixGraphicsStateGuardian::do_get_extension_func
//       Access: Public, Virtual
//  Description: Returns the pointer to the GL extension function with
//               the indicated name.  It is the responsibility of the
//               caller to ensure that the required extension is
//               defined in the OpenGL runtime prior to calling this;
//               it is an error to call this for a function that is
//               not defined.
////////////////////////////////////////////////////////////////////
void *PosixGraphicsStateGuardian::
do_get_extension_func(const char *name) {
  nassertr(name != NULL, NULL);

  if (glx_get_os_address) {
    return get_system_func(name);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PosixGraphicsStateGuardian::get_system_func
//       Access: Protected
//  Description: Support for get_extension_func(), above, that uses
//               system calls to find a GL or GLX function (in the
//               absence of a working glxGetProcAddress() function to
//               call).
////////////////////////////////////////////////////////////////////
void *PosixGraphicsStateGuardian::
get_system_func(const char *name) {
  if (_libgl_handle == (void *)NULL) {
    // We open the current executable, rather than naming a particular
    // library.  Presumably libGL.so (or whatever the library should
    // be called) is already available in the current executable
    // address space, so this is more portable than insisting on a
    // particular shared library name.
    _libgl_handle = dlopen(NULL, RTLD_LAZY);
    nassertr(_libgl_handle != (void *)NULL, NULL);

    // If that doesn't locate the symbol we expected, then fall back
    // to loading the GL library by its usual name.
    if (dlsym(_libgl_handle, name) == NULL) {
      dlclose(_libgl_handle);
      glxdisplay_cat.warning()
        << name << " not found in executable; looking in libGL.so instead.\n";
      _libgl_handle = dlopen("libGL.so", RTLD_LAZY);
      nassertr(_libgl_handle != (void *)NULL, NULL);
    }
  }

  return dlsym(_libgl_handle, name);
}
