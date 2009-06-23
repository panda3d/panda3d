// Filename: ppInstance.cxx
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

#include "ppInstance.h"

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::Constructor
//       Access: Public
//  Description: Creates a new instance of a Panda3D plugin window.
//               The create_data structure is supplied from NPAPI, and
//               defines the initial parameters specified in the HTML
//               document.
////////////////////////////////////////////////////////////////////
PPInstance::
PPInstance(NPMIMEType pluginType, NPP instance, uint16 mode, 
           int16 argc, char *argn[], char *argv[], NPSavedData *saved) {
  logfile << "constructing " << this << "\n" << flush;
  _inst = NULL;

  // Copy the tokens and save them within this object.
  _tokens.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    P3D_token token;
    token._keyword = strdup(argn[i]);
    token._value = strdup(argv[i]);
    logfile
      << " " << i << ": " << token._keyword << " = " << token._value << "\n";
    _tokens.push_back(token);
  }

  _npp_mode = mode;
  _got_window = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPInstance::
~PPInstance() {
  logfile
    << "destructing " << this << "\n" << flush;

  if (_inst != NULL) {
    P3D_instance_finish(_inst);
    _inst = NULL;
  }

  // Free the tokens we allocated.
  Tokens::iterator ti;
  for (ti = _tokens.begin(); ti != _tokens.end(); ++ti) {
    free((char *)(*ti)._keyword);
    free((char *)(*ti)._value);
  }
  _tokens.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::set_window
//       Access: Public
//  Description: Stores or updates the window parameters.
////////////////////////////////////////////////////////////////////
void PPInstance::
set_window(NPWindow *window) {
  if (window->x == _window.x &&
      window->y == _window.y &&
      window->width == _window.width &&
      window->height == _window.height) {
    // No changes.
    return;
  }

  _window = *window;
  _got_window = true;
  
  if (_inst != NULL) {
    send_window();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::create_instance
//       Access: Private
//  Description: Actually creates the internal P3D_instance object.
////////////////////////////////////////////////////////////////////
void PPInstance::
create_instance() {
  assert(_inst == NULL);
  const P3D_token *tokens = NULL;
  if (!_tokens.empty()) {
    tokens = &_tokens[0];
  }

  _inst = P3D_create_instance
    (NULL, NULL, tokens, _tokens.size());

  if (_inst != NULL && _got_window) {
    send_window();
  }
}

  
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::send_window
//       Access: Private
//  Description: Actually issues the window parameters to the internal
//               P3D_instance object.
////////////////////////////////////////////////////////////////////
void PPInstance::
send_window() {
  assert(_inst != NULL);

  P3D_window_handle parent_window;
#ifdef _WIN32
  parent_window._hwnd = (HWND)(_window.window);
#endif

  P3D_instance_setup_window
    (_inst, P3D_WT_embedded,
     _window.x, _window.y, _window.width, _window.height,
     parent_window);
}
