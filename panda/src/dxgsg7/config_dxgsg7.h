// Filename: config_dxgsg7.h
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

#ifndef CONFIG_DXGSG7_H
#define CONFIG_DXGSG7_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"

NotifyCategoryDecl(dxgsg7, EXPCL_PANDADX, EXPTP_PANDADX);
NotifyCategoryDecl(wdxdisplay7, EXPCL_PANDADX, EXPTP_PANDADX);

extern ConfigVariableBool dx_full_screen;
extern ConfigVariableBool dx_no_vertex_fog;
extern ConfigVariableBool dx_full_screen_antialiasing;
extern ConfigVariableBool dx_use_rangebased_fog;
extern ConfigVariableBool link_tristrips;

// debug flags we might want to use in full optimized build
extern ConfigVariableBool dx_ignore_mipmaps;
extern ConfigVariableBool dx_mipmap_everything;
extern ConfigVariableBool dx_force_16bpptextures;
extern ConfigVariableBool dx_no_dithering;
extern ConfigVariableBool dx_force_16bpp_zbuffer;
extern ConfigVariableBool dx_do_vidmemsize_check;
extern ConfigVariableBool dx_preserve_fpu_state;

#ifndef NDEBUG
extern ConfigVariableInt dx_force_backface_culling;
#endif

extern ConfigVariableBool dx_broken_max_index;

#ifdef _DEBUG
extern ConfigVariableDouble dx_global_miplevel_bias;
extern ConfigVariableBool dx_debug_view_mipmaps;
extern ConfigVariableBool dx_force_anisotropic_filtering;
#endif

extern EXPCL_PANDADX void init_libdxgsg7();

#endif
