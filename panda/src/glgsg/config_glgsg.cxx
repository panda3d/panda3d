// Filename: config_glgsg.cxx
// Created by:  drose (06Oct99)
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

#include "config_glgsg.h"
#include "glGraphicsStateGuardian.h"
#include "glSavedFrameBuffer.h"
#include "glTextureContext.h"
#include "glGeomNodeContext.h"

#include <dconfig.h>

Configure(config_glgsg);
NotifyCategoryDef(glgsg, ":display:gsg");

// Configure this true to glHint the textures into the cheapest
// possible mode.
bool gl_cheap_textures = config_glgsg.GetBool("gl-cheap-textures", false);

// Configure this true to ignore texture modes like modulate that
// blend texture color with polygon color (a little cheaper for
// software renderers).
bool gl_always_decal_textures = config_glgsg.GetBool("gl-always-decal-textures", false);

// Configure this true to disable texture clamp mode (all textures
// repeat, a little cheaper for software renderers).
bool gl_ignore_clamp = config_glgsg.GetBool("gl-ignore-clamp", false);

// Configure this true to disable any texture filters at all (forcing
// point sampling).
bool gl_ignore_filters = config_glgsg.GetBool("gl-ignore-filters", false);

// Configure this true to disable mipmapping only.
bool gl_ignore_mipmaps = config_glgsg.GetBool("gl-ignore-mipmaps", false) || gl_ignore_filters;

// Configure this true to enable full trilinear mipmapping on every
// texture, whether it asks for it or not.
bool gl_force_mipmaps = config_glgsg.GetBool("gl-force-mipmaps", false);

// Configure this true to cause mipmaps to be rendered with phony
// colors, using mipmap_level_*.rgb if they are available.
bool gl_show_mipmaps = config_glgsg.GetBool("gl-show-mipmaps", false);

// Configure this true to cause the generated mipmap images to be
// written out to image files on the disk as they are generated.
bool gl_save_mipmaps = config_glgsg.GetBool("gl-save-mipmaps", false);

// Configure this true to cause all lighting normals to automatically
// be normalized by the graphics hardware before rendering.  This is
// necessary if you intend to render things under scale transforms and
// expect lighting to work correctly.  Maybe one day there will be
// another way to set this at runtime, instead of only as a configure
// variable.
bool gl_auto_normalize_lighting = config_glgsg.GetBool("auto-normalize-lighting", true);

// Configure this true to try to implement decals using a
// DepthOffsetAttrib, false to do them with the more reliable 3-pass
// rendering method instead.
bool gl_depth_offset_decals = config_glgsg.GetBool("depth-offset-decals", false);

// Configure this true to indicate the current version of GL fully
// supports textures with B, G, R ordering; false if it only supports
// R, G, B.  false will always work, but true might be faster if the
// implementation supports it.
#ifdef GL_BGR
bool gl_supports_bgr = config_glgsg.GetBool("gl-supports-bgr", false);
#else
// If it's not even defined, we can't use it.
bool gl_supports_bgr = false;
#endif  // GL_BGR

// Configure this false if your GL's implementation of glColorMask()
// is broken (some are).  This will force the use of a (presumably)
// more expensive blending operation instead.
bool gl_color_mask = config_glgsg.GetBool("gl-color-mask", true);

ConfigureFn(config_glgsg) {
  init_libglgsg();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libglgsg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libglgsg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  GLGraphicsStateGuardian::init_type();
  GLSavedFrameBuffer::init_type();
  GLTextureContext::init_type();
  GLGeomNodeContext::init_type();
}
