// Filename: config_windisplay.h
// Created by:  drose (20Dec02)
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

#ifndef CONFIG_WINDISPLAY_H
#define CONFIG_WINDISPLAY_H

#include "pandabase.h"
#include "filename.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(windisplay, EXPCL_PANDAWIN, EXPTP_PANDAWIN);

extern Filename get_icon_filename();
extern Filename get_color_cursor_filename();
extern Filename get_mono_cursor_filename();

extern bool show_fps_meter;
extern float fps_meter_update_interval;
extern bool responsive_minimized_fullscreen_window;
extern bool hold_keys_across_windows;
extern bool do_vidmemsize_check;
extern bool ime_composition_w;
extern bool ime_aware;
extern bool ime_hide;

extern EXPCL_PANDAWIN bool sync_video;
extern EXPCL_PANDAWIN bool swapbuffer_framelock;
extern EXPCL_PANDAWIN bool force_software_renderer;
extern EXPCL_PANDAWIN bool allow_software_renderer;

extern EXPCL_PANDAWIN void init_libwindisplay();

#endif
