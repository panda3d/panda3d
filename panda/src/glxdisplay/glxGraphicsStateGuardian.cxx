// Filename: glxGraphicsStateGuardian.cxx
// Created by:  drose (27Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "glxGraphicsStateGuardian.h"
#include "config_glxdisplay.h"
#include "config_glgsg.h"

#include <dlfcn.h>


TypeHandle glxGraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsStateGuardian::
glxGraphicsStateGuardian(const FrameBufferProperties &properties,
                         glxGraphicsStateGuardian *share_with,
                         GLXContext context, XVisualInfo *visual, 
                         Display *display, int screen
#ifdef HAVE_GLXFBCONFIG
                         , GLXFBConfig fbconfig
#endif  // HAVE_GLXFBCONFIG
                         ) :
  GLGraphicsStateGuardian(properties),
  _context(context),
  _visual(visual),
  _display(display),
  _screen(screen)
#ifdef HAVE_GLXFBCONFIG
  , _fbconfig(fbconfig)
#endif  // HAVE_GLXFBCONFIG
{
  if (share_with != (glxGraphicsStateGuardian *)NULL) {
    _prepared_objects = share_with->get_prepared_objects();
  }
  
  _libgl_handle = NULL;
  _checked_get_proc_address = false;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsStateGuardian::
~glxGraphicsStateGuardian() {
  if (_visual != (XVisualInfo *)NULL) {
    XFree(_visual);
  }
  if (_context != (GLXContext)NULL) {
    glXDestroyContext(_display, _context);
    _context = (GLXContext)NULL;
  }
  if (_libgl_handle != (void *)NULL) {
    dlclose(_libgl_handle);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsStateGuardian::glx_is_at_least_version
//       Access: Public
//  Description: Returns true if the runtime GLX version number is at
//               least the indicated value, false otherwise.
////////////////////////////////////////////////////////////////////
bool glxGraphicsStateGuardian::
glx_is_at_least_version(int major_version, int minor_version) const {
  if (_glx_version_major < major_version) {
    return false;
  }
  if (_glx_version_minor < minor_version) {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsStateGuardian::get_gl_version
//       Access: Protected, Virtual
//  Description: Queries the runtime version of OpenGL in use.
////////////////////////////////////////////////////////////////////
void glxGraphicsStateGuardian::
get_gl_version() {
  GLGraphicsStateGuardian::get_gl_version();

  show_glx_client_string("GLX_VENDOR", GLX_VENDOR);
  show_glx_client_string("GLX_VERSION", GLX_VERSION);
  show_glx_server_string("GLX_VENDOR", GLX_VENDOR);
  show_glx_server_string("GLX_VERSION", GLX_VERSION);

  glXQueryVersion(_display, &_glx_version_major, &_glx_version_minor);

  // We output to glgsg_cat instead of glxdisplay_cat, since this is
  // where the GL version has been output, and it's nice to see the
  // two of these together.
  glgsg_cat.debug()
    << "GLX_VERSION = " << _glx_version_major << "." << _glx_version_minor 
    << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsStateGuardian::get_extra_extensions
//       Access: Protected, Virtual
//  Description: This may be redefined by a derived class (e.g. glx or
//               wgl) to get whatever further extensions strings may
//               be appropriate to that interface, in addition to the
//               GL extension strings return by glGetString().
////////////////////////////////////////////////////////////////////
void glxGraphicsStateGuardian::
get_extra_extensions() {
  save_extensions(glXQueryExtensionsString(_display, _screen));
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsStateGuardian::get_extension_func
//       Access: Public, Virtual
//  Description: Returns the pointer to the GL extension function with
//               the indicated name.  It is the responsibility of the
//               caller to ensure that the required extension is
//               defined in the OpenGL runtime prior to calling this;
//               it is an error to call this for a function that is
//               not defined.
////////////////////////////////////////////////////////////////////
void *glxGraphicsStateGuardian::
get_extension_func(const char *prefix, const char *name) {
  if (!_checked_get_proc_address) {
    // First, check if we have glxGetProcAddress available.  This will
    // be superior if we can get it.
    const char *funcName = NULL;

    if (glx_is_at_least_version(1, 4)) {
      funcName = "glXGetProcAddress";

    } else if (has_extension("GLX_ARB_get_proc_address")) {
      funcName = "glXGetProcAddressARB";
    }

    if (funcName != NULL) {
      _glxGetProcAddress = (PFNGLXGETPROCADDRESSPROC)get_system_func(funcName);
      if (_glxGetProcAddress == NULL) {
        glxdisplay_cat.warning()
          << "Couldn't load function " << funcName
          << ", GL extensions may be unavailable.\n";
      }
    }

    _checked_get_proc_address = true;
  }

  string fullname = string(prefix) + string(name);

  // Use glxGetProcAddress() if we've got it; it should be more robust.
  if (_glxGetProcAddress != NULL) {
    return (void *)_glxGetProcAddress((const GLubyte *)fullname.c_str());
  }

  // Otherwise, fall back to the OS-provided calls.
  return get_system_func(fullname.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsStateGuardian::get_system_func
//       Access: Private
//  Description: Support for get_extension_func(), above, that uses
//               system calls to find a GL or GLX function (in the
//               absence of a working glxGetProcAddress() function to
//               call).
////////////////////////////////////////////////////////////////////
void *glxGraphicsStateGuardian::
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

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsStateGuardian::show_glx_client_string
//       Access: Protected
//  Description: Outputs the result of glxGetClientString() on the
//               indicated tag.
////////////////////////////////////////////////////////////////////
void glxGraphicsStateGuardian::
show_glx_client_string(const string &name, int id) {
  if (glgsg_cat.is_debug()) {
    const char *text = glXGetClientString(_display, id);
    if (text == (const char *)NULL) {
      glgsg_cat.debug()
        << "Unable to query " << name << " (client)\n";
    } else {
      glgsg_cat.debug()
        << name << " (client) = " << (const char *)text << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsStateGuardian::show_glx_server_string
//       Access: Protected
//  Description: Outputs the result of glxQueryServerString() on the
//               indicated tag.
////////////////////////////////////////////////////////////////////
void glxGraphicsStateGuardian::
show_glx_server_string(const string &name, int id) {
  if (glgsg_cat.is_debug()) {
    const char *text = glXQueryServerString(_display, _screen, id);
    if (text == (const char *)NULL) {
      glgsg_cat.debug()
        << "Unable to query " << name << " (server)\n";
    } else {
      glgsg_cat.debug()
        << name << " (server) = " << (const char *)text << "\n";
    }
  }
}
