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

#include "osxGraphicsStateGuardian.h"
#include "osxGraphicsBuffer.h"
#include "string_utils.h"
#include "config_osxdisplay.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "textureAttrib.h"
#include "pnmImage.h"

#include <OpenGL/gl.h>
#import <mach-o/dyld.h>

// This is generated data for the standard texture we use for drawing
// the resize box in the window corner.
#include "resize_box.rgb.c"

TypeHandle osxGraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::do_get_extension_func
//       Access: Public, Virtual
//  Description: Returns the pointer to the GL extension function with
//               the indicated name.  It is the responsibility of the
//               caller to ensure that the required extension is
//               defined in the OpenGL runtime prior to calling this;
//               it is an error to call this for a function that is
//               not defined.
////////////////////////////////////////////////////////////////////
void *osxGraphicsStateGuardian::
do_get_extension_func(const char *name) {
  string fullname = "_" + string(name);
  NSSymbol symbol = NULL;

  if (NSIsSymbolNameDefined(fullname.c_str())) {
    symbol = NSLookupAndBindSymbol(fullname.c_str());
  }

  return symbol ? NSAddressOfSymbol(symbol) : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsStateGuardian::
osxGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                         osxGraphicsStateGuardian *share_with) :
  GLGraphicsStateGuardian(engine, pipe),
  _share_with(share_with),
  _aglPixFmt(NULL),
  _aglcontext(NULL)
{
  _shared_buffer = 1011;
  get_gamma_table();
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsStateGuardian::
~osxGraphicsStateGuardian() {
  if (_aglcontext != (AGLContext)NULL) {
    aglSetCurrentContext(NULL);
    aglDestroyContext(_aglcontext);
    report_agl_error("aglDestroyContext");
    _aglcontext = (AGLContext)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void osxGraphicsStateGuardian::reset()
{
/*
  if(_aglcontext != (AGLContext)NULL)
  {
     aglDestroyContext(_aglcontext);
     report_agl_error();
     _aglcontext = (AGLContext)NULL;
  }
  */

  GLGraphicsStateGuardian::reset();

  if (_aglcontext != (AGLContext)NULL) {
    // Apply the video-sync setting.
    GLint value = sync_video ? 1 : 0;
    aglSetInteger(_aglcontext, AGL_SWAP_INTERVAL, &value);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::draw_resize_box
//       Access: Public, Virtual
//  Description: Draws an OSX-style resize icon in the bottom right
//               corner of the current display region.  This is
//               normally done automatically at the end of each frame
//               when the window is indicated as resizable, since the
//               3-D graphics overlay the normal, OS-drawn resize icon
//               and the user won't be able see it.
////////////////////////////////////////////////////////////////////
void osxGraphicsStateGuardian::
draw_resize_box() {
  // This state is created, once, and never freed.
  static CPT(RenderState) state;
  if (state == (RenderState *)NULL) {
    state = RenderState::make(TransparencyAttrib::make(TransparencyAttrib::M_alpha),
                              DepthWriteAttrib::make(DepthWriteAttrib::M_off),
                              DepthTestAttrib::make(DepthTestAttrib::M_none));

    // Get the default texture to apply to the resize box; it's
    // compiled into the code.
    string resize_box_string((const char *)resize_box, resize_box_len);
    istringstream resize_box_strm(resize_box_string);
    PNMImage resize_box_pnm;
    if (resize_box_pnm.read(resize_box_strm, "resize_box.rgb")) {
      PT(Texture) tex = new Texture;
      tex->set_name("resize_box.rgb");
      tex->load(resize_box_pnm);
      tex->set_minfilter(SamplerState::FT_linear);
      tex->set_magfilter(SamplerState::FT_linear);
      state = state->add_attrib(TextureAttrib::make(tex));
    }
  }

  // Clear out the lens.
  _projection_mat_inv = _projection_mat = TransformState::make_identity();
  prepare_lens();

  // Set the state to our specific, known state for drawing the icon.
  set_state_and_transform(state, TransformState::make_identity());

  // Now determine the inner corner of the quad, choosing a 15x15
  // pixel square in the lower-right corner, computed from the
  // viewport size.
  PN_stdfloat inner_x = 1.0f - (15.0f * 2.0f / _viewport_width);
  PN_stdfloat inner_y = (15.0f * 2.0f / _viewport_height) - 1.0f;

  // Draw the quad.  We just use the slow, simple immediate mode calls
  // here.  It's just one quad, after all.
  glBegin(GL_QUADS);

  glColor4f(1.0, 1.0, 1.0, 1.0);
  glTexCoord2f(0.0, 0.0);
  glVertex2f(inner_x, -1.0);

  glTexCoord2f(0.9375, 0.0);
  glVertex2f(1.0, -1.0);

  glTexCoord2f(0.9375, 0.9375);
  glVertex2f(1.0, inner_y);

  glTexCoord2f(0.0, 0.9375);
  glVertex2f(inner_x, inner_y);

  glEnd();
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::build_gl
//       Access: Public, Virtual
//  Description: This function will build up a context for a gsg..
////////////////////////////////////////////////////////////////////
OSStatus osxGraphicsStateGuardian::
build_gl(bool full_screen, bool pbuffer, FrameBufferProperties &fb_props) {
  if (_aglcontext) {
    describe_pixel_format(fb_props);
    return noErr; // already built
  }

  OSStatus err = noErr;

  GDHandle display = GetMainDevice();

  pvector<GLint> attrib;
  if (!fb_props.get_indexed_color()) {
    attrib.push_back(AGL_RGBA);
    int color_bits = fb_props.get_color_bits();
    int alpha_bits = fb_props.get_alpha_bits();
    attrib.push_back(AGL_BUFFER_SIZE);
    attrib.push_back(color_bits + alpha_bits);
    attrib.push_back(AGL_PIXEL_SIZE);
    attrib.push_back(color_bits);
    attrib.push_back(AGL_RED_SIZE);
    attrib.push_back(fb_props.get_red_bits());
    attrib.push_back(AGL_GREEN_SIZE);
    attrib.push_back(fb_props.get_green_bits());
    attrib.push_back(AGL_BLUE_SIZE);
    attrib.push_back(fb_props.get_blue_bits());
    attrib.push_back(AGL_ALPHA_SIZE);
    attrib.push_back(alpha_bits);
  }
  attrib.push_back(AGL_DEPTH_SIZE);
  attrib.push_back(fb_props.get_depth_bits());
  attrib.push_back(AGL_STENCIL_SIZE);
  attrib.push_back(fb_props.get_stencil_bits());
  if (fb_props.get_multisamples() != 0) {
    attrib.push_back(AGL_MULTISAMPLE);
    attrib.push_back(AGL_SAMPLE_BUFFERS_ARB);
    attrib.push_back(1);
    attrib.push_back(AGL_SAMPLES_ARB);
    attrib.push_back(fb_props.get_multisamples());
  }

  if (fb_props.is_stereo()) {
    attrib.push_back(AGL_STEREO);
  }

  if (!fb_props.is_single_buffered()) {
    attrib.push_back(AGL_DOUBLEBUFFER);
  }
  if (full_screen) {
    attrib.push_back(AGL_FULLSCREEN);
  }
  if (pbuffer) {
    attrib.push_back(AGL_PBUFFER);
  }

  if (fb_props.get_force_hardware()) {
    attrib.push_back(AGL_ACCELERATED);
    attrib.push_back(AGL_NO_RECOVERY);
  }

  // Allow the system to choose the largest buffers requested that
  // meets all our selections.
  attrib.push_back(AGL_MAXIMUM_POLICY);

  // Terminate the list.
  attrib.push_back(AGL_NONE);

  // build context
  _aglcontext = NULL;
  _aglPixFmt = aglChoosePixelFormat(&display, 1, &attrib[0]);
  err = report_agl_error("aglChoosePixelFormat");
  if (_aglPixFmt) {
    if(_share_with == NULL) {
      _aglcontext = aglCreateContext(_aglPixFmt, NULL);
    } else {
      _aglcontext = aglCreateContext(_aglPixFmt, ((osxGraphicsStateGuardian *)_share_with)->_aglcontext);
    }
    err = report_agl_error("aglCreateContext");

    if (_aglcontext == NULL) {
      osxdisplay_cat.error()
        << "osxGraphicsStateGuardian::build_gl Error Getting GL Context \n" ;
      if(err == noErr) {
        err = -1;
      }
    } else {
      aglSetInteger(_aglcontext, AGL_BUFFER_NAME, &_shared_buffer);
      err = report_agl_error("aglSetInteger AGL_BUFFER_NAME");
    }

  } else {
    osxdisplay_cat.error()
      << "osxGraphicsStateGuardian::build_gl Error Getting Pixel Format\n" ;
    osxdisplay_cat.error()
      << fb_props << "\n";
    if(err == noErr) {
      err = -1;
    }
  }

  if (err == noErr) {
    describe_pixel_format(fb_props);
  }

  if (osxdisplay_cat.is_debug()) {
    osxdisplay_cat.debug()
      << "osxGraphicsStateGuardian::build_gl Returning :" << err << "\n";
    osxdisplay_cat.debug()
      << fb_props << "\n";
  }

  return err;
}


////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::describe_pixel_format
//       Access: Private
//  Description: Fills in the fb_props member with the appropriate
//               values according to the chosen pixel format.
////////////////////////////////////////////////////////////////////
void osxGraphicsStateGuardian::
describe_pixel_format(FrameBufferProperties &fb_props) {
  fb_props.clear();
  GLint value;

  if (aglDescribePixelFormat(_aglPixFmt, AGL_RGBA, &value)) {
    fb_props.set_indexed_color(!value);
    fb_props.set_rgb_color(value);
  }
  if (aglDescribePixelFormat(_aglPixFmt, AGL_DEPTH_SIZE, &value)) {
    fb_props.set_depth_bits(value);
  }
  int color_bits = 0;
  if (aglDescribePixelFormat(_aglPixFmt, AGL_RED_SIZE, &value)) {
    fb_props.set_red_bits(value);
    color_bits += value;
  }
  if (aglDescribePixelFormat(_aglPixFmt, AGL_GREEN_SIZE, &value)) {
    fb_props.set_green_bits(value);
    color_bits += value;
  }
  if (aglDescribePixelFormat(_aglPixFmt, AGL_BLUE_SIZE, &value)) {
    fb_props.set_blue_bits(value);
    color_bits += value;
  }
  fb_props.set_color_bits(color_bits);
  if (aglDescribePixelFormat(_aglPixFmt, AGL_ALPHA_SIZE, &value)) {
    fb_props.set_alpha_bits(value);
  }

  if (aglDescribePixelFormat(_aglPixFmt, AGL_STENCIL_SIZE, &value)) {
    fb_props.set_stencil_bits(value);
  }

  int accum_bits = 0;
  if (aglDescribePixelFormat(_aglPixFmt, AGL_ACCUM_RED_SIZE, &value)) {
    accum_bits += value;
  }
  if (aglDescribePixelFormat(_aglPixFmt, AGL_ACCUM_GREEN_SIZE, &value)) {
    accum_bits += value;
  }
  if (aglDescribePixelFormat(_aglPixFmt, AGL_ACCUM_BLUE_SIZE, &value)) {
    accum_bits += value;
  }

  if (aglDescribePixelFormat(_aglPixFmt, AGL_SAMPLES_ARB, &value)) {
    fb_props.set_multisamples(value);
  }

  if (aglDescribePixelFormat(_aglPixFmt, AGL_DOUBLEBUFFER, &value)) {
    if (value) {
      fb_props.set_back_buffers(1);
    } else {
      fb_props.set_back_buffers(0);
    }
  }

  if (aglDescribePixelFormat(_aglPixFmt, AGL_STEREO, &value)) {
    fb_props.set_stereo(value);
  }

  // Until we query the renderer, we don't know whether it's hardware
  // or software based, so set both flags to indicate we don't know.
  fb_props.set_force_hardware(true);
  fb_props.set_force_software(true);

  GLint ndevs;
  AGLDevice *gdevs = aglDevicesOfPixelFormat(_aglPixFmt, &ndevs);
  if (gdevs != (AGLDevice *)NULL) {
    AGLRendererInfo rinfo = aglQueryRendererInfo(gdevs, ndevs);
    if (rinfo != NULL) {
      if (aglDescribeRenderer(rinfo, AGL_ACCELERATED, &value)) {
        // Now we know whether it's hardware or software.
        fb_props.set_force_hardware(value);
        fb_props.set_force_software(!value);
      }
      if (aglDescribeRenderer(rinfo, AGL_VIDEO_MEMORY, &value)) {
        osxdisplay_cat.debug()
          << "Reported video memory is " << value << "\n";
      }
      if (aglDescribeRenderer(rinfo, AGL_TEXTURE_MEMORY, &value)) {
        osxdisplay_cat.debug()
          << "Reported texture memory is " << value << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::get_gamma_table
//       Access: Public, Static
//  Description: Static function for getting the orig gamma tables
////////////////////////////////////////////////////////////////////
bool osxGraphicsStateGuardian::
get_gamma_table() {
  CGDisplayRestoreColorSyncSettings();
  _cgErr = CGGetDisplayTransferByTable( 0, 256, _gOriginalRedTable, _gOriginalGreenTable, _gOriginalBlueTable, &_sampleCount);
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::static_set_gamma
//       Access: Public, Static
//  Description: Static function for setting gamma which is needed
//               for atexit.
////////////////////////////////////////////////////////////////////
bool osxGraphicsStateGuardian::
static_set_gamma(bool restore, PN_stdfloat gamma) {
  bool set;

  set = false;

  if (restore) {
    CGDisplayRestoreColorSyncSettings();
    set = true;
    return set;
  }
  // CGDisplayRestoreColorSyncSettings();

  // CGGammaValue gOriginalRedTable[ 256 ];
  // CGGammaValue gOriginalGreenTable[ 256 ];
  // CGGammaValue gOriginalBlueTable[ 256 ];

  // CGTableCount sampleCount;
  // CGDisplayErr cgErr;

  // cgErr = CGGetDisplayTransferByTable( 0, 256, _gOriginalRedTable, _gOriginalGreenTable, _gOriginalBlueTable, &_sampleCount);

  CGGammaValue redTable[ 256 ];
  CGGammaValue greenTable[ 256 ];
  CGGammaValue blueTable[ 256 ];

  short j, i;
  short y[3];

  for (j = 0; j < 3; j++) {
    y[j] = 255;
  }

  y[0] = 256 * gamma;
  y[1] = 256 * gamma;
  y[2] = 256 * gamma;

  for (i = 0; i < 256; i++) {
    redTable[i] = _gOriginalRedTable[ i ] * (y[ 0 ] ) / 256;
    greenTable[ i ] = _gOriginalGreenTable[ i ] * (y[ 1 ] ) / 256;
    blueTable[ i ] = _gOriginalBlueTable[ i ] * (y[ 2 ] ) / 256;
  }
  _cgErr = CGSetDisplayTransferByTable( 0, 256, redTable, greenTable, blueTable);

  if (_cgErr == 0) {
    set = true;
  }

  return set;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::set_gamma
//       Access: Published
//  Description: Non static version of setting gamma.  Returns true
//               on success.
////////////////////////////////////////////////////////////////////
bool osxGraphicsStateGuardian::
set_gamma(PN_stdfloat gamma) {
  bool set;

  set = static_set_gamma(false, gamma);

  return set;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::restore_gamma
//       Access: Published
//  Description: Restore original gamma.
////////////////////////////////////////////////////////////////////
void osxGraphicsStateGuardian::
restore_gamma() {
  static_set_gamma(true, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::atexit_function
//       Access: Public, Static
//  Description: This function is passed to the atexit function.
////////////////////////////////////////////////////////////////////
void osxGraphicsStateGuardian::
atexit_function() {
  static_set_gamma(true, 1.0);
}


