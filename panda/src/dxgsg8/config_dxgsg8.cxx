// Filename: config_dxgsg8.cxx
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

#include "config_dxgsg8.h"
#include "dxGraphicsStateGuardian8.h"
#include "dxSavedFrameBuffer8.h"
#include "dxTextureContext8.h"
#include "graphicsPipeSelection.h"
#include "wdxGraphicsWindow8.h"
#include "wdxGraphicsPipe8.h"

#include "dconfig.h"

Configure(config_dxgsg8);
//NotifyCategoryDef(dxgsg8, ":display:gsg");  dont want to merge this with the regular parent class dbg output
NotifyCategoryDef(dxgsg8, "dxgsg");
NotifyCategoryDef(wdxdisplay8, "windisplay");

// Configure this variable true to cause the DXGSG to show each
// transform space it renders by drawing a little unit axis.  This
// cannot be enabled when the player is compiled in NDEBUG mode.
ConfigVariableBool dx_show_transforms
("dx-show-transforms", false);

//  Configure this to TRUE if you want DirectX to control the entire screen,
//  If false, it will just blit into a window.
ConfigVariableBool dx_full_screen
("dx-full-screen", false);

//  Configure this true to force the rendering to sync to the video
//  refresh, or false to let your frame rate go as high as it can,
//  irrespective of the video refresh.
ConfigVariableBool dx_sync_video
("sync-video", true);

// Set Level of MultiSampling to be used, if HW supports it.  Valid values are 2-16.
ConfigVariableInt dx_multisample_antialiasing_level
("dx-multisample-antialiasing-level", 0);

// Configure this true to perform a cull traversal over the geometry
// by default, false otherwise.  The cull traversal provides support
// for state-sorting, z-sorting, and binning.
ConfigVariableBool dx_cull_traversal
("dx-cull-traversal", true);

// if true, if card only supports per-vertex fog, it will be treated as no-HW fog capability
ConfigVariableBool dx_no_vertex_fog
("dx-no-vertex-fog", false);

// if true, overwrite cursor bitmap tip with "D3D" to distinguish it from GDI cursor 
ConfigVariableBool dx_show_cursor_watermark
("dx-show-cursor-watermark", 
#ifdef _DEBUG
    true
#else
    false
#endif
    );

// if true, triangle filter will be used to generate mipmap levels instead of default box filter
ConfigVariableBool dx_use_triangle_mipgen_filter
("dx-use-triangle-mipgen-filter", false);

// Configure this true to cause all lighting normals to automatically
// be normalized by the CPU before rendering.  This is
// necessary if you intend to render things under scale transforms and
// expect lighting to work correctly.  Maybe one day there will be
// another way to set this at runtime, instead of only as a configure
// variable
ConfigVariableBool dx_auto_normalize_lighting
("auto-normalize-lighting", true);

#ifndef NDEBUG
// debugging flag
// values are same as D3DCULL enumtype, 0 - no force, 1 - force none, 2 - force CW, 3 - force CCW
ConfigVariableInt dx_force_backface_culling
("dx-force-backface-culling", 0);
#endif

ConfigVariableBool dx_mipmap_everything
("dx-mipmap-everything", false);
ConfigVariableBool dx_ignore_mipmaps
("dx-ignore-mipmaps", false);

// if this is set, more accurate but more expensive fog computations are performed
ConfigVariableBool dx_use_rangebased_fog
("dx-use-rangebased-fog", false);
ConfigVariableBool dx_force_16bpptextures
("dx-force-16bpptextures", false);
ConfigVariableBool dx_no_dithering
("dx-no-dithering", false);
ConfigVariableBool dx_force_16bpp_zbuffer
("dx-force-16bpp-zbuffer", false);
ConfigVariableBool dx_do_vidmemsize_check
("do-vidmemsize-check", true);
// Setting this true theoretically hinders render performance, because
// it forces the FPU to go through some extra work to clean itself up
// after rendering a frame, but the performance cost seems to be
// small.  On the other hand, setting it false can force the
// application to run in single-precision arithmetic mode, even if
// it believes it is using double-precision variables.
ConfigVariableBool dx_preserve_fpu_state
("dx-preserve-fpu-state", true);

// if true, override win-width/height and use driver vidmem info to
// pick what will be a fullscreen window size close to the best perf
// capability of card, based on a heuristic
ConfigVariableBool dx_pick_best_screenres
("pick-best-screenres", false);

ConfigVariableInt dx_preferred_device_id
("dx-preferred-device-id", -1);

#ifdef _DEBUG
float dx_global_miplevel_bias
("dx-global-miplevel-bias", 0.0);
ConfigVariableBool dx_debug_view_mipmaps
("dx-debug-view-mipmaps", false);
#endif

// use dx8 or GDI mouse cursor in fullscreen mode?
// Nvidia dx8 cursor is invisible as of 28.32 drivers, so using GDI in fullscrn by default for now
ConfigVariableBool dx_use_dx_cursor
("dx-use-dx-cursor", false);

ConfigVariableBool dx_force_anisotropic_filtering
("dx-force-anisotropic-filtering", false);

// set 'retained-mode #t' and this to have prepare_geom concatenate all tristrips within a geom 
// together using degenerate tris
ConfigVariableBool link_tristrips
("link-tristrips", false);

ConfigureFn(config_dxgsg8) {
  init_libdxgsg8();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libdxgsg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libdxgsg8() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  DXGraphicsStateGuardian8::init_type();
  DXSavedFrameBuffer8::init_type();
  DXTextureContext8::init_type();

  wdxGraphicsPipe8::init_type();
  wdxGraphicsWindow8::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(wdxGraphicsPipe8::get_class_type(),
                           wdxGraphicsPipe8::pipe_constructor);

}
