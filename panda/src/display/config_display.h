// Filename: config_display.h
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

#ifndef CONFIG_DISPLAY_H
#define CONFIG_DISPLAY_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>
#include <dconfig.h>

#include <string>
#include "pvector.h"

NotifyCategoryDecl(display, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(gsg, EXPCL_PANDA, EXPTP_PANDA);

extern const string pipe_spec_machine;
extern const string pipe_spec_filename;
extern const int pipe_spec_pipe_number;
extern const bool pipe_spec_is_file;
extern const bool pipe_spec_is_remote;

extern const bool compare_state_by_pointer;

extern const float gsg_clear_r;
extern const float gsg_clear_g;
extern const float gsg_clear_b;

extern Config::ConfigTable::Symbol::iterator pipe_modules_begin(void);
extern Config::ConfigTable::Symbol::iterator pipe_modules_end(void);
extern Config::ConfigTable::Symbol::iterator gsg_modules_begin(void);
extern Config::ConfigTable::Symbol::iterator gsg_modules_end(void);

extern Config::ConfigTable::Symbol::iterator preferred_pipe_begin();
extern Config::ConfigTable::Symbol::iterator preferred_pipe_end();
extern Config::ConfigTable::Symbol::iterator preferred_window_begin();
extern Config::ConfigTable::Symbol::iterator preferred_window_end();
extern Config::ConfigTable::Symbol::iterator preferred_gsg_begin();
extern Config::ConfigTable::Symbol::iterator preferred_gsg_end();

extern EXPCL_PANDA void init_libdisplay();

#endif /* CONFIG_DISPLAY_H */
