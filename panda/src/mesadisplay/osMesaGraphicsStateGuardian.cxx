// Filename: osMesaGraphicsStateGuardian.cxx
// Created by:  drose (09Feb04)
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

#include "osMesaGraphicsStateGuardian.h"

TypeHandle OSMesaGraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OSMesaGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
OSMesaGraphicsStateGuardian::
OSMesaGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                            OSMesaGraphicsStateGuardian *share_with) : 
  MesaGraphicsStateGuardian(engine, pipe)
{
  OSMesaContext share_context = NULL;
  if (share_with != (OSMesaGraphicsStateGuardian *)NULL) {
    share_context = share_with->_context;
    _prepared_objects = share_with->get_prepared_objects();
  }

  _context = OSMesaCreateContext(OSMESA_RGBA, share_context);

  // OSMesa is never hardware-accelerated.
  _is_hardware = false;
}

////////////////////////////////////////////////////////////////////
//     Function: OSMesaGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
OSMesaGraphicsStateGuardian::
~OSMesaGraphicsStateGuardian() {
  if (_context != (OSMesaContext)NULL) {
    OSMesaDestroyContext(_context);
    _context = (OSMesaContext)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OSMesaGraphicsStateGuardian::do_get_extension_func
//       Access: Public, Virtual
//  Description: Returns the pointer to the GL extension function with
//               the indicated name.  It is the responsibility of the
//               caller to ensure that the required extension is
//               defined in the OpenGL runtime prior to calling this;
//               it is an error to call this for a function that is
//               not defined.
////////////////////////////////////////////////////////////////////
void *OSMesaGraphicsStateGuardian::
do_get_extension_func(const char *prefix, const char *name) {
#if (OSMESA_MAJOR_VERSION == 4 && OSMESA_MINOR_VERSION >= 1) || OSMESA_MAJOR_VERSION > 4
  // If we've got at least OSMesa version 4.1, then we can use
  // OSMesaGetProcAddress.

  // We ignore the prefix and always use "gl", since that's what Mesa
  // does (even if we compile with name mangling enabled to rename the
  // Mesa functions to "mgl", they're still stored as "gl" in the
  // OSMesaGetProcAddress() lookup table.
  string fullname = string("gl") + string(name);
  void *ptr = (void *)OSMesaGetProcAddress(fullname.c_str());
  if (mesadisplay_cat.is_debug()) {
    mesadisplay_cat.debug()
      << "Looking for function " << fullname << ": " << ptr << "\n";
  }
  if (ptr == (void *)NULL) {
    // Well, try for the more accurate name.
    fullname = string(prefix) + string(name);
    ptr = (void *)OSMesaGetProcAddress(fullname.c_str());
    if (mesadisplay_cat.is_debug()) {
      mesadisplay_cat.debug()
        << "Looking for function " << fullname << ": " << ptr << "\n";
    }
  }

  return ptr;

#else
  if (mesadisplay_cat.is_debug()) {
    mesadisplay_cat.debug()
      << "Couldn't look up extension function: compiled with Mesa version "
      << OSMESA_MAJOR_VERSION << "." << OSMESA_MINOR_VERSION << "\n";
  }
  
  // Otherwise, too bad.  No extension functions for you.  We could
  // try to write code that would dig around in the system interface
  // (using dlopen(), for instance) to find the extension functions,
  // but why should we have to do that?  Just go get the latest Mesa,
  // for goodness sakes!
  return NULL;
#endif
}
