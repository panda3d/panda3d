// Filename: wglGraphicsBuffer.cxx
// Created by:  drose (08Feb04)
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

#include "wglGraphicsBuffer.h"
#include "wglGraphicsPipe.h"
#include "config_wgldisplay.h"
#include "glgsg.h"
#include "pStatTimer.h"

#include <wingdi.h>

TypeHandle wglGraphicsBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsBuffer::
wglGraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                  const string &name,
                  int x_size, int y_size, bool want_texture) :
  GraphicsBuffer(pipe, gsg, name, x_size, y_size, want_texture) 
{
  _pbuffer = (HPBUFFERARB)0;
  _pbuffer_dc = (HDC)0;

  // Since the pbuffer never gets flipped, we get screenshots from the
  // same buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsBuffer::
~wglGraphicsBuffer() {
}
 
////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
begin_frame() {
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);

  if (_pbuffer_dc) {
    int flag = 0;
    wglgsg->_wglQueryPbufferARB(_pbuffer, WGL_PBUFFER_LOST_ARB, &flag);
    if (flag != 0) {
      // The pbuffer was lost, due to a mode change or something
      // silly like that.  We must therefore recreate the pbuffer.
      close_buffer();
      if (!open_buffer()) {
        return false;
      }
    }
  }

  return GraphicsBuffer::begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
end_frame() {
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);
  _gsg->end_frame();

  if (_copy_texture) {
    wglGraphicsStateGuardian *wglgsg;
    DCAST_INTO_V(wglgsg, _gsg);

    // If we've lost the pbuffer image (due to a mode-switch, for
    // instance), don't attempt to copy it to the texture, since the
    // frame is invalid.  In fact, now we need to recreate the
    // pbuffer.
    if (_pbuffer_dc) {
      int flag = 0;
      wglgsg->_wglQueryPbufferARB(_pbuffer, WGL_PBUFFER_LOST_ARB, &flag);
      if (flag != 0) {
        wgldisplay_cat.info()
          << "Pbuffer contents lost.\n";
        return;
      }
    }

    // For now, we copy the framebuffer to the texture every frame.
    // Eventually we can take advantage of the render_texture
    // extension, if it is available, to render directly into a
    // texture in the first place (but I don't have a card that
    // supports that right now).
    nassertv(has_texture());

    RenderBuffer buffer = _gsg->get_render_buffer(get_draw_buffer_type());
    _gsg->copy_texture(get_texture(), _default_display_region, buffer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
make_current() {
  PStatTimer timer(_make_current_pcollector);

  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_V(wglgsg, _gsg);

  wglMakeCurrent(_pbuffer_dc, wglgsg->get_context(_pbuffer_dc));
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
release_gsg() {
  GraphicsBuffer::release_gsg();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties()
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
process_events() {
  GraphicsBuffer::process_events();

  MSG msg;
    
  // Handle all the messages on the queue in a row.  Some of these
  // might be for another window, but they will get dispatched
  // appropriately.
  while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
    process_1_event();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
close_buffer() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    wglGraphicsStateGuardian *wglgsg;
    DCAST_INTO_V(wglgsg, _gsg);

    if (_pbuffer_dc) {
      wglgsg->_wglReleasePbufferDCARB(_pbuffer, _pbuffer_dc);
    }
    if (_pbuffer) {
      wglgsg->_wglDestroyPbufferARB(_pbuffer);
    }
  }
  _pbuffer_dc = 0;
  _pbuffer = 0;

  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
open_buffer() {
  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);

  HDC twindow_dc = wglgsg->get_twindow_dc();
  if (twindow_dc == 0) {
    // If we couldn't make a window, we can't get a GL context.
    return false;
  }

  wglMakeCurrent(twindow_dc, wglgsg->get_context(twindow_dc));
  wglgsg->reset_if_new();
  _needs_context = false;

  // Now that we have fully made a window and used that window to
  // create a rendering context, we can attempt to create a pbuffer.
  // This might fail if the pbuffer extensions are not supported.

  if (!make_pbuffer(twindow_dc)) {
    wglMakeCurrent(0, 0);
    return false;
  }

  _pbuffer_dc = wglgsg->_wglGetPbufferDCARB(_pbuffer);
  if (wgldisplay_cat.is_debug()) {
    wgldisplay_cat.debug()
      << "Created PBuffer " << _pbuffer << ", DC " << _pbuffer_dc << "\n";
  }
  
  wglMakeCurrent(_pbuffer_dc, wglgsg->get_context(_pbuffer_dc));
  wglgsg->report_my_gl_errors();
  
  _is_valid = true;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::make_pbuffer
//       Access: Private
//  Description: Once the GL context has been fully realized, attempts
//               to create an offscreen pbuffer if the graphics API
//               supports it.  Returns true if successful, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
make_pbuffer(HDC twindow_dc) {
  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);

  if (!wglgsg->_supports_pbuffer) {
    wgldisplay_cat.info()
      << "PBuffers not supported by GL implementation.\n";
    return false;
  }

  int pbformat = wglgsg->get_pfnum();

  if (wglgsg->_supports_pixel_format) {
    // Select a suitable pixel format that matches the GSG's existing
    // format, and also is appropriate for a pixel buffer.

    static const int max_attrib_list = 32;
    int iattrib_list[max_attrib_list];
    int ivalue_list[max_attrib_list];
    int ni = 0;

    int acceleration_i, pixel_type_i, double_buffer_i, stereo_i,
      red_bits_i, green_bits_i, blue_bits_i, alpha_bits_i, 
      accum_red_bits_i, accum_green_bits_i, accum_blue_bits_i,
      accum_alpha_bits_i, depth_bits_i, 
      stencil_bits_i, sample_buffers_i, multisamples_i;

    iattrib_list[acceleration_i = ni++] = WGL_ACCELERATION_ARB;
    iattrib_list[pixel_type_i = ni++] = WGL_PIXEL_TYPE_ARB;
    iattrib_list[double_buffer_i = ni++] = WGL_DOUBLE_BUFFER_ARB;
    iattrib_list[stereo_i = ni++] = WGL_STEREO_ARB;
    iattrib_list[red_bits_i = ni++] = WGL_RED_BITS_ARB;
    iattrib_list[green_bits_i = ni++] = WGL_GREEN_BITS_ARB;
    iattrib_list[blue_bits_i = ni++] = WGL_BLUE_BITS_ARB;
    iattrib_list[alpha_bits_i = ni++] = WGL_ALPHA_BITS_ARB;
    iattrib_list[accum_red_bits_i = ni++] = WGL_ACCUM_RED_BITS_ARB;
    iattrib_list[accum_green_bits_i = ni++] = WGL_ACCUM_GREEN_BITS_ARB;
    iattrib_list[accum_blue_bits_i = ni++] = WGL_ACCUM_BLUE_BITS_ARB;
    iattrib_list[accum_alpha_bits_i = ni++] = WGL_ACCUM_ALPHA_BITS_ARB;
    iattrib_list[depth_bits_i = ni++] = WGL_DEPTH_BITS_ARB;
    iattrib_list[stencil_bits_i = ni++] = WGL_STENCIL_BITS_ARB;

    if (wglgsg->_supports_wgl_multisample) {
      iattrib_list[sample_buffers_i = ni++] = WGL_SAMPLE_BUFFERS_ARB;
      iattrib_list[multisamples_i = ni++] = WGL_SAMPLES_ARB;
    }

    // Terminate the list.
    nassertr(ni <= max_attrib_list, false);

    if (!wglgsg->_wglGetPixelFormatAttribivARB(twindow_dc, pbformat, 0,
                                               ni, iattrib_list, ivalue_list)) {
      return false;
    }

    ni = 0;
    float fattrib_list[max_attrib_list];
    int nf = 0;

    // Since we are trying to create a pbuffer, the pixel format we
    // request (and subsequently use) must be "pbuffer capable".
    iattrib_list[ni++] = WGL_DRAW_TO_PBUFFER_ARB;
    iattrib_list[ni++] = true;
    iattrib_list[ni++] = WGL_SUPPORT_OPENGL_ARB;
    iattrib_list[ni++] = true;

    // Match up the framebuffer bits.
    iattrib_list[ni++] = WGL_RED_BITS_ARB;
    iattrib_list[ni++] = ivalue_list[red_bits_i];
    iattrib_list[ni++] = WGL_GREEN_BITS_ARB;
    iattrib_list[ni++] = ivalue_list[green_bits_i];
    iattrib_list[ni++] = WGL_BLUE_BITS_ARB;
    iattrib_list[ni++] = ivalue_list[blue_bits_i];
    iattrib_list[ni++] = WGL_ALPHA_BITS_ARB;
    iattrib_list[ni++] = ivalue_list[alpha_bits_i];

    iattrib_list[ni++] = WGL_ACCUM_RED_BITS_ARB;
    iattrib_list[ni++] = ivalue_list[accum_red_bits_i];
    iattrib_list[ni++] = WGL_ACCUM_GREEN_BITS_ARB;
    iattrib_list[ni++] = ivalue_list[accum_green_bits_i];
    iattrib_list[ni++] = WGL_ACCUM_BLUE_BITS_ARB;
    iattrib_list[ni++] = ivalue_list[accum_blue_bits_i];
    iattrib_list[ni++] = WGL_ACCUM_ALPHA_BITS_ARB;
    iattrib_list[ni++] = ivalue_list[accum_alpha_bits_i];

    iattrib_list[ni++] = WGL_DEPTH_BITS_ARB;
    iattrib_list[ni++] = ivalue_list[depth_bits_i];

    iattrib_list[ni++] = WGL_STENCIL_BITS_ARB;
    iattrib_list[ni++] = ivalue_list[stencil_bits_i];

    if (wglgsg->_supports_wgl_multisample) {
      iattrib_list[ni++] = WGL_SAMPLE_BUFFERS_ARB;
      iattrib_list[ni++] = ivalue_list[sample_buffers_i];
      iattrib_list[ni++] = WGL_SAMPLES_ARB;
      iattrib_list[ni++] = ivalue_list[multisamples_i];
    }

    // Match up properties.
    iattrib_list[ni++] = WGL_DOUBLE_BUFFER_ARB;
    iattrib_list[ni++] = ivalue_list[double_buffer_i];
    iattrib_list[ni++] = WGL_STEREO_ARB;
    iattrib_list[ni++] = ivalue_list[stereo_i];

    // Terminate the lists.
    nassertr(ni < max_attrib_list && nf < max_attrib_list, NULL);
    iattrib_list[ni] = 0;
    fattrib_list[nf] = 0;

    // Now obtain a list of pixel formats that meet these minimum
    // requirements.
    static const unsigned int max_pformats = 32;
    int pformat[max_pformats];
    memset(pformat, 0, sizeof(pformat));
    unsigned int nformats = 0;
    if (!wglgsg->_wglChoosePixelFormatARB(twindow_dc, iattrib_list, fattrib_list,
                                          max_pformats, pformat, &nformats)
        || nformats == 0) {
      wgldisplay_cat.info()
        << "Couldn't find a suitable pixel format for creating a pbuffer.\n";
      return false;
    }

    nformats = min(nformats, max_pformats);

    if (wgldisplay_cat.is_debug()) {
      wgldisplay_cat.debug()
        << "Found " << nformats << " pbuffer formats: [";
      for (unsigned int i = 0; i < nformats; i++) {
        wgldisplay_cat.debug(false)
          << " " << pformat[i];
      }
      wgldisplay_cat.debug(false)
        << " ]\n";
    }

    // If one of the options is the original pixfmt, keep it.
    bool found_pbformat = false;
    for (unsigned int i = 0; i < nformats && !found_pbformat; i++) {
      if (pformat[i] == pbformat) {
        found_pbformat = true;
      }
    }

    if (!found_pbformat) {
      // Otherwise, pick any of them.
      pbformat = pformat[0];
    }
  }

  if (wgldisplay_cat.is_debug()) {
    wgldisplay_cat.debug()
      << "Chose pixfmt #" << pbformat << " for pbuffer\n";
  }
  
  int attrib_list[] = {
    0,
  };
  
  _pbuffer = wglgsg->_wglCreatePbufferARB(twindow_dc, pbformat, 
                                          _x_size, _y_size, attrib_list);

  if (_pbuffer == 0) {
    wgldisplay_cat.info()
      << "Attempt to create pbuffer failed.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::process_1_event
//       Access: Private, Static
//  Description: Handles one event from the message queue.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
process_1_event() {
  MSG msg;

  if (!GetMessage(&msg, NULL, 0, 0)) {
    // WM_QUIT received.  We need a cleaner way to deal with this.
    //    DestroyAllWindows(false);
    exit(msg.wParam);  // this will invoke AtExitFn
  }

  // Translate virtual key messages
  TranslateMessage(&msg);
  // Call window_proc
  DispatchMessage(&msg);
}


