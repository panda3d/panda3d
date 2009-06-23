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

#ifdef _WIN32
#include <malloc.h>
#endif

ofstream logfile;

NPNetscapeFuncs *browser;

static bool logfile_is_open = false;
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
  
////////////////////////////////////////////////////////////////////
//     Function: NP_Initialize
//  Description: This function is called (almost) before any other
//               function, to ask the plugin to initialize itself and
//               to send the pointers to the browser control
//               functions.  Also see NP_GetEntryPoints.
////////////////////////////////////////////////////////////////////
#ifdef _WIN32
NPError OSCALL 
NP_Initialize(NPNetscapeFuncs *browserFuncs)
#else
// On Mac, the API specifies this second parameter is included, but it
// lies.  We actually don't get a second parameter on Mac, but we have
// to put it here to make the compiler happy.
NPError OSCALL
NP_Initialize(NPNetscapeFuncs *browserFuncs,
              NPPluginFuncs *pluginFuncs)
#endif
{
  // save away browser functions
  browser = browserFuncs;

  open_logfile();
  logfile << "initializing\n" << flush;

  logfile << "browserFuncs = " << browserFuncs << "\n" << flush;

#ifdef _WIN32
  string plugin_location = "c:/cygwin/home/drose/player/direct/built/lib/p3d_plugin.dll";
#else
  string plugin_location = "/Users/drose/player/direct/built/lib/p3d_plugin.dylib";
#endif

  if (!load_plugin(plugin_location.c_str())) {
    logfile << "couldn't load plugin\n" << flush;
    return NPERR_INVALID_PLUGIN_ERROR;
  }

  return NPERR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: NP_GetEntryPoints
//  Description: This method is extracted directly from the DLL and
//               called at initialization time by the browser, either
//               before or after NP_Initialize, to retrieve the
//               pointers to the rest of the plugin functions that are
//               not exported from the DLL.
////////////////////////////////////////////////////////////////////
NPError OSCALL
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

////////////////////////////////////////////////////////////////////
//     Function: NP_Shutdown
//  Description: This function is called when the browser is done with
//               the plugin; it asks the plugin to unload itself and
//               free all used resources.
////////////////////////////////////////////////////////////////////
NPError OSCALL
NP_Shutdown(void) {
  logfile << "shutdown\n" << flush;
  unload_plugin();

  // Not clear whether there's a return value or not.  Some versions
  // of the API have different opinions on this.
  return NPERR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_New
//  Description: Called by the browser to create a new instance of the
//               plugin.
////////////////////////////////////////////////////////////////////
NPError 
NPP_New(NPMIMEType pluginType, NPP instance, uint16 mode, 
        int16 argc, char *argn[], char *argv[], NPSavedData *saved) {
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

////////////////////////////////////////////////////////////////////
//     Function: NPP_Destroy
//  Description: Called by the browser to destroy an instance of the
//               plugin previously created with NPP_New.
////////////////////////////////////////////////////////////////////
NPError
NPP_Destroy(NPP instance, NPSavedData **save) {
  logfile << "destroy instance\n" << flush;
  (*save) = NULL;
  P3D_instance_finish((P3D_instance *)(instance->pdata));
  instance->pdata = NULL;

  return NPERR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_SetWindow
//  Description: Called by the browser to inform the instance of its
//               window size and placement.  This is called initially
//               to create the window, and may be called subsequently
//               when the window needs to be moved.  It may be called
//               redundantly.
////////////////////////////////////////////////////////////////////
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

  P3D_instance_setup_window
    (inst, P3D_WT_embedded,
     window->x, window->y, window->width, window->height,
     parent_window);

  return NPERR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_NewStream
//  Description: Called by the browser when a new data stream is
//               created, usually in response to a geturl request; but
//               it is also called initially to supply the data in the
//               data or src element.  The plugin must specify how it
//               can receive the stream.
////////////////////////////////////////////////////////////////////
NPError
NPP_NewStream(NPP instance, NPMIMEType type, NPStream *stream, 
              NPBool seekable, uint16 *stype) {
  logfile << "NewStream " << type << ", " << stream->url 
          << ", " << stream->end << "\n" << flush;
  *stype = NP_ASFILEONLY;
  return NPERR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_DestroyStream
//  Description: Called by the browser to mark the end of a stream
//               created with NewStream.
////////////////////////////////////////////////////////////////////
NPError 
NPP_DestroyStream(NPP instance, NPStream *stream, NPReason reason) {
  logfile << "DestroyStream\n" << flush;
  return NPERR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_WriteReady
//  Description: Called by the browser to ask how many bytes it can
//               deliver for a stream.
////////////////////////////////////////////////////////////////////
int32
NPP_WriteReady(NPP instance, NPStream *stream) {
  logfile << "WriteReady\n";
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_Write
//  Description: Called by the browser to deliver bytes for the
//               stream; the plugin should return the number of bytes
//               consumed.
////////////////////////////////////////////////////////////////////
int32
NPP_Write(NPP instance, NPStream *stream, int32 offset, 
          int32 len, void *buffer) {
  logfile << "Write\n";
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_StreamAsFile
//  Description: Called by the browser to report the filename that
//               contains the fully-downloaded stream, if
//               NP_ASFILEONLY was specified by the plugin in
//               NPP_NewStream.
////////////////////////////////////////////////////////////////////
void
NPP_StreamAsFile(NPP instance, NPStream *stream, const char *fname) {
  logfile << "StreamAsFile: " << fname << "\n" << flush;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_Print
//  Description: Called by the browser when the user attempts to print
//               the page containing the plugin instance.
////////////////////////////////////////////////////////////////////
void 
NPP_Print(NPP instance, NPPrint *platformPrint) {
  logfile << "Print\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_HandleEvent
//  Description: Called by the browser to inform the plugin of OS
//               window events.
////////////////////////////////////////////////////////////////////
int16
NPP_HandleEvent(NPP instance, void *event) {
  logfile << "HandleEvent\n";
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_URLNotify
//  Description: Called by the browser to inform the plugin of a
//               completed URL request.
////////////////////////////////////////////////////////////////////
void
NPP_URLNotify(NPP instance, const char *url,
              NPReason reason, void *notifyData) {
  logfile << "URLNotify: " << url << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_GetValue
//  Description: Called by the browser to query specific information
//               from the plugin.
////////////////////////////////////////////////////////////////////
NPError
NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
  logfile << "GetValue " << variable << "\n";
  return NPERR_GENERIC_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_URLNotify
//  Description: Called by the browser to update a scriptable value.
////////////////////////////////////////////////////////////////////
NPError 
NPP_SetValue(NPP instance, NPNVariable variable, void *value) {
  logfile << "SetValue " << variable << "\n";
  return NPERR_GENERIC_ERROR;
}
