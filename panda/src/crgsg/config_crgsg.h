// Filename: config_crgsg.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_GLGSG_H
#define CONFIG_GLGSG_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(crgsg, EXPCL_PANDACR, EXPTP_PANDACR);

extern bool cr_show_transforms;
extern bool cr_cheap_textures;
extern bool cr_cull_traversal;
extern bool cr_ignore_mipmaps;
extern bool cr_force_mipmaps;
extern bool cr_show_mipmaps;
extern bool cr_save_mipmaps;
extern bool cr_auto_normalize_lighting;
extern bool cr_depth_offset_decals;
extern bool cr_supports_bgr;

// Ways to implement decals.
enum CRDecalType {
  GDT_mask,   // GL 1.0 style, involving three steps
  GDT_blend,  // As above, but slower; a hack for broken nVidia driver
  GDT_offset  // The fastest, using GL 1.1 style chromium.PolygonOffset
};
extern CRDecalType cr_decal_type;


extern EXPCL_PANDACR void init_libcrgsg();

#endif
