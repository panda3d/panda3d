// Filename: config_wcrdisplay.h
// Created by:  skyler, based on wgl* file.
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

#ifndef __CONFIG_WCRDISPLAY_H__
#define __CONFIG_WCRDISPLAY_H__

#include "pandabase.h"
#include "filename.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(wcrdisplay, EXPCL_PANDACR, EXPTP_PANDACR);

// for some reason there is a conflict with the pandadx versions of get_icon_filename during linking,
// so appended '_2' to name
extern Filename get_icon_filename_2();
extern Filename get_color_cursor_filename_2();
extern Filename get_mono_cursor_filename_2();

extern bool gl_show_fps_meter;
extern float gl_fps_meter_update_interval;
extern bool gl_sync_video;
extern int gl_forced_pixfmt;
extern bool bResponsive_minimized_fullscreen_window;
extern bool support_wiregl;

extern EXPCL_PANDACR void init_libwcrdisplay();

#endif /* __CONFIG_WCRDISPLAY_H__ */
