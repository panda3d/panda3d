/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_dxgsg9.cxx
 * @author drose
 * @date 1999-10-06
 */

#include "config_dxgsg9.h"
#include "dxGraphicsStateGuardian9.h"
#include "dxTextureContext9.h"
#include "dxVertexBufferContext9.h"
#include "dxIndexBufferContext9.h"
#include "dxOcclusionQueryContext9.h"
#include "dxShaderContext9.h"
#include "dxGeomMunger9.h"
#include "graphicsPipeSelection.h"
#include "wdxGraphicsWindow9.h"
#include "wdxGraphicsPipe9.h"
#include "wdxGraphicsBuffer9.h"
#include "pandaSystem.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDADX)
  #error Buildsystem error: BUILDING_PANDADX not defined
#endif

DToolConfigure(config_dxgsg9);
NotifyCategoryDef(dxgsg9, ":display:gsg");
NotifyCategoryDef(wdxdisplay9, "display");

// Configure this variable true to cause the DXGSG to show each transform
// space it renders by drawing a little unit axis.  This cannot be enabled
// when the player is compiled in NDEBUG mode.
ConfigVariableBool dx_show_transforms
("dx-show-transforms", false);

// if true, if card only supports per-vertex fog, it will be treated as no-HW
// fog capability
ConfigVariableBool dx_no_vertex_fog
("dx-no-vertex-fog", false);

// if true, overwrite cursor bitmap tip with "D3D" to distinguish it from GDI
// cursor
ConfigVariableBool dx_show_cursor_watermark
("dx-show-cursor-watermark",
#ifdef _DEBUG
    true
#else
    false
#endif
    );

// if true, triangle filter will be used to generate mipmap levels instead of
// default box filter
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

ConfigVariableBool dx_broken_depth_bias
("dx-broken-depth-bias", true,
 PRC_DESC("Configure this true if your graphics driver claims to support "
          "D3DPRASTERCAPS_DEPTHBIAS, but doesn't appear to do anything "
          "useful when you set it.  In fact, there's not much reason not "
          "to just leave this set, since the workaround seems to be "
          "sufficient for all cases."));

ConfigVariableDouble dx_depth_bias_scale
("dx-depth-bias-scale", 0.000001,
 PRC_DESC("If depth bias is not directly supported by the graphics driver "
          "(or if dx-broken-depth-bias is set true), this configures the "
          "amount by which we slide the viewport back to achieve the effect "
          "of a depth bias.  It should generally be a small number."));

ConfigVariableBool dx_count_all_cards_memory
("dx-count-all-cards-memory", true,
 PRC_DESC("Set this to false to skip the counting of extra cards memory "
          "via DX7 calls."));

#ifndef NDEBUG
// debugging flag values are same as D3DCULL enumtype, 0 - no force, 1 - force
// none, 2 - force CW, 3 - force CCW
ConfigVariableInt dx_force_backface_culling
("dx-force-backface-culling", 0);
#endif

ConfigVariableBool dx_mipmap_everything
("dx-mipmap-everything", false);
ConfigVariableBool dx_ignore_mipmaps
("dx-ignore-mipmaps", false);

// if this is set, more accurate but more expensive fog computations are
// performed
ConfigVariableBool dx_use_rangebased_fog
("dx-use-rangebased-fog", false);
ConfigVariableBool dx_no_dithering
("dx-no-dithering", false);
ConfigVariableBool dx_force_16bpp_zbuffer
("dx-force-16bpp-zbuffer", false);
ConfigVariableBool dx_do_vidmemsize_check
("do-vidmemsize-check", true);
// Setting this true theoretically hinders render performance, because it
// forces the FPU to go through some extra work to clean itself up after
// rendering a frame, but the performance cost seems to be small.  On the
// other hand, setting it false can force the application to run in single-
// precision arithmetic mode, even if it believes it is using double-precision
// variables.
ConfigVariableBool dx_preserve_fpu_state
("dx-preserve-fpu-state", true);

ConfigVariableInt dx_preferred_device_id
("dx-preferred-device-id", -1);

ConfigVariableBool dx_intel_compressed_texture_bug
("dx-intel-compressed-texture-bug", true,
 PRC_DESC("Set this true to work around a bug in the Intel driver "
          "igdumd32.dll, for at least the 965 Express chipset family, "
          "which breaks compressed texture images smaller "
          "than about 256x256 (even mipmap levels).  The workaround is "
          "simply to disable compressed texture support when this "
          "driver is detected."));

#ifdef _DEBUG
ConfigVariableDouble dx_global_miplevel_bias
("dx-global-miplevel-bias", 0.0);
ConfigVariableBool dx_debug_view_mipmaps
("dx-debug-view-mipmaps", false);
#endif

ConfigVariableBool dx_force_anisotropic_filtering
("dx-force-anisotropic-filtering", false);

// set 'retained-mode #t' and this to have prepare_geom concatenate all
// tristrips within a geom together using degenerate tris
ConfigVariableBool link_tristrips
("link-tristrips", false);

// true = use DirectX management of video memory false = see dx_lru_management
// config variable below
ConfigVariableBool dx_management
("dx-management", false);

// valid only if dx_management == false true = use DirectX management of
// texture memory false = lru will manage texture memory
ConfigVariableBool dx_texture_management
("dx-texture-management", true);

// valid only if dx_management == false true = enable LRU management of video
// memory false = no video memory management
ConfigVariableBool dx_lru_management
("dx-lru-management", true);

// number of LRU pages to pre-allocate if the maximum number of pages is used
// up, then LRU pages will be dynamically allocatedfreed
ConfigVariableInt dx_lru_maximum_pages
("dx-lru-maximum-pages", 8192);

// the amount of video memory the LRU will try not to use this will allow
// DirectX some space in case of memory fragmentation, ... this does not apply
// if dx_lru_minimum_memory_requirement is not met
ConfigVariableInt dx_lru_free_memory_requirement
("dx-lru-free-memory-requirement", 12000000);

// this is like the minimum recommended amount of video memory
ConfigVariableInt dx_lru_minimum_memory_requirement
("dx-lru-minimum-memory-requirement", 64000000);

// used to cap the amount of video memory used 0 = use all available DirectX
// video memory
ConfigVariableInt dx_lru_maximum_memory_requirement
("dx-lru-maximum-memory-requirement", 0);

// the number of LRU pages the LRU will update per frame do not set this too
// high or it will degrade performance
ConfigVariableInt dx_lru_maximum_page_updates_per_frame
("dx-lru-maximum-page-updates-per-frame", 40);

// lru debug onoff
ConfigVariableBool dx_lru_debug
("dx-lru-debug", false);

// valid only if dx_lru_debug == true number of frames to wait until printing
// out the LRU status
ConfigVariableInt dx_lru_debug_frames_til_output
("dx-lru-debug-frames-til-output", 500);

// valid only if dx_lru_debug == true
ConfigVariableBool dx_lru_debug_textures
("dx-lru-debug-textures", false);

// valid only if dx_lru_debug == true
ConfigVariableBool dx_lru_debug_vertex_buffers
("dx-lru-debug-vertex-buffers", false);

ConfigVariableBool dx_use_dynamic_textures
("dx-use-dynamic-textures", true);

// DX device option
ConfigVariableBool dx_use_multithread
("dx-use-multithread", false);

// DX device option
ConfigVariableBool dx_use_puredevice
("dx-use-puredevice", false);

// DX device option
ConfigVariableBool dx_disable_driver_management
("dx-disable-driver-management", false);

// DX device option
ConfigVariableBool dx_disable_driver_management_ex
("dx-disable-driver-management-ex", false);

// nVidia's performace heads up display
ConfigVariableBool dx_use_nvperfhud
("dx-use-nvperfhud", false);

ConfigureFn(config_dxgsg9) {
  init_libdxgsg9();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libdxgsg9() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  DXGraphicsStateGuardian9::init_type();
  DXTextureContext9::init_type();
  DXVertexBufferContext9::init_type();
  DXIndexBufferContext9::init_type();
  DXOcclusionQueryContext9::init_type();
  DXShaderContext9::init_type();
  DXGeomMunger9::init_type();

  wdxGraphicsPipe9::init_type();
  wdxGraphicsWindow9::init_type();
  wdxGraphicsBuffer9::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(wdxGraphicsPipe9::get_class_type(),
                           wdxGraphicsPipe9::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("DirectX9");
}
