// Filename: wglGraphicsStateGuardian.cxx
// Created by:  drose (27Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "wglGraphicsStateGuardian.h"
#include "string_utils.h"

TypeHandle wglGraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsStateGuardian::
wglGraphicsStateGuardian(const FrameBufferProperties &properties,
                         int pfnum) : 
  GLGraphicsStateGuardian(properties),
  _pfnum(pfnum)
{
  _made_context = false;
  _context = (HGLRC)NULL;

  _supports_pbuffer = false;
  _supports_pixel_format = false;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsStateGuardian::
~wglGraphicsStateGuardian() {
  if (_context != (HGLRC)NULL) {
    wglDeleteContext(_context);
    _context = (HGLRC)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void wglGraphicsStateGuardian::
reset() {
  GLGraphicsStateGuardian::reset();

  // Output the vendor and version strings.
  show_gl_string("GL_VENDOR", GL_VENDOR);
  show_gl_string("GL_RENDERER", GL_RENDERER);
  show_gl_string("GL_VERSION", GL_VERSION);

  // Save the extensions tokens.
  save_extensions((const char *)glGetString(GL_EXTENSIONS));

  // Also save the tokens listed by wglGetExtensionsString.  This is a
  // little trickier, since the query function is itself an extension.
  typedef const GLubyte *(*wglGetExtensionsStringEXT_proc)(void);
  wglGetExtensionsStringEXT_proc wglGetExtensionsStringEXT = 
    (wglGetExtensionsStringEXT_proc)wglGetProcAddress("wglGetExtensionsStringEXT");
  if (wglGetExtensionsStringEXT != NULL) {
    save_extensions((const char *)wglGetExtensionsStringEXT());
  }

  if (wgldisplay_cat.is_debug()) {
    wgldisplay_cat.debug()
      << "GL Extensions:\n";
    pset<string>::const_iterator ei;
    for (ei = _extensions.begin(); ei != _extensions.end(); ++ei) {
      wgldisplay_cat.debug() << (*ei) << "\n";
    }
  }

  _supports_pbuffer = (_extensions.count("WGL_ARB_pbuffer") != 0);
  _supports_pixel_format = (_extensions.count("WGL_ARB_pixel_format") != 0);

  _wglCreatePbufferARB = 
    (wglCreatePbufferARB_proc)wglGetProcAddress("wglCreatePbufferARB");
  _wglGetPbufferDCARB = 
    (wglGetPbufferDCARB_proc)wglGetProcAddress("wglGetPbufferDCARB");
  _wglReleasePbufferDCARB = 
    (wglReleasePbufferDCARB_proc)wglGetProcAddress("wglReleasePbufferDCARB");
  _wglDestroyPbufferARB = 
    (wglDestroyPbufferARB_proc)wglGetProcAddress("wglDestroyPbufferARB");
  _wglQueryPbufferARB = 
    (wglQueryPbufferARB_proc)wglGetProcAddress("wglQueryPbufferARB");

  if (_supports_pbuffer) {
    if (_wglCreatePbufferARB == NULL ||
        _wglGetPbufferDCARB == NULL ||
        _wglReleasePbufferDCARB == NULL ||
        _wglDestroyPbufferARB == NULL ||
        _wglQueryPbufferARB == NULL) {
      wgldisplay_cat.error()
        << "Driver claims to support WGL_ARB_pbuffer extension, but does not define all functions.\n";
      _supports_pbuffer = false;
    }
  }

  _wglGetPixelFormatAttribivARB =
    (wglGetPixelFormatAttribivARB_proc)wglGetProcAddress("wglGetPixelFormatAttribivARB");
  _wglGetPixelFormatAttribfvARB =
    (wglGetPixelFormatAttribfvARB_proc)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
  _wglChoosePixelFormatARB =
    (wglChoosePixelFormatARB_proc)wglGetProcAddress("wglChoosePixelFormatARB");

  if (_supports_pixel_format) {
    if (_wglGetPixelFormatAttribivARB == NULL ||
        _wglGetPixelFormatAttribfvARB == NULL ||
        _wglChoosePixelFormatARB == NULL) {
      wgldisplay_cat.error()
        << "Driver claims to support WGL_ARB_pixel_format extension, but does not define all functions.\n";
      _supports_pixel_format = false;
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::make_context
//       Access: Private
//  Description: Creates a suitable context for rendering into the
//               given window.  This should only be called from the
//               draw thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsStateGuardian::
make_context(HDC hdc) {
  // We should only call this once for a particular GSG.
  nassertv(!_made_context);

  _made_context = true;

  // Attempt to create a context.
  _context = wglCreateContext(hdc);

  if (_context == NULL) {
    wgldisplay_cat.error()
      << "Could not create GL context.\n";
    return;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::show_gl_string
//       Access: Private
//  Description: Outputs the result of glGetString() on the indicated
//               tag.
////////////////////////////////////////////////////////////////////
void wglGraphicsStateGuardian::
show_gl_string(const string &name, GLenum id) {
  if (wgldisplay_cat.is_debug()) {
    const GLubyte *text = glGetString(id);
    if (text == (const GLubyte *)NULL) {
      wgldisplay_cat.debug()
        << "Unable to query " << name << "\n";
    } else {
      wgldisplay_cat.debug()
        << name << " = " << (const char *)text << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::save_extensions
//       Access: Private
//  Description: Separates the string returned by GL_EXTENSIONS or
//               wglGetExtensionsStringEXT into its individual tokens
//               and saves them in the _extensions member.
////////////////////////////////////////////////////////////////////
void wglGraphicsStateGuardian::
save_extensions(const char *extensions) {
  vector_string tokens;
  extract_words(extensions, tokens);

  vector_string::iterator ti;
  for (ti = tokens.begin(); ti != tokens.end(); ++ti) {
    _extensions.insert(*ti);
  }
}

