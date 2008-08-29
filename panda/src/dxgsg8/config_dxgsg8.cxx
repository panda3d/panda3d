// Filename: config_dxgsg8.cxx
// Created by:  drose (06Oct99)
//
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

#include "config_dxgsg8.h"
#include "dxGraphicsStateGuardian8.h"
#include "dxTextureContext8.h"
#include "dxVertexBufferContext8.h"
#include "dxIndexBufferContext8.h"
#include "dxGeomMunger8.h"
#include "graphicsPipeSelection.h"
#include "wdxGraphicsWindow8.h"
#include "wdxGraphicsPipe8.h"
#include "pandaSystem.h"

#include "dconfig.h"

Configure(config_dxgsg8);
NotifyCategoryDef(dxgsg8, ":display:gsg");
NotifyCategoryDef(wdxdisplay8, "display");

// Configure this variable true to cause the DXGSG to show each
// transform space it renders by drawing a little unit axis.  This
// cannot be enabled when the player is compiled in NDEBUG mode.
ConfigVariableBool dx_show_transforms
("dx-show-transforms", false);

// Set Level of MultiSampling to be used, if HW supports it.  Valid values are 2-16.
ConfigVariableInt dx_multisample_antialiasing_level
("dx-multisample-antialiasing-level", 0);

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

ConfigVariableBool dx_broken_max_index
("dx-broken-max-index", false,
 PRC_DESC("Configure this true if you have a buggy graphics driver that "
          "doesn't correctly implement the third parameter, NumVertices, "
          "of DrawIndexedPrimitive().  In particular, the NVIDIA Quadro "
          "driver version 6.14.10.7184 seems to treat this as a maximum "
          "vertex index, rather than a delta between the maximum and "
          "minimum vertex index.  Turn this on if you are seeing stray "
          "triangles, or you are not seeing all of your triangles.  Enabling "
          "this should work around this bug, at the cost of some additional "
          "rendering overhead on the GPU."));

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

ConfigVariableInt dx_preferred_device_id
("dx-preferred-device-id", -1);

#ifdef _DEBUG
ConfigVariableDouble dx_global_miplevel_bias
("dx-global-miplevel-bias", 0.0);
ConfigVariableBool dx_debug_view_mipmaps
("dx-debug-view-mipmaps", false);
#endif

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
  DXTextureContext8::init_type();
  DXVertexBufferContext8::init_type();
  DXIndexBufferContext8::init_type();
  DXGeomMunger8::init_type();

  wdxGraphicsPipe8::init_type();
  wdxGraphicsWindow8::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(wdxGraphicsPipe8::get_class_type(),
                           wdxGraphicsPipe8::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("DirectX8");
}
