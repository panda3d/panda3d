// Filename: config_crgsg.cxx
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

#include "config_crgsg.h"
#include "crGraphicsStateGuardian.h"
#include "crSavedFrameBuffer.h"
#include "crTextureContext.h"
#include "crGeomNodeContext.h"

#include "dconfig.h"

Configure(config_crgsg);
NotifyCategoryDef(crgsg, ":display:gsg");

// Configure this variable true to cause the CRGSG to show each
// transform space it renders by drawing a little unit axis.  This
// cannot be enabled when the player is compiled in NDEBUG mode.
bool cr_show_transforms = config_crgsg.GetBool("cr-show-transforms", false);

// Configure this true to chromium.Hint the textures into the cheapest
// possible mode.
bool cr_cheap_textures = config_crgsg.GetBool("cr-cheap-textures", false);

// Configure this true to perform a cull traversal over the geometry
// by default, false otherwise.  The cull traversal provides support
// for state-sorting, z-sorting, and binning.
bool cr_cull_traversal = config_crgsg.GetBool("cr-cull-traversal", true);

// Configure this true to disable the use of mipmapping in the
// renderer.
bool cr_ignore_mipmaps = config_crgsg.GetBool("cr-ignore-mipmaps", false);

// Configure this true to enable full trilinear mipmapping on every
// texture, whether it asks for it or not.
bool cr_force_mipmaps = config_crgsg.GetBool("cr-force-mipmaps", false);

// Configure this true to cause mipmaps to be rendered with phony
// colors, using mipmap_level_*.rgb if they are available.
bool cr_show_mipmaps = config_crgsg.GetBool("cr-show-mipmaps", false);

// Configure this true to cause the generated mipmap images to be
// written out to image files on the disk as they are generated.
bool cr_save_mipmaps = config_crgsg.GetBool("cr-save-mipmaps", false);

// Configure this true to cause all lighting normals to automatically
// be normalized by the graphics hardware before rendering.  This is
// necessary if you intend to render things under scale transforms and
// expect lighting to work correctly.  Maybe one day there will be
// another way to set this at runtime, instead of only as a configure
// variable.
bool cr_auto_normalize_lighting = config_crgsg.GetBool("auto-normalize-lighting", false);

// Configure this true to try to implement decals using a
// DepthOffsetAttrib, false to do them with the more reliable 3-pass
// rendering method instead.
bool cr_depth_offset_decals = config_crgsg.GetBool("depth-offset-decals", false);

// Configure this true to indicate the current version of GL fully
// supports textures with B, G, R ordering; false if it only supports
// R, G, B.  false will always work, but true might be faster if the
// implementation supports it.
#ifdef GL_BGR
bool cr_supports_bgr = config_crgsg.GetBool("cr-supports-bgr", false);
#else
// If it's not even defined, we can't use it.
bool cr_supports_bgr = false;
#endif  // GL_BGR

CRDecalType cr_decal_type = GDT_offset;

static CRDecalType
parse_decal_type(const string &type) {
  if (type == "mask") {
    return GDT_mask;
  } else if (type == "blend") {
    return GDT_blend;
  } else if (type == "offset") {
    return GDT_offset;
  }
  crgsg_cat.error() << "Invalid cr-decal-type: " << type << "\n";
  return GDT_offset;
}

ConfigureFn(config_crgsg) {
  init_libcrgsg();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libcrgsg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libcrgsg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  string decal_type = config_crgsg.GetString("cr-decal-type", "");
  if (!decal_type.empty()) {
    cr_decal_type = parse_decal_type(decal_type);
  }

  CRGraphicsStateGuardian::init_type();
  CRSavedFrameBuffer::init_type();
  CRTextureContext::init_type();
  CRGeomNodeContext::init_type();

  GraphicsStateGuardian::get_factory().register_factory
    (CRGraphicsStateGuardian::get_class_type(),
     CRGraphicsStateGuardian::make_GlGraphicsStateGuardian);
}
