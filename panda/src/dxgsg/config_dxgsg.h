// Filename: config_dxgsg.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_DXGSG_H
#define CONFIG_DXGSG_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(dxgsg, EXPCL_PANDADX, EXPTP_PANDADX);

extern bool dx_show_transforms;
extern bool dx_full_screen;
extern bool dx_cheap_textures;
extern bool dx_cull_traversal;

// Ways to implement decals.
enum DXDecalType {
  GDT_mask,   // GL 1.0 style, involving three steps
  GDT_blend,  // As above, but slower; a hack for broken nVidia driver
  GDT_offset  // The fastest, using GL 1.1 style glPolygonOffset
};
extern DXDecalType dx_decal_type;


#endif
