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

#include "../plugin/load_plugin_src.cxx"

ofstream logfile;
bool logfile_is_open = false;
static void
open_logfile() {
  if (!logfile_is_open) {
#ifdef _WIN32
    logfile.open("c:/cygwin/home/drose/t.log");
#else
    logfile.open("/Users/drose/t.log");
#endif
    logfile_is_open = true;
  }
}
  

// structure containing pointers to functions implemented by the browser
static NPNetscapeFuncs *browser;

// Symbol called once by the browser to initialize the plugin
NPError
NP_Initialize(NPNetscapeFuncs *browserFuncs) {
  // save away browser functions
  browser = browserFuncs;

  open_logfile();
  logfile << "initializing\n" << flush;

  logfile << "browserFuncs = " << browserFuncs << "\n" << flush;

  string plugin_location = "/Users/drose/player/direct/built/lib/libp3d_plugin.dylib";

  if (!load_plugin(plugin_location.c_str())) {
    logfile << "couldn't load plugin\n" << flush;
    return NPERR_INVALID_PLUGIN_ERROR;
  }

  return NPERR_NO_ERROR;
}

// Symbol called by the browser to get the plugin's function list
NPError
NP_GetEntryPoints(NPPluginFuncs *pluginFuncs) {
  open_logfile();
  logfile << "NP_GetEntryPoints, pluginFuncs = " << pluginFuncs << "\n"
          << flush;
  pluginFuncs->version = 11;
  pluginFuncs->size = sizeof(pluginFuncs);
  pluginFuncs->newp = NPP_New;
  pluginFuncs->destroy = NPP_Destroy;
  pluginFuncs->setwindow = NPP_SetWindow;
  pluginFuncs->newstream = NPP_NewStream;
  pluginFuncs->destroystream = NPP_DestroyStream;
  pluginFuncs->asfile = NPP_StreamAsFile;
  pluginFuncs->writeready = NPP_WriteReady;
  pluginFuncs->write = NPP_Write;
  pluginFuncs->print = NPP_Print;
  pluginFuncs->event = NPP_HandleEvent;
  pluginFuncs->urlnotify = NPP_URLNotify;
  pluginFuncs->getvalue = NPP_GetValue;
  pluginFuncs->setvalue = NPP_SetValue;

  return NPERR_NO_ERROR;
}

// Symbol called once by the browser to shut down the plugin
NPError 
NP_Shutdown(void) {
  logfile << "shutdown\n" << flush;
  unload_plugin();

  return NPERR_NO_ERROR;
}

// Called to create a new instance of the plugin
NPError 
NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, 
        int16_t argc, char *argn[], char *argv[], NPSavedData *saved) {
  logfile << "new instance\n" << flush;

  // Copy the tokens into a temporary array of P3D_token objects.
  P3D_token *tokens = (P3D_token *)alloca(sizeof(P3D_token) * argc);
  for (int i = 0; i < argc; ++i) {
    P3D_token &token = tokens[i];
    token._keyword = argn[i];
    token._value = argv[i];
    logfile << " " << i << ": " << token._keyword << " = " << token._value << "\n";
  }

  instance->pdata = P3D_create_instance(NULL, NULL, tokens, argc);

  return NPERR_NO_ERROR;
}

// Called to destroy an instance of the plugin
NPError
NPP_Destroy(NPP instance, NPSavedData **save) {
  logfile << "destroy instance\n" << flush;
  (*save) = NULL;
  P3D_instance_finish((P3D_instance *)(instance->pdata));
  instance->pdata = NULL;

  return NPERR_NO_ERROR;
}

// Called to update a plugin instances's NPWindow
NPError
NPP_SetWindow(NPP instance, NPWindow *window) {
  logfile << "SetWindow " << window->x << ", " << window->y
          << ", " << window->width << ", " << window->height
          << "\n" << flush;

  P3D_instance *inst = (P3D_instance *)(instance->pdata);
  assert(inst != NULL);
  
  P3D_window_handle parent_window;
#ifdef _WIN32
  parent_window._hwnd = (HWND)(window->window);
#endif

  P3D_window_type window_type = P3D_WT_embedded;

  P3D_instance_setup_window
    (inst, window_type,
     window->x, window->y, window->width, window->height,
     parent_window);

  return NPERR_NO_ERROR;
}


NPError
NPP_NewStream(NPP instance, NPMIMEType type, NPStream *stream, 
              NPBool seekable, uint16_t *stype) {
  *stype = NP_ASFILEONLY;
  return NPERR_NO_ERROR;
}

NPError 
NPP_DestroyStream(NPP instance, NPStream *stream, NPReason reason) {
  return NPERR_NO_ERROR;
}

int32_t
NPP_WriteReady(NPP instance, NPStream *stream) {
  return 0;
}

int32_t
NPP_Write(NPP instance, NPStream *stream, int32_t offset, 
          int32_t len, void *buffer) {
  return 0;
}

void
NPP_StreamAsFile(NPP instance, NPStream *stream, const char *fname) {
}

void 
NPP_Print(NPP instance, NPPrint *platformPrint) {
}

int16_t
NPP_HandleEvent(NPP instance, void *event) {
  return 0;
}

void
NPP_URLNotify(NPP instance, const char *url,
              NPReason reason, void *notifyData) {
}

NPError
NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
  return NPERR_GENERIC_ERROR;
}

NPError 
NPP_SetValue(NPP instance, NPNVariable variable, void *value) {
  return NPERR_GENERIC_ERROR;
}
