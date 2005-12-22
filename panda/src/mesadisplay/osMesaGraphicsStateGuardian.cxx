// Filename: osMesaGraphicsStateGuardian.cxx
// Created by:  drose (09Feb04)
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

#include "osMesaGraphicsStateGuardian.h"

TypeHandle OSMesaGraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OSMesaGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
OSMesaGraphicsStateGuardian::
OSMesaGraphicsStateGuardian(const FrameBufferProperties &properties,
                            OSMesaGraphicsStateGuardian *share_with) : 
  MesaGraphicsStateGuardian(properties)
{
  OSMesaContext share_context = NULL;
  if (share_with != (OSMesaGraphicsStateGuardian *)NULL) {
    share_context = share_with->_context;
    _prepared_objects = share_with->get_prepared_objects();
  }

  _context = OSMesaCreateContext(OSMESA_RGBA, share_context);
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
//     Function: OSMesaGraphicsStateGuardian::get_extension_func
//       Access: Public, Virtual
//  Description: Returns the pointer to the GL extension function with
//               the indicated name.  It is the responsibility of the
//               caller to ensure that the required extension is
//               defined in the OpenGL runtime prior to calling this;
//               it is an error to call this for a function that is
//               not defined.
////////////////////////////////////////////////////////////////////
void *OSMesaGraphicsStateGuardian::
get_extension_func(const char *, const char *name) {
#if (OSMESA_MAJOR_VERSION == 4 && OSMESA_MINOR_VERSION >= 1) || OSMESA_MAJOR_VERSION > 4
  // If we've got at least OSMesa version 4.1, then we can use
  // OSMesaGetProcAddress.

  // We ignore the prefix and always use "gl", since that's what Mesa
  // does (even if we compile with name mangling enabled to rename the
  // Mesa functions to "mgl", they're still stored as "gl" in the
  // OSMesaGetProcAddress() lookup table.
  string fullname = string("gl") + string(name);
  return (void *)OSMesaGetProcAddress(fullname.c_str());

#else
  // Otherwise, too bad.  No extension functions for you.  We could
  // try to write code that would dig around in the system interface
  // (using dlopen(), for instance) to find the extension functions,
  // but why should we have to do that?  Just go get the latest Mesa,
  // for goodness sakes!
  return NULL;
#endif
}
