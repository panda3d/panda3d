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

//  Configure this to TRUE if you want DirectX to control the entire screen,
//  If false, it will just blit into a window.
ConfigVariableBool dx_full_screen
("dx-full-screen", false);

//  Configure this true to force the rendering to sync to the video
//  refresh, or false to let your frame rate go as high as it can,
//  irrespective of the video refresh.
ConfigVariableBool dx_sync_video
("sync-video", true);

// enable this to turn on full-screen anti-aliasing, if the HW supports it
// this var is also used in wdxGraphicsWindows.cxx
ConfigVariableBool dx_full_screen_antialiasing
("dx-antialias", false);

// if true, if card only supports per-vertex fog, it will be treated as no-HW fog capability
ConfigVariableBool dx_no_vertex_fog
("dx-no-vertex-fog", false);

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

#ifdef _DEBUG
ConfigVariableDouble dx_global_miplevel_bias
("dx-global-miplevel-bias", 0.0);
ConfigVariableBool dx_debug_view_mipmaps
("dx-debug-view-mipmaps", false);
ConfigVariableBool dx_force_anisotropic_filtering
("dx-force-anisotropic-filtering", false);
//int dx_print_texstats
("dx-print-texstats", 0);
#endif

// set 'retained-mode #t' and this to have prepare_geom concatenate all tristrips within a geom 
// together using degenerate tris
ConfigVariableBool link_tristrips
("link-tristrips", false);

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

  DXGraphicsStateGuardian7::init_type();
  DXSavedFrameBuffer7::init_type();
  DXTextureContext7::init_type();

  wdxGraphicsPipe7::init_type();
  wdxGraphicsWindow7::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(wdxGraphicsPipe7::get_class_type(),
                           wdxGraphicsPipe7::pipe_constructor);
}
