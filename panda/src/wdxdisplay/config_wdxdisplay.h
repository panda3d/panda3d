// Filename: config_wdxdisplay.h
// Created by:  mike (07Oct99)
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

#ifndef __CONFIG_WDXDISPLAY_H__
#define __CONFIG_WDXDISPLAY_H__

#include <pandabase.h>
#include <filename.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(wdxdisplay, EXPCL_PANDADX, EXPTP_PANDADX);

extern bool bResponsive_minimized_fullscreen_window;
extern bool dx_force_16bpp_zbuffer;
extern Filename get_icon_filename();
extern Filename get_mono_cursor_filename();
extern Filename get_color_cursor_filename();

extern EXPCL_PANDADX void init_libwdxdisplay();

#endif /* __CONFIG_WDXDISPLAY_H__ */
