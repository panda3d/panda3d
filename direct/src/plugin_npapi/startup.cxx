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
#include "ppBrowserObject.h"
#include "wstring_encode.h"
#include "find_root_dir.h"

#ifdef _WIN32
#include <shlobj.h>
#include <malloc.h>
#endif

static ofstream logfile;
ostream *nout_stream = &logfile;

string global_root_dir;
bool has_plugin_thread_async_call;

NPNetscapeFuncs *browser;

// These function prototypes changed slightly (but insignificantly)
// between releases of NPAPI.  To avoid compilation errors, we use our
// own name, and cast it to the correct type.
static int32_t
NPP_WriteReady_x(NPP instance, NPStream *stream);
static int32_t
NPP_Write_x(NPP instance, NPStream *stream, int32_t offset, 
            int32_t len, void *buffer);

// open_logfile() also assigns global_root_dir.
static bool logfile_is_open = false;
static void
open_logfile() {
  if (!logfile_is_open) {
    global_root_dir = find_root_dir();

    // Note that this logfile name may not be specified at runtime.  It
    // must be compiled in if it is specified at all.

    string log_directory;
  // Allow the developer to compile in the log directory.
#ifdef P3D_PLUGIN_LOG_DIRECTORY
    if (log_directory.empty()) {
      log_directory = P3D_PLUGIN_LOG_DIRECTORY;
    }
#endif
    
    // Failing that, we write logfiles to Panda3D/log.
    if (log_directory.empty()) {
      log_directory = global_root_dir + "/log";
    }
    mkdir_complete(log_directory, cerr);

    // Ensure that the log directory ends with a slash.
    if (!log_directory.empty() && log_directory[log_directory.size() - 1] != '/') {
#ifdef _WIN32
      if (log_directory[log_directory.size() - 1] != '\\')
#endif
        log_directory += "/";
    }
    
    // Construct the logfile pathname.
    
    string log_basename;
#ifdef P3D_PLUGIN_LOG_BASENAME1
    if (log_basename.empty()) {
      log_basename = P3D_PLUGIN_LOG_BASENAME1;
    }
#endif
    if (log_basename.empty()) {
      log_basename = "p3dplugin";
    }

    if (!log_basename.empty()) {
      string log_pathname = log_directory;
      log_pathname += log_basename;
      log_pathname += ".log";

      logfile.close();
      logfile.clear();
#ifdef _WIN32
      wstring log_pathname_w;
      string_to_wstring(log_pathname_w, log_pathname);
      logfile.open(log_pathname_w.c_str(), ios::out | ios::trunc);
#else
      logfile.open(log_pathname.c_str(), ios::out | ios::trunc);
#endif  // _WIN32
      logfile.setf(ios::unitbuf);
    }

    // If we didn't have a logfile name compiled in, we throw away log
    // output by the simple expedient of never actually opening the
    // ofstream.
    logfile_is_open = true;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: NP_GetMIMEDescription
//  Description: On Unix, this function is called by the browser to
//               get the mimetypes and extensions this plugin is
//               supposed to handle.
////////////////////////////////////////////////////////////////////
#if NP_VERSION_MAJOR == 0 && NP_VERSION_MINOR <= 22
char *
#else
const char *
#endif
NP_GetMIMEDescription(void) {
  return "application/x-panda3d:p3d:Panda3D applet;";
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
      *(const char **)value = "Panda3D Game Engine Plug-In";
      break;
    case NPPVpluginDescriptionString:
      *(const char **)value = "Runs 3-D games and interactive applets";
      break;
    default:
      nout << "Ignoring GetValue request " << variable << "\n";
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

  // open_logfile() also assigns global_root_dir.
  open_logfile();
  nout << "Initializing Panda3D plugin version " << P3D_PLUGIN_VERSION_STR << "\n";

  nout << "browserFuncs = " << browserFuncs << "\n";

  // On Unix, we have to use the pluginFuncs argument
  // to pass our entry points.
#if !defined(_WIN32) && !defined(__APPLE__)
  if (pluginFuncs != NULL) {
    NP_GetEntryPoints(pluginFuncs);
  }
#endif

  int browser_major = (browser->version >> 8) & 0xff;
  int browser_minor = browser->version & 0xff;
  nout << "Browser NPAPI version " << browser_major << "." << browser_minor << "\n";

  int expected_major = NP_VERSION_MAJOR;
  int expected_minor = NP_VERSION_MINOR;

  nout << "Plugin compiled with NPAPI version "
       << expected_major << "." << expected_minor << "\n";

  has_plugin_thread_async_call = false;
#ifdef HAS_PLUGIN_THREAD_ASYNC_CALL
  // Check if the browser offers this very useful call.
  if (browser_major > 0 || browser_minor >= NPVERS_HAS_PLUGIN_THREAD_ASYNC_CALL) {
    if ((void *)browser->pluginthreadasynccall == (void *)NULL) {
      nout << "Browser should have PLUGIN_THREAD_ASYNC_CALL, but the pointer is NULL.\n";
      has_plugin_thread_async_call = false;
    } else {
      has_plugin_thread_async_call = true;
    }
  }
#endif

  // Seed the lame random number generator in rand(); we use it to
  // select a mirror for downloading.
  srand((unsigned int)time(NULL));

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
  // open_logfile() also assigns global_root_dir.
  open_logfile();
  nout << "NP_GetEntryPoints, pluginFuncs = " << pluginFuncs << "\n";
  if (pluginFuncs->size == 0) {
    pluginFuncs->size = sizeof(*pluginFuncs);
  }
  if (pluginFuncs->size < offsetof(NPPluginFuncs, gotfocus)) {
    nout << "Invalid NPPPluginFuncs size\n";
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }

  // Not entirely sure what version number we should send back here.
  // Sending the verion number of the NPAPI library we're compiled
  // against doesn't seem 100% right, because there's no reason to
  // think that *this* code knows about all of the functions provided
  // by the particular library version it's compiled against.
  pluginFuncs->version = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;

  pluginFuncs->newp = NPP_New;
  pluginFuncs->destroy = NPP_Destroy;
  pluginFuncs->setwindow = NPP_SetWindow;
  pluginFuncs->newstream = NPP_NewStream;
  pluginFuncs->destroystream = NPP_DestroyStream;
  pluginFuncs->asfile = NPP_StreamAsFile;

  // WebKit's NPAPI defines the wrong prototype for these functions,
  // so we have to cast them.  But the casting typename isn't
  // consistent between WebKit and Mozilla's NPAPI headers.
#ifdef NewNPP_WriteProc
  pluginFuncs->writeready = NewNPP_WriteReadyProc(NPP_WriteReady_x);
  pluginFuncs->write = NewNPP_WriteProc(NPP_Write_x);
#else
  pluginFuncs->writeready = (NPP_WriteReadyProcPtr)NPP_WriteReady_x;
  pluginFuncs->write = (NPP_WriteProcPtr)NPP_Write_x;
#endif

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
  nout << "shutdown\n";
  unload_plugin(nout);
  PPBrowserObject::clear_class_definition();

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
NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, 
        int16_t argc, char *argn[], char *argv[], NPSavedData *saved) {
  nout << "new instance " << instance << "\n";

  P3D_window_handle_type window_handle_type = P3D_WHT_none;
  P3D_event_type event_type = P3D_ET_none;

#ifdef __APPLE__
  // The default drawing model for Apple is via the deprecated
  // QuickDraw GrafPtr, and the default event model is via the
  // deprecated Carbon EventRecord.
  window_handle_type = P3D_WHT_osx_port;
  event_type = P3D_ET_osx_event_record;

  // But we have to request the CoreGraphics drawing model to be
  // compatible with Snow Leopard.
  NPBool supports_core_graphics = false;
#ifdef MACOSX_HAS_COREGRAPHICS_DRAWING_MODEL
  NPError err = browser->getvalue(instance,
                                  NPNVsupportsCoreGraphicsBool,
                                  &supports_core_graphics);
  if (err != NPERR_NO_ERROR) {
    supports_core_graphics = false;
  }

  if (supports_core_graphics) {
    // Set the drawing model
    err = browser->setvalue(instance,
                            (NPPVariable)NPNVpluginDrawingModel,
                            (void *)NPDrawingModelCoreGraphics);
    if (err == NPERR_NO_ERROR) {
      window_handle_type = P3D_WHT_osx_cgcontext;
    }
  }
#endif  // MACOSX_HAS_COREGRAPHICS_DRAWING_MODEL
  nout << "supports_core_graphics = " << (bool)supports_core_graphics
       << " window_handle_type = " << window_handle_type << "\n";

  // And Snow Leopard also wants the new Cocoa event model.
  NPBool supports_cocoa = false;
#ifdef MACOSX_HAS_EVENT_MODELS
  err = browser->getvalue(instance, NPNVsupportsCocoaBool, &supports_cocoa);
  if (err != NPERR_NO_ERROR) {
    supports_cocoa = false;
  }

  if (supports_cocoa) {
    // Set the event model
    err = browser->setvalue(instance,
                            (NPPVariable)NPPVpluginEventModel,
                            (void *)NPEventModelCocoa);
    if (err == NPERR_NO_ERROR) {
      event_type = P3D_ET_osx_cocoa;
    }
  }
#endif  // MACOSX_HAS_EVENT_MODELS
  nout << "supports_cocoa = " << (bool)supports_cocoa
       << " event_type = " << event_type << "\n";
#endif  // __APPLE__

  PPInstance *inst = new PPInstance(pluginType, instance, mode,
                                    argc, argn, argv, saved,
                                    window_handle_type, event_type);
  instance->pdata = inst;
  nout << "new instance->pdata = " << inst << "\n";

  // To experiment with a "windowless" plugin, which really means we
  // create our own window without an intervening window, try this.
  //browser->setvalue(instance, NPPVpluginWindowBool, (void *)false);

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
  nout << "destroy instance " << instance << ", "
       << (PPInstance *)instance->pdata << "\n";
  nout << "save = " << (void *)save << "\n";
  //  (*save) = NULL;
  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);
  inst->stop_outstanding_streams();

  delete inst;
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
  nout << "SetWindow " << window->x << ", " << window->y
          << ", " << window->width << ", " << window->height
          << "\n";

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
              NPBool seekable, uint16_t *stype) {
  nout << "NewStream " << type << ": " << (void *)stream
       << ", " << stream->url << ", size = " << stream->end 
       << ", notifyData = " << stream->notifyData
       << ", for " << instance 
       << ", " << (PPInstance *)(instance->pdata) << "\n";
  PPInstance::generic_browser_call();
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
  nout << "DestroyStream: " << (void *)stream << ", " << stream->url 
       << ", notifyData = " << stream->notifyData
       << ", reason = " << reason
       << ", for " << instance 
       << ", " << (PPInstance *)(instance->pdata) << "\n";

  PPInstance::generic_browser_call();
  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);

  return inst->destroy_stream(stream, reason);
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_WriteReady
//  Description: Called by the browser to ask how many bytes it can
//               deliver for a stream.
////////////////////////////////////////////////////////////////////
int32_t
NPP_WriteReady_x(NPP instance, NPStream *stream) {
  //  nout << "WriteReady " << stream->url << " for " << instance << ", " << (PPInstance *)(instance->pdata) << "\n";
  PPInstance::generic_browser_call();
  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);

  return inst->write_ready(stream);
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_Write
//  Description: Called by the browser to deliver bytes for the
//               stream; the plugin should return the number of bytes
//               consumed.
////////////////////////////////////////////////////////////////////
int32_t
NPP_Write_x(NPP instance, NPStream *stream, int32_t offset, 
            int32_t len, void *buffer) {
  //  nout << "Write " << stream->url << ", " << offset << ", " << len << " for " << instance << ", " << (PPInstance *)(instance->pdata) << "\n";
  PPInstance::generic_browser_call();
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
  nout << "StreamAsFile " << stream->url 
       << ", " << stream->end 
       << ", notifyData = " << stream->notifyData
       << "\n";

  PPInstance::generic_browser_call();
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
  nout << "Print\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NPP_HandleEvent
//  Description: Called by the browser to inform the plugin of OS
//               window events.
////////////////////////////////////////////////////////////////////
int16_t
NPP_HandleEvent(NPP instance, void *event) {
  //  nout << "HandleEvent\n";
  PPInstance::generic_browser_call();

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
  nout << "URLNotify: " << url 
       << ", notifyData = " << notifyData
       << ", reason = " << reason
       << "\n";

  PPInstance::generic_browser_call();
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
  nout << "GetValue " << variable << "\n";
  PPInstance::generic_browser_call();
  PPInstance *inst = (PPInstance *)(instance->pdata);
  assert(inst != NULL);

  if (variable == NPPVpluginScriptableNPObject) {
    NPObject *obj = inst->get_panda_script_object();
    if (obj != NULL) {
      *(NPObject **)value = obj;
      return NPERR_NO_ERROR;
    }

  } else if (variable == NPPVpluginNeedsXEmbed) {
    // If we have Gtk2 available, we can use it to support the XEmbed
    // protocol, which Chromium (at least) requires.

    // In this case, we'll say we can do it, if the browser supports
    // it.  (Though probably the browser wouldn't be asking if it
    // couldn't.)

    NPBool supports_xembed = false;
    NPError err = browser->getvalue(instance, NPNVSupportsXEmbedBool, &supports_xembed);
    if (err != NPERR_NO_ERROR) {
      supports_xembed = false;
    }
    nout << "browser supports_xembed: " << (supports_xembed != 0) << "\n";
#ifdef HAVE_GTK
    bool plugin_supports = true;
#else
    bool plugin_supports = false;
    supports_xembed = false;
#endif  // HAVE_GTK
    nout << "plugin supports_xembed: " << plugin_supports << "\n";

    inst->set_xembed(supports_xembed != 0);
    *(NPBool *)value = supports_xembed;

    return NPERR_NO_ERROR;

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
  nout << "SetValue " << variable << "\n";
  PPInstance::generic_browser_call();
  return NPERR_GENERIC_ERROR;
}

