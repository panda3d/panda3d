// Filename: config_gobj.h
// Created by:  drose (01Oct99)
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

#ifndef CONFIG_GOBJ_H
#define CONFIG_GOBJ_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableEnum.h"
#include "configVariableDouble.h"

NotifyCategoryDecl(gobj, EXPCL_PANDA, EXPTP_PANDA);

enum BamTextureMode {
  BTM_unchanged,
  BTM_fullpath,
  BTM_relative,
  BTM_basename,
  BTM_rawdata
};
EXPCL_PANDA ostream &operator << (ostream &out, BamTextureMode btm);
EXPCL_PANDA istream &operator >> (istream &in, BamTextureMode &btm);

enum AutoTextureScale {
  ATS_none,
  ATS_down,
  ATS_up
};
EXPCL_PANDA ostream &operator << (ostream &out, AutoTextureScale ats);
EXPCL_PANDA istream &operator >> (istream &in, AutoTextureScale &ats);

// Configure variables for gobj package.
extern EXPCL_PANDA ConfigVariableInt max_texture_dimension;
extern EXPCL_PANDA ConfigVariableBool keep_texture_ram;
extern EXPCL_PANDA ConfigVariableBool keep_geom_ram;
extern EXPCL_PANDA ConfigVariableBool retained_mode;
extern EXPCL_PANDA ConfigVariableBool vertex_buffers;
extern EXPCL_PANDA ConfigVariableBool display_lists;
extern EXPCL_PANDA ConfigVariableBool hardware_animated_vertices;
extern EXPCL_PANDA ConfigVariableBool matrix_palette;
extern EXPCL_PANDA ConfigVariableBool display_list_animation;
extern EXPCL_PANDA ConfigVariableBool connect_triangle_strips;

extern EXPCL_PANDA ConfigVariableBool use_qpgeom;
extern EXPCL_PANDA ConfigVariableBool support_old_geom;

extern EXPCL_PANDA ConfigVariableEnum<BamTextureMode> bam_texture_mode;
extern EXPCL_PANDA ConfigVariableEnum<AutoTextureScale> textures_power_2;
extern EXPCL_PANDA ConfigVariableEnum<AutoTextureScale> textures_square;
extern EXPCL_PANDA ConfigVariableString fake_texture_image;

extern EXPCL_PANDA ConfigVariableInt geom_cache_size;
extern EXPCL_PANDA ConfigVariableInt geom_cache_min_frames;

extern ConfigVariableDouble default_near;
extern ConfigVariableDouble default_far;
extern ConfigVariableDouble default_fov;
extern ConfigVariableDouble default_keystone;

#endif


