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
PPInstance(nsPluginCreateData *create_data) {
  log << "constructing " << this << "\n" << flush;
  _inst = NULL;

  log << "  instance = " << create_data->instance
      << "\n  type = " << create_data->type
      << "\n  mode = " << create_data->mode << "\n";

  // Copy the tokens from the create_data structure.
  _tokens.reserve(create_data->argc);
  for (int i = 0; i < create_data->argc; ++i) {
    P3D_token token;
    token._keyword = strdup(create_data->argn[i]);
    token._value = strdup(create_data->argv[i]);
    log << " " << i << ": " << token._keyword << " = " << token._value << "\n";
    _tokens.push_back(token);
  }

  _npp_mode = create_data->mode;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPInstance::
~PPInstance() {
  assert(_inst == NULL);
  log << "destructing " << this << "\n" << flush;

  Tokens::iterator ti;
  for (ti = _tokens.begin(); ti != _tokens.end(); ++ti) {
    free((char *)(*ti)._keyword);
    free((char *)(*ti)._value);
  }
  _tokens.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::init
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NPBool PPInstance::
init(NPWindow *window) {
  assert(_inst == NULL);
  log << "init, window = " << window << "\n" << flush;
  log << " x,y = " << window->x << "," << window->y
      << " w,h = " << window->width << "," << window->height
      << "\n" << flush;

  P3D_window_handle parent_window;
#ifdef _WIN32
  parent_window._hwnd = (HWND)(window->window);
#endif

  P3D_window_type window_type = P3D_WT_embedded;

  const P3D_token *tokens = NULL;
  if (!_tokens.empty()) {
    tokens = &_tokens[0];
  }

  _inst = P3D_create_instance
    (NULL, NULL, window_type,
     window->x, window->y, window->width, window->height,
     parent_window, tokens, _tokens.size());

  return (_inst != NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::shut
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PPInstance::
shut() {
  assert(_inst != NULL);
  log << "shut\n";
  P3D_instance_finish(_inst);
  _inst = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::isInitialized
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NPBool PPInstance::
isInitialized() {
  return _inst != NULL;
}

/*
  virtual NPError SetWindow(NPWindow *pNPWindow);
  virtual NPError NewStream(NPMIMEType type, NPStream *stream, 
                            NPBool seekable, uint16 *stype);
  virtual NPError DestroyStream(NPStream *stream, NPError reason);
  virtual void    StreamAsFile(NPStream *stream, const char *fname);
  virtual int32   WriteReady(NPStream *stream);
  virtual int32   Write(NPStream *stream, int32 offset, 
                        int32 len, void *buffer);
  virtual void    Print(NPPrint *printInfo);
  virtual uint16  HandleEvent(void *event);
  virtual void    URLNotify(const char *url, NPReason reason, 
                            void *notifyData);
  virtual NPError GetValue(NPPVariable variable, void *value);
  virtual NPError SetValue(NPNVariable variable, void *value);
*/
