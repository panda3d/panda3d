// Filename: config_dxgsg.cxx
// Created by:  drose (06Oct99)
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

#include "config_dxgsg7.h"
#include "dxGraphicsStateGuardian7.h"
#include "dxSavedFrameBuffer7.h"
#include "dxTextureContext7.h"
#include "wdxGraphicsPipe7.h"
#include "wdxGraphicsWindow7.h"
#include "graphicsPipeSelection.h"

#include "dconfig.h"

Configure(config_dxgsg7);
NotifyCategoryDef(dxgsg7, ":display:gsg");
NotifyCategoryDef(wdxdisplay7, "windisplay");

// Configure this variable true to cause the DXGSG to show each
// transform space it renders by drawing a little unit axis.  This
// cannot be enabled when the player is compiled in NDEBUG mode.
bool dx_show_transforms = config_dxgsg7.GetBool("dx-show-transforms", false);

//  Configure this to TRUE if you want DirectX to control the entire screen,
//  If false, it will just blit into a window.
bool dx_full_screen = config_dxgsg7.GetBool("dx-full-screen", false);

//  Configure this true to force the rendering to sync to the video
//  refresh, or false to let your frame rate go as high as it can,
//  irrespective of the video refresh.
bool dx_sync_video = config_dxgsg7.GetBool("sync-video", true);

// enable this to turn on full-screen anti-aliasing, if the HW supports it
// this var is also used in wdxGraphicsWindows.cxx
bool dx_full_screen_antialiasing = config_dxgsg7.GetBool("dx-antialias", false);

// Configure this true to perform a cull traversal over the geometry
// by default, false otherwise.  The cull traversal provides support
// for state-sorting, z-sorting, and binning.
bool dx_cull_traversal = config_dxgsg7.GetBool("dx-cull-traversal", true);

// if true, if card only supports per-vertex fog, it will be treated as no-HW fog capability
bool dx_no_vertex_fog = config_dxgsg7.GetBool("dx-no-vertex-fog", false);

// Configure this true to cause all lighting normals to automatically
// be normalized by the CPU before rendering.  This is
// necessary if you intend to render things under scale transforms and
// expect lighting to work correctly.  Maybe one day there will be
// another way to set this at runtime, instead of only as a configure
// variable
bool dx_auto_normalize_lighting = config_dxgsg7.GetBool("auto-normalize-lighting", true);

bool dx_allow_software_renderer = config_dxgsg7.GetBool("dx-allow-software-renderer", false);
bool dx_force_software_renderer = config_dxgsg7.GetBool("dx-force-software-renderer", false);

#ifndef NDEBUG
// debugging flag
// values are same as D3DCULL enumtype, 0 - no force, 1 - force none, 2 - force CW, 3 - force CCW
int dx_force_backface_culling = config_dxgsg7.GetInt("dx-force-backface-culling", 0);
#endif

bool dx_mipmap_everything = config_dxgsg7.GetBool("dx-mipmap-everything", false);
bool dx_ignore_mipmaps = config_dxgsg7.GetBool("dx-ignore-mipmaps", false);

// if this is set, more accurate but more expensive fog computations are performed
bool dx_use_rangebased_fog = config_dxgsg7.GetBool("dx-use-rangebased-fog", false);
bool dx_force_16bpptextures = config_dxgsg7.GetBool("dx-force-16bpptextures", false);
bool dx_no_dithering = config_dxgsg7.GetBool("dx-no-dithering", false);
bool dx_force_16bpp_zbuffer = config_dxgsg7.GetBool("dx-force-16bpp-zbuffer", false);
bool dx_do_vidmemsize_check = config_dxgsg7.GetBool("do-vidmemsize-check", true);
// Setting this true theoretically hinders render performance, because
// it forces the FPU to go through some extra work to clean itself up
// after rendering a frame, but the performance cost seems to be
// small.  On the other hand, setting it false can force the
// application to run in single-precision arithmetic mode, even if
// it believes it is using double-precision variables.
bool dx_preserve_fpu_state = config_dxgsg7.GetBool("dx-preserve-fpu-state", true);

// Configure this true to try to implement decals using a
// DepthOffsetAttrib, false to do them with the more reliable 3-pass
// rendering method instead.
bool dx_depth_offset_decals = config_dxgsg7.GetBool("depth-offset-decals", false);

#ifdef _DEBUG
float dx_global_miplevel_bias = config_dxgsg7.GetFloat("dx-global-miplevel-bias", 0.0);
bool dx_debug_view_mipmaps = config_dxgsg7.GetBool("dx-debug-view-mipmaps", false);
bool dx_force_anisotropic_filtering = config_dxgsg7.GetBool("dx-force-anisotropic-filtering", false);
//int dx_print_texstats = config_dxgsg7.GetBool("dx-print-texstats", 0);
#endif

// set 'retained-mode #t' and this to have prepare_geom concatenate all tristrips within a geom 
// together using degenerate tris
const bool link_tristrips = config_dxgsg7.GetBool("link-tristrips", false);

// note:  offset currently disabled since it wasnt working properly
DXDecalType dx_decal_type = GDT_blend;

static DXDecalType
parse_decal_type(const string &type) {
  if (type == "mask") {
    return GDT_mask;
  } else if (type == "blend") {
    return GDT_blend;
  } else if (type == "offset") {
    return GDT_offset;
  }
  dxgsg7_cat.error() << "Invalid dx-decal-type: " << type << "\n";
  return GDT_blend;
}

ConfigureFn(config_dxgsg7) {
  init_libdxgsg7();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libdxgsg7
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libdxgsg7() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  string decal_type = config_dxgsg7.GetString("dx-decal-type", "");
  if (!decal_type.empty()) {
    dx_decal_type = parse_decal_type(decal_type);
  }

  DXGraphicsStateGuardian7::init_type();
  DXSavedFrameBuffer7::init_type();
  DXTextureContext7::init_type();
  DXGeomNodeContext7::init_type();

  wdxGraphicsPipe7::init_type();
  wdxGraphicsWindow7::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(wdxGraphicsPipe7::get_class_type(),
                           wdxGraphicsPipe7::pipe_constructor);
}
