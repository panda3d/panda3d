// Filename: config_dxgsg.cxx
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

#include "config_dxgsg8.h"
#include "dxGraphicsStateGuardian8.h"
#include "dxSavedFrameBuffer8.h"
#include "dxTextureContext8.h"

#include <dconfig.h>

Configure(config_dxgsg);
NotifyCategoryDef(dxgsg, ":display:gsg");

// Configure this variable true to cause the DXGSG to show each
// transform space it renders by drawing a little unit axis.  This
// cannot be enabled when the player is compiled in NDEBUG mode.
bool dx_show_transforms = config_dxgsg.GetBool("dx-show-transforms", false);

//  Configure this to TRUE if you want DirectX to control the entire screen,
//  If false, it will just blit into a window.
bool dx_full_screen = config_dxgsg.GetBool("dx-full-screen", false);

//  Configure this true to force the rendering to sync to the video
//  refresh, or false to let your frame rate go as high as it can,
//  irrespective of the video refresh.
bool dx_sync_video = config_dxgsg.GetBool("sync-video", true);

// Set Level of MultiSampling to be used, if HW supports it.  Valid values are 2-16.
DWORD dx_multisample_antialiasing_level = (DWORD) config_dxgsg.GetInt("dx-multisample-antialiasing-level", 0);

// Configure this true to perform a cull traversal over the geometry
// by default, false otherwise.  The cull traversal provides support
// for state-sorting, z-sorting, and binning.
bool dx_cull_traversal = config_dxgsg.GetBool("dx-cull-traversal", true);

// if true, if card only supports per-vertex fog, it will be treated as no-HW fog capability
bool dx_no_vertex_fog = config_dxgsg.GetBool("dx-no-vertex-fog", false);

// if true, overwrite cursor bitmap tip with "D3D" to distinguish it from GDI cursor 
bool dx_show_cursor_watermark = config_dxgsg.GetBool("dx-show-cursor-watermark", 
#ifdef _DEBUG
    true
#else
    false
#endif
    );

// if true, triangle filter will be used to generate mipmap levels instead of default box filter
bool dx_use_triangle_mipgen_filter = config_dxgsg.GetBool("dx-use-triangle-mipgen-filter", false);

// Configure this true to cause all lighting normals to automatically
// be normalized by the CPU before rendering.  This is
// necessary if you intend to render things under scale transforms and
// expect lighting to work correctly.  Maybe one day there will be
// another way to set this at runtime, instead of only as a configure
// variable
bool dx_auto_normalize_lighting = config_dxgsg.GetBool("auto-normalize-lighting", false);

bool dx_show_fps_meter = config_dxgsg.GetBool("show-fps-meter", false);
float dx_fps_meter_update_interval = max(0.5,config_dxgsg.GetFloat("fps-meter-update-interval", 1.7));

#ifndef NDEBUG
// debugging flag
// values are same as D3DCULL enumtype, 0 - no force, 1 - force none, 2 - force CW, 3 - force CCW
int dx_force_backface_culling = config_dxgsg.GetInt("dx-force-backface-culling", 0);
#endif

bool dx_mipmap_everything = config_dxgsg.GetBool("dx-mipmap-everything", false);
bool dx_ignore_mipmaps = config_dxgsg.GetBool("dx-ignore-mipmaps", false);

// if this is set, more accurate but more expensive fog computations are performed
bool dx_use_rangebased_fog = config_dxgsg.GetBool("dx-use-rangebased-fog", false);
bool dx_force_16bpptextures = config_dxgsg.GetBool("dx-force-16bpptextures", false);
bool dx_no_dithering = config_dxgsg.GetBool("dx-no-dithering", false);

#ifdef _DEBUG
float dx_global_miplevel_bias = config_dxgsg.GetFloat("dx-global-miplevel-bias", 0.0);
bool dx_debug_view_mipmaps = config_dxgsg.GetBool("dx-debug-view-mipmaps", false);
//int dx_print_texstats = config_dxgsg.GetBool("dx-print-texstats", 0);
#endif

// use dx8 or GDI mouse cursor in fullscreen mode?
// Nvidia dx8 cursor is invisible as of 28.32 drivers, so using GDI in fullscrn by default for now
bool dx_use_dx_cursor = config_dxgsg.GetBool("dx-use-dx-cursor", false);

bool dx_force_anisotropic_filtering = config_dxgsg.GetBool("dx-force-anisotropic-filtering", false);

// set 'retained-mode #t' and this to have prepare_geom concatenate all tristrips within a geom 
// together using degenerate tris
const bool link_tristrips = config_dxgsg.GetBool("link-tristrips", false);

// note:  offset currently disabled since it wasnt working properly
DXDecalType dx_decal_type = GDT_mask;

static DXDecalType
parse_decal_type(const string &type) {
  if (type == "mask") {
    return GDT_mask;
  } else if (type == "blend") {
    return GDT_blend;
  } else if (type == "offset") {
    return GDT_offset;
  }
  dxgsg_cat.error() << "Invalid dx-decal-type: " << type << "\n";
  return GDT_mask;
}

ConfigureFn(config_dxgsg) {
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

  string decal_type = config_dxgsg.GetString("dx-decal-type", "");
  if (!decal_type.empty()) {
    dx_decal_type = parse_decal_type(decal_type);
  }

  DXGraphicsStateGuardian::init_type();
  DXSavedFrameBuffer::init_type();
  DXTextureContext::init_type();
  DXGeomNodeContext::init_type();

  GraphicsStateGuardian::get_factory().register_factory
    (DXGraphicsStateGuardian::get_class_type(),
     DXGraphicsStateGuardian::make_DXGraphicsStateGuardian);
}

const char *D3DFormatStr(D3DFORMAT fmt) {

#define CASESTR(XX) case XX: return #XX;

  switch(fmt) {
    CASESTR(D3DFMT_UNKNOWN);
    CASESTR(D3DFMT_R8G8B8);
    CASESTR(D3DFMT_A8R8G8B8);
    CASESTR(D3DFMT_X8R8G8B8);
    CASESTR(D3DFMT_R5G6B5);
    CASESTR(D3DFMT_X1R5G5B5);
    CASESTR(D3DFMT_A1R5G5B5);
    CASESTR(D3DFMT_A4R4G4B4);
    CASESTR(D3DFMT_R3G3B2);
    CASESTR(D3DFMT_A8);
    CASESTR(D3DFMT_A8R3G3B2);
    CASESTR(D3DFMT_X4R4G4B4);
    CASESTR(D3DFMT_A2B10G10R10);
    CASESTR(D3DFMT_G16R16);
    CASESTR(D3DFMT_A8P8);
    CASESTR(D3DFMT_P8);
    CASESTR(D3DFMT_L8);
    CASESTR(D3DFMT_A8L8);
    CASESTR(D3DFMT_A4L4);
    CASESTR(D3DFMT_V8U8);
    CASESTR(D3DFMT_L6V5U5);
    CASESTR(D3DFMT_X8L8V8U8);
    CASESTR(D3DFMT_Q8W8V8U8);
    CASESTR(D3DFMT_V16U16);
    CASESTR(D3DFMT_W11V11U10);
    CASESTR(D3DFMT_A2W10V10U10);
    CASESTR(D3DFMT_UYVY);
    CASESTR(D3DFMT_YUY2);
    CASESTR(D3DFMT_DXT1);
    CASESTR(D3DFMT_DXT2);
    CASESTR(D3DFMT_DXT3);
    CASESTR(D3DFMT_DXT4);
    CASESTR(D3DFMT_DXT5);
    CASESTR(D3DFMT_D16_LOCKABLE);
    CASESTR(D3DFMT_D32);
    CASESTR(D3DFMT_D15S1);
    CASESTR(D3DFMT_D24S8);
    CASESTR(D3DFMT_D16);
    CASESTR(D3DFMT_D24X8);
    CASESTR(D3DFMT_D24X4S4);
    CASESTR(D3DFMT_VERTEXDATA);
    CASESTR(D3DFMT_INDEX16);
    CASESTR(D3DFMT_INDEX32);
  }

  return "Invalid D3DFORMAT";
}

