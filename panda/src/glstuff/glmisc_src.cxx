// Filename: glmisc_src.cxx
// Created by:  drose (09Feb04)
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


// Configure this true to GLP(Hint) the textures into the cheapest
// possible mode.
bool CLP(cheap_textures) = CONFIGOBJ.GetBool("gl-cheap-textures", false);

// Configure this true to ignore texture modes like modulate that
// blend texture color with polygon color (a little cheaper for
// software renderers).
bool CLP(always_decal_textures) = CONFIGOBJ.GetBool("gl-always-decal-textures", false);

// Configure this true to disable texture clamp mode (all textures
// repeat, a little cheaper for software renderers).
bool CLP(ignore_clamp) = CONFIGOBJ.GetBool("gl-ignore-clamp", false);

// Configure this true to disable any texture filters at all (forcing
// point sampling).
bool CLP(ignore_filters) = CONFIGOBJ.GetBool("gl-ignore-filters", false);

// Configure this true to disable mipmapping only.
bool CLP(ignore_mipmaps) = CONFIGOBJ.GetBool("gl-ignore-mipmaps", false) || CLP(ignore_filters);

// Configure this true to enable full trilinear mipmapping on every
// texture, whether it asks for it or not.
bool CLP(force_mipmaps) = CONFIGOBJ.GetBool("gl-force-mipmaps", false);

// Configure this true to cause mipmaps to be rendered with phony
// colors, using mipmap_level_*.rgb if they are available.
bool CLP(show_mipmaps) = CONFIGOBJ.GetBool("gl-show-mipmaps", false);

// Configure this true to cause the generated mipmap images to be
// written out to image files on the disk as they are generated.
bool CLP(save_mipmaps) = CONFIGOBJ.GetBool("gl-save-mipmaps", false);

// Configure this true to cause all lighting normals to automatically
// be normalized by the graphics hardware before rendering.  This is
// necessary if you intend to render things under scale transforms and
// expect lighting to work correctly.  Maybe one day there will be
// another way to set this at runtime, instead of only as a configure
// variable.
bool CLP(auto_normalize_lighting) = CONFIGOBJ.GetBool("auto-normalize-lighting", true);

// Configure this true to try to implement decals using a
// DepthOffsetAttrib, false to do them with the more reliable 3-pass
// rendering method instead.
bool CLP(depth_offset_decals) = CONFIGOBJ.GetBool("depth-offset-decals", false);

// Configure this false if your GL's implementation of GLP(ColorMask)()
// is broken (some are).  This will force the use of a (presumably)
// more expensive blending operation instead.
bool CLP(color_mask) = CONFIGOBJ.GetBool("gl-color-mask", true);

void CLP(init_classes)() {
  CLP(GraphicsStateGuardian)::init_type();
  CLP(SavedFrameBuffer)::init_type();
  CLP(TextureContext)::init_type();
  CLP(GeomNodeContext)::init_type();
}

