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
extern bool dx_cull_traversal;
extern bool dx_ignore_mipmaps;

#ifdef _DEBUG
extern float dx_global_miplevel_bias;
extern bool dx_debug_view_mipmaps;
extern bool dx_mipmap_everything;
extern bool dx_force_16bpptextures;
extern bool dx_force_anisotropic_filtering;
#endif

// Ways to implement decals.
enum DXDecalType {
  GDT_mask,   // GL 1.0 style, involving three steps and double-draw of polygon
  GDT_blend,  // As above, but slower; use blending to disable colorbuffer writes
  GDT_offset  // The fastest, using GL 1.1 style glPolygonOffset
};
extern DXDecalType dx_decal_type;

extern EXPCL_PANDADX void init_libdxgsg();

#endif
