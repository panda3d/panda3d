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
// possible mode.
bool dx_cheap_textures = config_dxgsg.GetBool("dx-cheap-textures", false);

// Configure this true to perform a cull traversal over the geometry
// by default, false otherwise.  The cull traversal provides support
// for state-sorting, z-sorting, and binning.
bool dx_cull_traversal = config_dxgsg.GetBool("dx-cull-traversal", true);

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



