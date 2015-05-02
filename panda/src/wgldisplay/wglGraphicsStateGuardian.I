// Filename: wglGraphicsStateGuardian.I
// Created by:  drose (27Jan03)
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

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_pfnum
//       Access: Public
//  Description: Returns the pixel format number chosen for windows
//               that use this context.  In OpenGL under Microsoft
//               Windows, the window must be created first and then
//               the GL context is created from the window, and the
//               context inherits the pixel format of the window.
//               Therefore, all windows that share a particular
//               context must also share the same pixel format.
////////////////////////////////////////////////////////////////////
INLINE int wglGraphicsStateGuardian::
get_pfnum() const {
  return _pfnum;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::pfnum_supports_pbuffer
//       Access: Public
//  Description: Returns true if the gsg's pixel format is capable
//               of supporting a pbuffer.
////////////////////////////////////////////////////////////////////
INLINE bool wglGraphicsStateGuardian::
pfnum_supports_pbuffer() const {
  return _pfnum_supports_pbuffer;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_fb_properties
//       Access: Public
//  Description: Returns the properties of the pixel format that
//               was chosen for this gsg.  In OpenGL under Microsoft
//               Windows, the window must be created first and then
//               the GL context is created from the window, and the
//               context inherits the pixel format of the window.
//               Therefore, all windows that share a particular
//               context must also share the same pixel format.
////////////////////////////////////////////////////////////////////
INLINE const FrameBufferProperties &wglGraphicsStateGuardian::
get_fb_properties() const {
  return _pfnum_properties;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::made_context
//       Access: Public
//  Description: Returns true if the GSG has had a context made for it
//               already, false otherwise.  If this returns false, the
//               next call to get_context() will quietly make a new
//               context.
////////////////////////////////////////////////////////////////////
INLINE bool wglGraphicsStateGuardian::
made_context() const {
  return _made_context;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_context
//       Access: Public
//  Description: Returns the GL context associated with the GSG.  If
//               the context has not yet been created, this creates a
//               suitable context for rendering to the indicated
//               window.  This means that this method may only be
//               called from within the draw thread.
////////////////////////////////////////////////////////////////////
INLINE HGLRC wglGraphicsStateGuardian::
get_context(HDC hdc) {
  if (!_made_context) {
    make_context(hdc);
  }
  return _context;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_twindow_dc
//       Access: Public
//  Description: Returns the DC associated with the temporary,
//               invisible window that was created with the gsg to
//               query WGL extensions.
////////////////////////////////////////////////////////////////////
INLINE HDC wglGraphicsStateGuardian::
get_twindow_dc() {
  if (_twindow_dc == 0) {
    make_twindow();
  }
  return _twindow_dc;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_supports_wgl_render_texture
//       Access: Published
//  Description: Returns true if this particular GSG can render
//               from a wglGraphicsBuffer directly into a texture, or
//               false if it must always copy-to-texture at the end of
//               each frame to achieve this effect.
////////////////////////////////////////////////////////////////////
INLINE bool wglGraphicsStateGuardian::
get_supports_wgl_render_texture() const {
  return _supports_wgl_render_texture;
}
