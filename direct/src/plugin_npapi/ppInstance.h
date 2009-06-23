// Filename: ppInstance.h
// Created by:  drose (19Jun09)
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

#ifndef PPINSTANCE_H
#define PPINSTANCE_H

#include "nppanda3d_common.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : PPInstance
// Description : This represents a single instance of the Panda3D
//               plugin, via the NPAPI interface.  This instance
//               brokers the communication with the P3D Core API, as
//               defined in the plugin directory.
////////////////////////////////////////////////////////////////////
class PPInstance {
public:
  PPInstance(NPMIMEType pluginType, NPP instance, uint16 mode, 
             int16 argc, char *argn[], char *argv[], NPSavedData *saved);
  ~PPInstance();

  void set_window(NPWindow *window);

private:
  void create_instance();
  void send_window();

private:
  typedef vector<P3D_token> Tokens;
  Tokens _tokens;
  unsigned int _npp_mode;

  bool _got_window;
  NPWindow _window;

  P3D_instance *_inst;
};

#include "ppInstance.I"

#endif
