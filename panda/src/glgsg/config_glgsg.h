// Filename: config_glgsg.h
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

#ifndef CONFIG_GLGSG_H
#define CONFIG_GLGSG_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(glgsg, EXPCL_PANDAGL, EXPTP_PANDAGL);

extern bool gl_show_transforms;
extern bool gl_cheap_textures;
extern bool gl_cull_traversal;
extern bool gl_ignore_mipmaps;
extern bool gl_force_mipmaps;
extern bool gl_show_mipmaps;
extern bool gl_save_mipmaps;
extern bool gl_auto_normalize_lighting;
extern bool gl_depth_offset_decals;
extern bool gl_supports_bgr;

// Ways to implement decals.
enum GLDecalType {
  GDT_mask,   // GL 1.0 style, involving three steps
  GDT_blend,  // As above, but slower; a hack for broken nVidia driver
  GDT_offset  // The fastest, using GL 1.1 style glPolygonOffset
};
extern GLDecalType gl_decal_type;


extern EXPCL_PANDAGL void init_libglgsg();

#endif
