// Filename: config_dxgsg8.cxx
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
#include "graphicsPipeSelection.h"
#include "wdxGraphicsWindow8.h"
#include "wdxGraphicsPipe8.h"

#include <dconfig.h>

Configure(config_dxgsg8);
//NotifyCategoryDef(dxgsg8, ":display:gsg");  dont want to merge this with the regular parent class dbg output
NotifyCategoryDef(dxgsg8, "dxgsg");
NotifyCategoryDef(wdxdisplay8, "windisplay");

// Configure this variable true to cause the DXGSG to show each
// transform space it renders by drawing a little unit axis.  This
// cannot be enabled when the player is compiled in NDEBUG mode.
bool dx_show_transforms = config_dxgsg8.GetBool("dx-show-transforms", false);

//  Configure this to TRUE if you want DirectX to control the entire screen,
//  If false, it will just blit into a window.
bool dx_full_screen = config_dxgsg8.GetBool("dx-full-screen", false);

//  Configure this true to force the rendering to sync to the video
//  refresh, or false to let your frame rate go as high as it can,
//  irrespective of the video refresh.
bool dx_sync_video = config_dxgsg8.GetBool("sync-video", true);

// Set Level of MultiSampling to be used, if HW supports it.  Valid values are 2-16.
DWORD dx_multisample_antialiasing_level = (DWORD) config_dxgsg8.GetInt("dx-multisample-antialiasing-level", 0);

// Configure this true to perform a cull traversal over the geometry
// by default, false otherwise.  The cull traversal provides support
// for state-sorting, z-sorting, and binning.
bool dx_cull_traversal = config_dxgsg8.GetBool("dx-cull-traversal", true);

// if true, if card only supports per-vertex fog, it will be treated as no-HW fog capability
bool dx_no_vertex_fog = config_dxgsg8.GetBool("dx-no-vertex-fog", false);

// if true, overwrite cursor bitmap tip with "D3D" to distinguish it from GDI cursor 
bool dx_show_cursor_watermark = config_dxgsg8.GetBool("dx-show-cursor-watermark", 
#ifdef _DEBUG
    true
#else
    false
#endif
    );

// if true, triangle filter will be used to generate mipmap levels instead of default box filter
bool dx_use_triangle_mipgen_filter = config_dxgsg8.GetBool("dx-use-triangle-mipgen-filter", false);

// Configure this true to cause all lighting normals to automatically
// be normalized by the CPU before rendering.  This is
// necessary if you intend to render things under scale transforms and
// expect lighting to work correctly.  Maybe one day there will be
// another way to set this at runtime, instead of only as a configure
// variable
bool dx_auto_normalize_lighting = config_dxgsg8.GetBool("auto-normalize-lighting", true);

bool dx_show_fps_meter = config_dxgsg8.GetBool("show-fps-meter", false);
float dx_fps_meter_update_interval = max(0.5,config_dxgsg8.GetFloat("fps-meter-update-interval", 1.7));

#ifndef NDEBUG
// debugging flag
// values are same as D3DCULL enumtype, 0 - no force, 1 - force none, 2 - force CW, 3 - force CCW
int dx_force_backface_culling = config_dxgsg8.GetInt("dx-force-backface-culling", 0);
#endif

bool dx_mipmap_everything = config_dxgsg8.GetBool("dx-mipmap-everything", false);
bool dx_ignore_mipmaps = config_dxgsg8.GetBool("dx-ignore-mipmaps", false);

// if this is set, more accurate but more expensive fog computations are performed
bool dx_use_rangebased_fog = config_dxgsg8.GetBool("dx-use-rangebased-fog", false);
bool dx_force_16bpptextures = config_dxgsg8.GetBool("dx-force-16bpptextures", false);
bool dx_no_dithering = config_dxgsg8.GetBool("dx-no-dithering", false);
bool dx_force_16bpp_zbuffer = config_dxgsg8.GetBool("dx-force-16bpp-zbuffer", false);
bool dx_do_vidmemsize_check = config_dxgsg8.GetBool("do-vidmemsize-check", true);
// Setting this true theoretically hinders render performance, because
// it forces the FPU to go through some extra work to clean itself up
// after rendering a frame, but the performance cost seems to be
// small.  On the other hand, setting it false can force the
// application to run in single-precision arithmetic mode, even if
// it believes it is using double-precision variables.
bool dx_preserve_fpu_state = config_dxgsg8.GetBool("dx-preserve-fpu-state", true);

// if true, override win-width/height and use driver vidmem info to
// pick what will be a fullscreen window size close to the best perf
// capability of card, based on a heuristic
bool dx_pick_best_screenres = config_dxgsg8.GetBool("pick-best-screenres", false);

int dx_preferred_device_id = config_dxgsg8.GetInt("dx-preferred-device-id", -1);

#ifdef _DEBUG
float dx_global_miplevel_bias = config_dxgsg8.GetFloat("dx-global-miplevel-bias", 0.0);
bool dx_debug_view_mipmaps = config_dxgsg8.GetBool("dx-debug-view-mipmaps", false);
//int dx_print_texstats = config_dxgsg8.GetBool("dx-print-texstats", 0);
#endif

// use dx8 or GDI mouse cursor in fullscreen mode?
// Nvidia dx8 cursor is invisible as of 28.32 drivers, so using GDI in fullscrn by default for now
bool dx_use_dx_cursor = config_dxgsg8.GetBool("dx-use-dx-cursor", false);

bool dx_force_anisotropic_filtering = config_dxgsg8.GetBool("dx-force-anisotropic-filtering", false);

// set 'retained-mode #t' and this to have prepare_geom concatenate all tristrips within a geom 
// together using degenerate tris
const bool link_tristrips = config_dxgsg8.GetBool("link-tristrips", false);

// note:  offset currently disabled since it wasnt working properly
DXDecalType dx_decal_type = GDT_mask;

// Note: must be a ptr not a regular string because the init-string constructor for
// a global/static string variable will run AFTER the dll static init fn
// init_libdxgsg8(), which means the string will be reset to "" after we read it in
string *pdx_vertexshader_filename=NULL;
string *pdx_pixelshader_filename=NULL;

// texture file to be set globally, usually for special FX
string *pdx_globaltexture_filename=NULL;
// tex stagenum to set the global tex to
UINT dx_globaltexture_stagenum = (UINT) config_dxgsg8.GetInt("dx-globaltexture-stagenum", 0);

static DXDecalType
parse_decal_type(const string &type) {
  if (type == "mask") {
    return GDT_mask;
  } else if (type == "blend") {
    return GDT_blend;
  } else if (type == "offset") {
    return GDT_offset;
  }
  dxgsg8_cat.error() << "Invalid dx-decal-type: " << type << "\n";
  return GDT_mask;
}

ConfigureFn(config_dxgsg8) {
  init_libdxgsg8();
}

void init_config_string(string *&pFname,const char *ConfigrcVarname) {
  // pFname is reference to string ptr

  // dont try to take the & of a soon-to-be-gone stack var string, this must be on the heap!
  pFname = new string(config_dxgsg8.GetString(ConfigrcVarname, ""));
  if(pFname->empty()) {
      delete pFname;
      pFname=NULL;
  }
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

  string decal_type = config_dxgsg8.GetString("dx-decal-type", "");
  if (!decal_type.empty()) {
    dx_decal_type = parse_decal_type(decal_type);
  }

  init_config_string(pdx_vertexshader_filename,"dx-vertexshader-filename");
  init_config_string(pdx_pixelshader_filename,"dx-pixelshader-filename");
  init_config_string(pdx_globaltexture_filename,"dx-globaltexture-filename");

  DXGraphicsStateGuardian8::init_type();
  DXSavedFrameBuffer8::init_type();
  DXTextureContext8::init_type();
  DXGeomNodeContext8::init_type();

  wdxGraphicsPipe8::init_type();
  wdxGraphicsWindow8::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(wdxGraphicsPipe8::get_class_type(),
                           wdxGraphicsPipe8::pipe_constructor);

}
