// Filename: config_glgsg.cxx
// Created by:  drose (06Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_glgsg.h"
#include "glGraphicsStateGuardian.h"
#include "glSavedFrameBuffer.h"
#include "glTextureContext.h"

#include <dconfig.h>

Configure(config_glgsg);
NotifyCategoryDef(glgsg, ":display:gsg");

// Configure this variable true to cause the GLGSG to show each
// transform space it renders by drawing a little unit axis.  This
// cannot be enabled when the player is compiled in NDEBUG mode.
bool gl_show_transforms = config_glgsg.GetBool("gl-show-transforms", false);

// Configure this true to glHint the textures into the cheapest
// possible mode.
bool gl_cheap_textures = config_glgsg.GetBool("gl-cheap-textures", false);

// Configure this true to perform a cull traversal over the geometry
// by default, false otherwise.  The cull traversal provides support
// for state-sorting, z-sorting, and binning.
bool gl_cull_traversal = config_glgsg.GetBool("gl-cull-traversal", true);

// Configure this true to disable the use of mipmapping in the
// renderer.
bool gl_ignore_mipmaps = config_glgsg.GetBool("gl-ignore-mipmaps", false);

// Configure this true to enable full trilinear mipmapping on every
// texture, whether it asks for it or not.
bool gl_force_mipmaps = config_glgsg.GetBool("gl-force-mipmaps", false);

// Configure this true to cause mipmaps to be rendered with phony
// colors, using mipmap_level_*.rgb if they are available.
bool gl_show_mipmaps = config_glgsg.GetBool("gl-show-mipmaps", false);

// Configure this true to cause all lighting normals to automatically
// be normalized by the graphics hardware before rendering.  This is
// necessary if you intend to render things under scale transforms and
// expect lighting to work correctly.  Maybe one day there will be
// another way to set this at runtime, instead of only as a configure
// variable.
bool gl_auto_normalize_lighting = config_glgsg.GetBool("auto-normalize-lighting", false);

GLDecalType gl_decal_type = GDT_offset;

static GLDecalType
parse_decal_type(const string &type) {
  if (type == "mask") {
    return GDT_mask;
  } else if (type == "blend") {
    return GDT_blend;
  } else if (type == "offset") {
    return GDT_offset;
  }
  glgsg_cat.error() << "Invalid gl-decal-type: " << type << "\n";
  return GDT_offset;
}

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

  string decal_type = config_glgsg.GetString("gl-decal-type", "");
  if (!decal_type.empty()) {
    gl_decal_type = parse_decal_type(decal_type);
  }
  
  GLGraphicsStateGuardian::init_type();
  GLSavedFrameBuffer::init_type();
  GLTextureContext::init_type();

  GraphicsStateGuardian::get_factory().register_factory
    (GLGraphicsStateGuardian::get_class_type(),
     GLGraphicsStateGuardian::make_GlGraphicsStateGuardian);
}
