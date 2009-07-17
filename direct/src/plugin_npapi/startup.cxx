// Filename: startup.cxx
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

#include "startup.h"
#include "p3d_plugin_config.h"
#include "p3d_lock.h"

#ifdef _WIN32
#include <malloc.h>
#endif

ofstream logfile;

NPNetscapeFuncs *browser;

static bool logfile_is_open = false;
static void
open_logfile() {
  if (!logfile_is_open) {
    logfile.open(P3D_PLUGIN_LOGFILE1);
    logfile_is_open = true;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: NP_GetMIMEDescription
//  Description: On Unix, this function is called by the browser to
//               get the mimetypes and extensions this plugin is
//               supposed to handle.
////////////////////////////////////////////////////////////////////
char*
NP_GetMIMEDescription(void) {
  return (char*) "application/x-panda3d:p3d:Panda3D applet;";
}

////////////////////////////////////////////////////////////////////
//     Function: NP_GetValue
//  Description: On Unix, this function is called by the browser to
//               get some information like the name and description.
////////////////////////////////////////////////////////////////////
NPError
NP_GetValue(void*, NPPVariable variable, void* value) {
  if (value == NULL) {
    return NPERR_INVALID_PARAM;
  }
  
  switch (variable) {
    case NPPVpluginNameString:
      *(const char **)value = "Panda3D Game Engine Plug-in";
      break;
    case NPPVpluginDescriptionString:
      *(const char **)value = "Runs 3-D games and interactive applets";
      break;
    case NPPVpluginNeedsXEmbed:
      *((PRBool *)value) = PR_FALSE;
      break;
    default:
      logfile << "Ignoring GetValue request " << variable << "\n" << flush;
      return NPERR_INVALID_PARAM;
  }
  
  return NPERR_NO_ERROR;
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
// On Mac, the API specifies this second parameter is included,
// but it lies.  We actually don't get a second parameter there,
// but we have to put it here to make the compiler happy.
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

  // On Unix, we have to use the pluginFuncs argument
  // to pass our entry points.
#if !defined(_WIN32) && !defined(__APPLE__)
  if (pluginFuncs != NULL) {
    NP_GetEntryPoints(pluginFuncs);
  }
#endif

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

  PPInstance *inst = new PPInstance(pluginType, instance, mode,
                                    argc, argn, argv, saved);
  instance->pdata = inst;

  // To experiment with a "windowless" plugin, which really means we
  // create our own window without an intervening window, try this.
  // At the moment, there's a deadlock condition when we try it, but
  // surely it can be solved.
  //  browser->setvalue(instance, NPPVpluginWindowBool, (void *)false);

  // Now that we have stored the pointer, we can call begin(), which
  // starts to initiate downloads.
  inst->begin();

  return NPERR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_Destroy
//  Description: Called by the browser to destroy an instance of the
//               plugin previously created with NPP_New.
////////////////////////////////////////////////////////////////////
NPError
NPP_Destroy(NPP instance, NPSavedData **save) {
  logfile << "destroy instance " << instance << "\n";
  logfile << "save = " << (void *)save << "\n" << flush;
  //  (*save) = NULL;
  delete (PPInstance *)(instance->pdata);
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

  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);
  inst->set_window(window);

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
          << ", " << stream->end 
          << ", notifyData = " << stream->notifyData
          << "\n" << flush;
  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);

  return inst->new_stream(type, stream, seekable != 0, stype);
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_DestroyStream
//  Description: Called by the browser to mark the end of a stream
//               created with NewStream.
////////////////////////////////////////////////////////////////////
NPError 
NPP_DestroyStream(NPP instance, NPStream *stream, NPReason reason) {
  logfile << "DestroyStream " << stream->url 
          << ", " << stream->end 
          << ", notifyData = " << stream->notifyData
          << ", reason = " << reason
          << "\n" << flush;

  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);

  return inst->destroy_stream(stream, reason);
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_WriteReady
//  Description: Called by the browser to ask how many bytes it can
//               deliver for a stream.
////////////////////////////////////////////////////////////////////
int32
NPP_WriteReady(NPP instance, NPStream *stream) {
  // We're supposed to return the maximum amount of data the plugin is
  // prepared to handle.  Gee, I don't know.  As much as you can give
  // me, I guess.
  return 0x7fffffff;
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
  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);

  return inst->write_stream(stream, offset, len, buffer);
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
  logfile << "StreamAsFile " << stream->url 
          << ", " << stream->end 
          << ", notifyData = " << stream->notifyData
          << "\n" << flush;

  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);

  inst->stream_as_file(stream, fname);
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
  //  logfile << "HandleEvent\n" << flush;

  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);

  return inst->handle_event(event);
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_URLNotify
//  Description: Called by the browser to inform the plugin of a
//               completed URL request.
////////////////////////////////////////////////////////////////////
void
NPP_URLNotify(NPP instance, const char *url,
              NPReason reason, void *notifyData) {
  logfile << "URLNotify: " << url 
          << ", notifyData = " << notifyData
          << ", reason = " << reason
          << "\n" << flush;

  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);

  inst->url_notify(url, reason, notifyData);
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_GetValue
//  Description: Called by the browser to query specific information
//               from the plugin.
////////////////////////////////////////////////////////////////////
NPError
NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
  logfile << "GetValue " << variable << "\n";
  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);

  if (variable == NPPVpluginScriptableNPObject) {
    NPObject *obj = inst->get_panda_script_object();
    if (obj != NULL) {
      *(NPObject **)value = obj;
      return NPERR_NO_ERROR;
    }
  } else {
    return NP_GetValue(NULL, variable, value);
  }

  return NPERR_GENERIC_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_SetValue
//  Description: Called by the browser to update a scriptable value.
////////////////////////////////////////////////////////////////////
NPError 
NPP_SetValue(NPP instance, NPNVariable variable, void *value) {
  logfile << "SetValue " << variable << "\n";
  return NPERR_GENERIC_ERROR;
}

