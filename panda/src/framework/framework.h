// Filename: framework.h
// Created by:  drose (04May00)
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

#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include <pandabase.h>

#include <event.h>
#include <eventHandler.h>
#include <namedNode.h>
#include <directionalLight.h>
#include <mouse.h>
#include <trackball.h>
#include <pointerTo.h>
#include <pt_NamedNode.h>
#include <loader.h>
#include <mouseWatcher.h>
#include <chancfg.h>

extern EventHandler event_handler;
extern PT_NamedNode lights;
extern PT_NamedNode root;
extern PT_NamedNode data_root;
extern PT_NamedNode render;
extern PT_NamedNode cameras;
extern PT(MouseAndKeyboard) mak;
extern PT(MouseWatcher) mouse_watcher;
extern PT(Trackball) trackball;
extern PT(GraphicsWindow) main_win;
extern PT(DirectionalLight) dlight;
extern bool have_dlight;
extern Loader loader;

extern void set_alt_trackball(Node*);
extern int framework_main(int argc, char *argv[]);
extern void (*define_keys)(EventHandler&);
extern void (*additional_idle)();
extern void (*extra_overrides_func)(ChanCfgOverrides&, std::string&);

#endif
