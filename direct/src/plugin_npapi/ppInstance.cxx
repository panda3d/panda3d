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
#include "startup.h"
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
  _p3d_inst = NULL;

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

  if (!is_plugin_loaded()) {
    // Start the plugin DLL downloading.
    string url = P3D_PLUGIN_DOWNLOAD;
    url += P3D_PLUGIN_PLATFORM;
    url += "/";
    url += get_plugin_basename();
    
    PPDownloadRequest *req = new PPDownloadRequest(PPDownloadRequest::RT_core_dll);
    browser->geturlnotify(_npp_instance, url.c_str(), NULL, req);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPInstance::
~PPInstance() {
  logfile
    << "destructing " << this << ", " << _p3d_inst << "\n" << flush;

  if (_p3d_inst != NULL) {
    P3D_instance_finish(_p3d_inst);
    _p3d_inst = NULL;
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
  
  if (_p3d_inst == NULL) {
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
      _started_instance_data = true;
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

  case PPDownloadRequest::RT_user:
    // This is a request from the plugin.  We'll receive this as a
    // stream.
    *stype = NP_NORMAL;
    return NPERR_NO_ERROR;

  default:
    // Don't know what this is.
    logfile << "Unexpected request " << (int)req->_rtype << "\n";
  }

  return NPERR_GENERIC_ERROR;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::write_stream
//       Access: Public
//  Description: Called by the browser to feed data read from a URL or
//               whatever.
////////////////////////////////////////////////////////////////////
int PPInstance::
write_stream(NPStream *stream, int offset, int len, void *buffer) {
  if (stream->notifyData == NULL) {
    logfile << "Unexpected write_stream on " << stream->url << "\n";
    return 0;
  }

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);
  switch (req->_rtype) {
  case PPDownloadRequest::RT_user:
    P3D_instance_feed_url_stream(_p3d_inst, req->_user_id,
                                 P3D_RC_in_progress, 0,
                                 stream->end, buffer, len);
    return len;
    
  default:
    logfile << "Unexpected write_stream on " << stream->url << "\n";
    break;
  }

  return 0;
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
  if (stream->notifyData == NULL) {
    logfile << "Unexpected destroy_stream on " << stream->url << "\n";
    return NPERR_GENERIC_ERROR;
  }

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);
  switch (req->_rtype) {
  case PPDownloadRequest::RT_user:
    {
      P3D_result_code result_code = P3D_RC_done;
      if (reason != NPRES_DONE) {
        result_code = P3D_RC_generic_error;
      }
      assert(!req->_notified_done);
      P3D_instance_feed_url_stream(_p3d_inst, req->_user_id,
                                   result_code, 0, stream->end, NULL, 0);
      req->_notified_done = true;
    }
    break;

  default:
    break;
  }
  
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
  if (notifyData == NULL) {
    return;
  }
  
  PPDownloadRequest *req = (PPDownloadRequest *)notifyData;
  switch (req->_rtype) {
  case PPDownloadRequest::RT_user:
    if (!req->_notified_done) {
      // We shouldn't have gotten here without notifying the stream
      // unless the stream never got started (and hence we never
      // called destroy_stream().
      logfile << "Failure starting stream\n" << flush;
      assert(reason != NPRES_DONE);

      P3D_instance_feed_url_stream(_p3d_inst, req->_user_id,
                                   P3D_RC_generic_error, 0, 0, NULL, 0);
      req->_notified_done = true;
    }
    break;
    
  default:
    break;
  }
  
  delete req;
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

  // TODO: Is "Macintosh HD:" the only possible prefix?
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
    {
      // This is the core API DLL (or dylib or whatever).  Now that
      // we've downloaded it, we can load it.
      string override_filename = P3D_PLUGIN_P3D_PLUGIN;
      if (!override_filename.empty()) {
        filename = override_filename;
      }
      logfile << "got plugin " << filename << "\n" << flush;
      if (!load_plugin(filename)) {
        logfile << "Unable to launch core API.\n";
        break;
      }
      logfile << "loaded core API\n";
      create_instance();
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
//     Function: PPInstance::handle_request
//       Access: Public
//  Description: Handles a request from the plugin, forwarding
//               it to the browser as appropriate.
////////////////////////////////////////////////////////////////////
void PPInstance::
handle_request(P3D_request *request) {
  logfile << "handle_request: " << request << ", " << request->_instance
          << " within " << this << ", " << _p3d_inst << "\n" << flush;
  assert(request->_instance == _p3d_inst);

  bool handled = false;

  switch (request->_request_type) {
  case P3D_RT_stop:
    logfile << "Got P3D_RT_stop\n";
    if (_p3d_inst != NULL) {
      P3D_instance_finish(_p3d_inst);
      _p3d_inst = NULL;
    }
    // Guess the browser doesn't really care.
    handled = true;
    break;

  case P3D_RT_get_url:
    {
      logfile << "Got P3D_RT_get_url: " << request->_request._get_url._url
              << "\n";
      
      PPDownloadRequest *req = 
        new PPDownloadRequest(PPDownloadRequest::RT_user, 
                              request->_request._get_url._unique_id);
      browser->geturlnotify(_npp_instance, request->_request._get_url._url,
                            NULL, req);
    }
    
    break;

  default:
    // Some request types are not handled.
    logfile << "Unhandled request: " << request->_request_type << "\n";
    break;
  };

  P3D_request_finish(request, handled);
}


////////////////////////////////////////////////////////////////////
//     Function: PPInstance::create_instance
//       Access: Private
//  Description: Actually creates the internal P3D_instance object, if
//               possible and needed.
////////////////////////////////////////////////////////////////////
void PPInstance::
create_instance() {
  if (_p3d_inst != NULL) {
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

  logfile << "within " << this << ", creating new instance\n" << flush;
  _p3d_inst = P3D_new_instance(request_ready, this);
  logfile << "within " << this << ", created new instance " << _p3d_inst 
          << "\n" << flush;

  if (_p3d_inst != NULL) {
    const P3D_token *tokens = NULL;
    if (!_tokens.empty()) {
      tokens = &_tokens[0];
    }
    P3D_instance_start(_p3d_inst, _p3d_filename.c_str(), tokens, _tokens.size());
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
  assert(_p3d_inst != NULL);

  P3D_window_handle parent_window;
#ifdef _WIN32
  parent_window._hwnd = (HWND)(_window.window);
#endif

  // Actually, we set up the window starting at (0, 0), instead of
  // whatever Mozilla tells us, because the window handle we get is a
  // specially created window that is already aligned to where we want
  // our window to be.
  P3D_instance_setup_window
    (_p3d_inst, P3D_WT_embedded,
     0, 0, _window.width, _window.height,
     parent_window);
}
