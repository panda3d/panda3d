// Filename: framework.h
// Created by:  drose (04May00)
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

EXPCL_FRAMEWORK extern EventHandler event_handler;
EXPCL_FRAMEWORK extern PT_NamedNode lights;
EXPCL_FRAMEWORK extern PT_NamedNode root;
EXPCL_FRAMEWORK extern PT_NamedNode data_root;
EXPCL_FRAMEWORK extern PT_NamedNode render;
EXPCL_FRAMEWORK extern PT_NamedNode cameras;
EXPCL_FRAMEWORK extern PT(MouseAndKeyboard) mak;
EXPCL_FRAMEWORK extern PT(MouseWatcher) mouse_watcher;
EXPCL_FRAMEWORK extern PT(Trackball) trackball;
EXPCL_FRAMEWORK extern PT(GraphicsWindow) main_win;
EXPCL_FRAMEWORK extern PT(DirectionalLight) dlight;
EXPCL_FRAMEWORK extern bool have_dlight;
EXPCL_FRAMEWORK extern Loader loader;

EXPCL_FRAMEWORK extern void set_alt_trackball(Node*);
EXPCL_FRAMEWORK extern int framework_main(int argc, char *argv[]);
EXPCL_FRAMEWORK extern void (*define_keys)(EventHandler&);
EXPCL_FRAMEWORK extern void (*additional_idle)();
EXPCL_FRAMEWORK extern void (*extra_overrides_func)(ChanCfgOverrides&,
						    std::string&);

#endif
