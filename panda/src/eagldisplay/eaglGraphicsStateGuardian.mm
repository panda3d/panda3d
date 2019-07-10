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
GLES2GraphicsStateGuardian(engine, pipe) {
  
}

/**
 *
 */
EAGLGraphicsStateGuardian::
~EAGLGraphicsStateGuardian() {
  
}

/**
 * Gets the FrameBufferProperties to match the indicated config.
 */
void EAGLGraphicsStateGuardian::
get_properties(FrameBufferProperties &properties) {
  
  properties.clear();
  
  // TODO: Don't make assumptions about the available properties.
  properties.set_color_bits(32);
  properties.set_alpha_bits(8);
  properties.set_depth_bits(8);
  properties.set_rgba_bits(8, 8, 8, 8);
  properties.set_back_buffers(1);
  properties.set_force_hardware(1);
  properties.set_srgb_color(true);
}

/**
 * Selects a visual or fbconfig for all the windows and buffers that use this
 * gsg.  Also creates the GL context and obtains the visual.
 */
void EAGLGraphicsStateGuardian::
choose_pixel_format(const FrameBufferProperties &properties,
                    bool need_pbuffer) {
  _fbprops.clear();
  get_properties(_fbprops);
  _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
}

/**
 * Gets a pointer to the named GLES extension.
 */
void *EAGLGraphicsStateGuardian::
do_get_extension_func(const char *name) {
  return dlsym(RTLD_DEFAULT, name);
}
