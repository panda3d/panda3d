/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file EAGLGraphicsStateGuardian.mm
 * @author D. Lawrence
 * @date 2019-01-03
 */

#include "eaglGraphicsStateGuardian.h"
#include <dlfcn.h>

TypeHandle EAGLGraphicsStateGuardian::_type_handle;

/**
 *
 */
EAGLGraphicsStateGuardian::
EAGLGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                          EAGLGraphicsStateGuardian *share_with) :
GLES2GraphicsStateGuardian(engine, pipe),
_shared_gsg(share_with) {

}

/**
 *
 */
// EAGLGraphicsStateGuardian::
// ~EAGLGraphicsStateGuardian() {
  
// }

/**
 * Creates a GLES context and tries to set the requested properties. When
 * this GSG is associated with a EAGLGraphicsWindow, we set our color to one of
 * the three modes: SRGBA8, RGBA8, and RGB565. Otherwise, we just pass the
 * requested properties along to GLES2GraphicsStateGuardian, since there is no
 * specific mechanism to check if certain properties are supported.
 */
void EAGLGraphicsStateGuardian::
choose_pixel_format(const FrameBufferProperties &properties,
                    CAEAGLLayer *layer) {
  _fbprops.clear();
  _fbprops.add_properties(properties);
  
  if (_shared_gsg) {
    _context = [[EAGLContext alloc] initWithAPI:_shared_gsg->_context.API
                                    sharegroup:_shared_gsg->_context.sharegroup];
  } else {
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  }

  // The rest of this method restricts the properties to the small subset that
  // a CAEAGLLayer supports.
  if (!layer) {
    return;
  }

  NSString *format;

  // Check if we're specifically requesting RGB565:
  if (_fbprops.get_red_bits() == 5 && _fbprops.get_green_bits() == 6 &&
      _fbprops.get_blue_bits() == 5 && _fbprops.get_alpha_bits() == 0) {
    
    format = kEAGLColorFormatRGB565;
  } else {
    // If not, use RGBA8 or SRGBA8.
    if (_fbprops.get_srgb_color()) {
      format = kEAGLColorFormatSRGBA8;
    } else {
      format = kEAGLColorFormatRGBA8;
    }

    _fbprops.set_rgba_bits(8, 8, 8, 8);
  }

  // EAGLGraphicsWindow will exclusively use a combined depth-stencil buffer.
  // Panda's architecture makes the assumption that a GraphicsWindow will
  // not be backed by an FBO, which isn't true in this case. Instead of
  // reimplementing the entirety of GraphicsBuffer, we'll just support a
  // small portion of it.
  _fbprops.set_depth_bits(24);
  _fbprops.set_stencil_bits(8);

  layer.drawableProperties = @{
    kEAGLDrawablePropertyRetainedBacking: @NO,
    kEAGLDrawablePropertyColorFormat: format
  };
}

/**
 * Gets a pointer to the named GLES extension.
 */
void *EAGLGraphicsStateGuardian::
do_get_extension_func(const char *name) {
  return dlsym(RTLD_DEFAULT, name);
}
