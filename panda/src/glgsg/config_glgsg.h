// Filename: config_glgsg.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_GLGSG_H
#define CONFIG_GLGSG_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(glgsg, EXPCL_PANDAGL, EXPTP_PANDAGL);

extern bool gl_show_transforms;
extern bool gl_cheap_textures;
extern bool gl_cull_traversal;
extern bool gl_ignore_mipmaps;

// Ways to implement decals.
enum GLDecalType {
  GDT_mask,   // GL 1.0 style, involving three steps
  GDT_blend,  // As above, but slower; a hack for broken nVidia driver
  GDT_offset  // The fastest, using GL 1.1 style glPolygonOffset
};
extern GLDecalType gl_decal_type;

#endif
