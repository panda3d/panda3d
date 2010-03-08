// Filename: glxGraphicsPipe.cxx
// Created by:  mike (09Jan97)
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

#include "glxGraphicsPipe.h"
#include "glxGraphicsWindow.h"
#include "glxGraphicsBuffer.h"
#include "glxGraphicsPixmap.h"
#include "glxGraphicsStateGuardian.h"
#include "config_glxdisplay.h"
#include "frameBufferProperties.h"

TypeHandle glxGraphicsPipe::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
glxGraphicsPipe::
glxGraphicsPipe(const string &display) : x11GraphicsPipe(display) {
  string display_spec (XDisplayString(_display));
  
  int errorBase, eventBase;
  if (!glXQueryExtension(_display, &errorBase, &eventBase)) {
    glxdisplay_cat.error()
      << "OpenGL GLX extension not supported on display \"" << display_spec
      << "\".\n";
    return;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string glxGraphicsPipe::
get_interface_name() const {
  return "OpenGL";
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               glxGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) glxGraphicsPipe::
pipe_constructor() {
  return new glxGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) glxGraphicsPipe::
make_output(const string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {
  
  if (!_is_valid) {
    return NULL;
  }

  glxGraphicsStateGuardian *glxgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(glxgsg, gsg, NULL);
  }

  bool support_rtt;
  support_rtt = false;
  if (glxgsg) {
     support_rtt = 
      glxgsg -> get_supports_render_texture() && 
      support_render_texture;
  }  
  // First thing to try: a glxGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new glxGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }
  
  // Second thing to try: a GLGraphicsBuffer

  if (retry == 1) {
    if ((host==0)||
        (!gl_support_fbo)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return NULL;
    }
    // Early failure - if we are sure that this buffer WONT
    // meet specs, we can bail out early.
    int _fbo_multisample = 0;
    if (!ConfigVariableBool("framebuffer-object-multisample", false, PRC_DESC("Enabled Multisample."))) {
        _fbo_multisample = 16;
    }
    if ((flags & BF_fb_props_optional)==0) {
      if ((fb_prop.get_indexed_color() > 0)||
          (fb_prop.get_back_buffers() > 0)||
          (fb_prop.get_accum_bits() > 0)||
          (fb_prop.get_multisamples() > _fbo_multisample)) {
        return NULL;
      }
    }
    // Early success - if we are sure that this buffer WILL
    // meet specs, we can precertify it.
    if ((glxgsg != 0) &&
        (glxgsg->is_valid()) &&
        (!glxgsg->needs_reset()) &&
        (glxgsg->_supports_framebuffer_object) &&
        (glxgsg->_glDrawBuffers != 0)&&
        (fb_prop.is_basic())) {
      precertify = true;
    }
    return new GLGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                flags, gsg, host);
  }

  // Third thing to try: a glxGraphicsBuffer
  if (glxgsg == NULL || glxgsg->_supports_fbconfig) {
    if (retry == 2) {
      if (!glx_support_pbuffer) {
        return NULL;
      }
      
      if (((flags&BF_require_parasite)!=0)||
          ((flags&BF_require_window)!=0)||
          ((flags&BF_resizeable)!=0)||
          ((flags&BF_size_track_host)!=0)) {
        return NULL;
      }
      
      if (!support_rtt) {
        if (((flags&BF_rtt_cumulative)!=0)||
            ((flags&BF_can_bind_every)!=0)) {
          // If we require Render-to-Texture, but can't be sure we
          // support it, bail.
          return NULL;
        }
      }
      
      return new glxGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                   flags, gsg, host);
    }
  }

  // Third thing to try: a glxGraphicsPixmap.
  if (retry == 3) {
    if (!glx_support_pixmap) {
      return NULL;
    }

    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)) {
      return NULL;
    }

    if (((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }

    return new glxGraphicsPixmap(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }
  
  // Nothing else left to try.
  return NULL;
}
