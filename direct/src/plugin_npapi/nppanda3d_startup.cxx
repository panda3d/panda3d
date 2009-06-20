// Filename: nppanda3d_startup.cxx
// Created by:  drose (17Jun09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "nppanda3d_startup.h"
#include "ppInstance.h"

// These source files are part of the Gecko SDK, sort of.  They're
// distributed with the sample applications.  They provide a
// higher-level wrapper around some of the NPAPI startup stuff, and
// this appears to be the intended way to use the Gecko SDK.  It's a
// weird system.
#include "npplat.h"
#include "pluginbase.h"
#include "np_entry.cpp"
#include "npn_gate.cpp"
#include "npp_gate.cpp"

#include "../plugin/load_plugin_src.cxx"

ofstream log;

NPError
NS_PluginInitialize() {
  log.open("c:/cygwin/home/drose/t.log");
  log << "initializing\n" << flush;

  if (!load_plugin("")) {
    log << "couldn't load plugin\n" << flush;
    return NPERR_INVALID_PLUGIN_ERROR;
  }

  return NPERR_NO_ERROR;
}

void
NS_PluginShutdown() {
  log << "shutdown\n" << flush;
#ifdef _WIN32
  FreeLibrary(module);
  module = NULL;
#endif
}

nsPluginInstanceBase *
NS_NewPluginInstance(nsPluginCreateData *create_data) {
  log << "new instance\n" << flush;
  return new PPInstance(create_data);
}

void
NS_DestroyPluginInstance(nsPluginInstanceBase *plugin) {
  log << "destroy instance\n" << flush;
  delete (PPInstance *)plugin;
}
