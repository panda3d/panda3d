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
#include "p3d_plugin_config.h"

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

  _npp_instance = instance;
  _npp_mode = mode;

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

  _started_instance_data = false;
  _got_instance_data = false;
  _got_window = false;

  // Start the plugin DLL downloading.
  string url = P3D_PLUGIN_DOWNLOAD;
  url += P3D_PLUGIN_PLATFORM;
  url += "/";
  url += get_plugin_basename();

  PPDownloadRequest *req = new PPDownloadRequest(PPDownloadRequest::RT_core_dll);
  browser->geturlnotify(_npp_instance, url.c_str(), NULL, req);
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
  
  if (_inst == NULL) {
    create_instance();
  } else {
    send_window();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::new_stream
//       Access: Public
//  Description: Receives notification of a new stream object, e.g. a
//               url request.
////////////////////////////////////////////////////////////////////
NPError PPInstance::
new_stream(NPMIMEType type, NPStream *stream, bool seekable, uint16 *stype) {
  if (stream->notifyData == NULL) {
    // This is an unsolicited stream.  Assume the first unsolicited
    // stream we receive is the instance data; any other unsolicited
    // stream is an error.
    if (!_started_instance_data) {
      stream->notifyData = new PPDownloadRequest(PPDownloadRequest::RT_instance_data);
      *stype = NP_ASFILEONLY;
      return NPERR_NO_ERROR;
    }

    // This is an unexpected unsolicited stream.  (Firefox seems to
    // give us the instance data twice for some reason.)
    return NPERR_GENERIC_ERROR;
  }

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);
  switch (req->_rtype) {
  case PPDownloadRequest::RT_core_dll:
    // This is the core API DLL (or dylib or whatever).  We want to
    // download this to file so we can run it directly.
    *stype = NP_ASFILEONLY;
    return NPERR_NO_ERROR;

  default:
    // Don't know what this is.
    logfile << "Unexpected request " << (int)req->_rtype << "\n";
  }

  return NPERR_GENERIC_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::destroy_stream
//       Access: Public
//  Description: Called by the browser to mark the end of a stream;
//               the file has either been successfully downloaded or
//               failed.
////////////////////////////////////////////////////////////////////
NPError PPInstance::
destroy_stream(NPStream *stream, NPReason reason) {
  return NPERR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::url_notify
//       Access: Public
//  Description: Called by the browser to announce the end of a
//               stream.  This normally follows destroy_stream(),
//               unless the stream was never created in the first
//               place.
////////////////////////////////////////////////////////////////////
void PPInstance::
url_notify(const char *url, NPReason reason, void *notifyData) {
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::stream_as_file
//       Access: Public
//  Description: Called by the browser to report the filename that
//               contains the fully-downloaded stream contents.
////////////////////////////////////////////////////////////////////
void PPInstance::
stream_as_file(NPStream *stream, const char *fname) {
  if (stream->notifyData == NULL) {
    logfile << "Unexpected stream_as_file on " << stream->url << "\n";
    return;
  }

  string filename = fname;
#ifdef __APPLE__
  // Safari seems to want to report the filename in the old-style form
  // "Macintosh HD:blah:blah:blah" instead of the new-style form
  // "/blah/blah/blah".  How annoying.
  if (filename.substr(0, 13) == "Macintosh HD:") {
    string fname2;
    for (size_t p = 12; p < filename.size(); ++p) {
      if (filename[p] == ':') {
        fname2 += '/';
      } else {
        fname2 += filename[p];
      }
    }
    filename = fname2;
    logfile << "converted filename to " << filename << "\n";
  }

#endif  // __APPLE__

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);
  switch (req->_rtype) {
  case PPDownloadRequest::RT_core_dll:
    // This is the core API DLL (or dylib or whatever).  Now that
    // we've downloaded it, we can load it.
    logfile << "got plugin\n";
    if (!load_plugin(filename)) {
      logfile << "Unable to launch core API.\n";
    }
    break;

  case PPDownloadRequest::RT_instance_data:
    // This is the instance data, e.g. the p3d filename.  Now we can
    // launch the instance.
    _got_instance_data = true;
    _p3d_filename = filename;
    create_instance();
    break;

  default:
    // Don't know what this is.
    logfile << "Unexpected stream_as_file, type " << (int)req->_rtype << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::create_instance
//       Access: Private
//  Description: Actually creates the internal P3D_instance object, if
//               possible and needed.
////////////////////////////////////////////////////////////////////
void PPInstance::
create_instance() {
  if (_inst != NULL) {
    // Already created.
    return;
  }

  if (!is_plugin_loaded()) {
    // Plugin is not loaded yet.
    return;
  }

  if (!_got_instance_data) {
    // No instance data yet.
    return;
  }

  if (!_got_window) {
    // No window yet.
    return;
  }

  const P3D_token *tokens = NULL;
  if (!_tokens.empty()) {
    tokens = &_tokens[0];
  }

  _inst = P3D_create_instance
    (NULL, _p3d_filename.c_str(), tokens, _tokens.size());

  if (_inst != NULL) {
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
