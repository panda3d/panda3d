// Filename: config_dxgsg.cxx
// Created by:  drose (06Oct99)

#include "config_dxgsg.h"
#include "dxGraphicsStateGuardian.h"
#include "dxSavedFrameBuffer.h"
#include "dxTextureContext.h"

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

// Configure this true to glHint the textures into the cheapest
// possible mode.  (this does nothing in dx currently)
bool dx_cheap_textures = config_dxgsg.GetBool("dx-cheap-textures", false);

// Configure this true to perform a cull traversal over the geometry
// by default, false otherwise.  The cull traversal provides support
// for state-sorting, z-sorting, and binning.
bool dx_cull_traversal = config_dxgsg.GetBool("dx-cull-traversal", true);

bool dx_ignore_mipmaps = config_dxgsg.GetBool("dx-ignore-mipmaps", false);
float dx_global_miplevel_bias = config_dxgsg.GetFloat("dx-global-miplevel-bias", 0.0);

#ifdef _DEBUG
bool dx_debug_view_mipmaps = config_dxgsg.GetBool("dx-debug-view-mipmaps", false);
bool dx_force_16bpptextures = config_dxgsg.GetBool("dx-force-16bpptextures", false);
bool dx_mipmap_everything = config_dxgsg.GetBool("dx-mipmap-everything", false);
bool dx_force_anisotropic_filtering = config_dxgsg.GetBool("dx-force-anisotropic-filtering", false);

#endif

// note:  offset currently disabled since it wasnt working properly
DXDecalType dx_decal_type = GDT_offset;

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
  return GDT_offset;
}

ConfigureFn(config_dxgsg) {

  string decal_type = config_dxgsg.GetString("dx-decal-type", "");
  if (!decal_type.empty()) {
    dx_decal_type = parse_decal_type(decal_type);
  }

  DXGraphicsStateGuardian::init_type();
  DXSavedFrameBuffer::init_type();
  DXTextureContext::init_type();

  GraphicsStateGuardian::_factory.register_factory
    (DXGraphicsStateGuardian::get_class_type(),
     DXGraphicsStateGuardian::make_DXGraphicsStateGuardian);
}



