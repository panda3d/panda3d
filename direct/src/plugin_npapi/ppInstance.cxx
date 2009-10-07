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
#include "ppPandaObject.h"
#include "ppToplevelObject.h"
#include "ppBrowserObject.h"
#include "startup.h"
#include "p3d_plugin_config.h"
#include "find_root_dir.h"
#include "mkdir_complete.h"

// We can include this header file to get the DTOOL_PLATFORM
// definition, even though we don't link with dtool.
#include "dtool_platform.h"

#include <fstream>
#include <algorithm>
#include <string.h>  // strcmp()
#include <time.h>

#ifndef _WIN32
#include <sys/select.h>
#endif

PPInstance::FileDatas PPInstance::_file_datas;

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
  _p3d_inst = NULL;

  _npp_instance = instance;
  _npp_mode = mode;
  _script_object = NULL;

  // Copy the tokens and save them within this object.
  _tokens.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    P3D_token token;
    token._keyword = strdup(argn[i]);
    token._value = strdup(argv[i]);
    _tokens.push_back(token);
  }

  _root_dir = global_root_dir;

  _got_instance_url = false;
  _got_window = false;
  _python_window_open = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPInstance::
~PPInstance() {
  cleanup_window();

  if (_p3d_inst != NULL) {
    P3D_instance_finish(_p3d_inst);
    _p3d_inst = NULL;
  }

  if (_script_object != NULL) {
    browser->releaseobject(_script_object);
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
//     Function: PPInstance::begin
//       Access: Public
//  Description: Begins the initial download of the core API.  This
//               should be called after constructing the PPInstance.
//               It is a separate method than the constructor, because
//               it initiates some callbacks that might rely on the
//               object having been fully constructed and its pointer
//               stored.
////////////////////////////////////////////////////////////////////
void PPInstance::
begin() {
  if (!is_plugin_loaded()) {
    // Go download the contents file, so we can download the core DLL.
    string url = PANDA_PACKAGE_HOST_URL;
    if (!url.empty() && url[url.length() - 1] != '/') {
      url += '/';
    }
    _download_url_prefix = url;
    ostringstream strm;
    strm << url << "contents.xml";

    // Append a uniquifying query string to the URL to force the
    // download to go all the way through any caches.  We use the time
    // in seconds; that's unique enough.
    strm << "?" << time(NULL);
    url = strm.str();

    PPDownloadRequest *req = new PPDownloadRequest(PPDownloadRequest::RT_contents_file);
    start_download(url, req);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::set_window
//       Access: Public
//  Description: Stores or updates the window parameters.
////////////////////////////////////////////////////////////////////
void PPInstance::
set_window(NPWindow *window) {
  if (_got_window && 
      window->x == _window.x &&
      window->y == _window.y &&
      window->width == _window.width &&
      window->height == _window.height) {
    // No changes.
    return;
  }

  if (_got_window) {
    // We don't expect the browser to change the window's parent
    // on-the-fly.
    assert(_window.window == window->window);
  }

#ifdef _WIN32
  if (!_got_window) {
    _orig_window_proc = NULL;
    if (window->type == NPWindowTypeWindow) {
      // Subclass the window to make it call our own window_proc instead
      // of whatever window_proc it has already.  This is just a dopey
      // trick to allow us to poll events in the main thread.
      HWND hwnd = (HWND)window->window;
      _orig_window_proc = SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)window_proc);

      // Also set a timer to go off every once in a while, just in
      // case something slips through.
      SetTimer(hwnd, 1, 1000, NULL);
    }
  }
#endif  // _WIN32

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

    // We don't let Mozilla finish downloading the instance data, but
    // we do extract its URL to pass to the instance.
    if (!_got_instance_url && stream->url != NULL) {
      _got_instance_url = true;
      _instance_url = stream->url;
      if (_p3d_inst != NULL) {
        P3D_instance_start(_p3d_inst, false, _instance_url.c_str());
      } 

      // We don't want the rest of this stream any more, but we can't
      // just return NPERR_GENERIC_ERROR, though--that seems to freak
      // out Firefox.
      stream->notifyData = new PPDownloadRequest(PPDownloadRequest::RT_instance_data);

      *stype = NP_NORMAL;
      return NPERR_NO_ERROR;
    }

    // Don't finish downloading the unsolicited stream.
    return NPERR_GENERIC_ERROR;
  }

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);
  switch (req->_rtype) {
  case PPDownloadRequest::RT_contents_file:
    // This is the initial contents.xml file.  We'll just download
    // this directly to a file, since it is small and this is easy.
    *stype = NP_ASFILEONLY;
    return NPERR_NO_ERROR;

  case PPDownloadRequest::RT_core_dll:
    // This is the core API DLL (or dylib or whatever).  We want to
    // download this to file for convenience.
    *stype = NP_ASFILEONLY;
    return NPERR_NO_ERROR;

  case PPDownloadRequest::RT_user:
    // This is a request from the plugin.  We'll receive this as a
    // stream.
    *stype = NP_NORMAL;
    return NPERR_NO_ERROR;

  default:
    // Don't know what this is.
    nout << "Unexpected request " << (int)req->_rtype << "\n";
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
    nout << "Unexpected write_stream on " << stream->url << "\n";
    browser->destroystream(_npp_instance, stream, NPRES_USER_BREAK);
    return 0;
  }

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);
  switch (req->_rtype) {
  case PPDownloadRequest::RT_user:
    P3D_instance_feed_url_stream(_p3d_inst, req->_user_id,
                                 P3D_RC_in_progress, 0,
                                 stream->end, buffer, len);
    return len;

  case PPDownloadRequest::RT_instance_data:
    // Here's a stream we don't really want.  But stopping it here
    // seems to freak out Safari.  (And stopping it before it starts
    // freaks out Firefox.)  Whatever.  We'll just quietly ignore the
    // data.
    return len;
    
  default:
    nout << "Unexpected write_stream on " << stream->url << "\n";
    break;
  }

  browser->destroystream(_npp_instance, stream, NPRES_USER_BREAK);
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
    nout << "Unexpected destroy_stream on " << stream->url << "\n";
    return NPERR_NO_ERROR;
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

  case PPDownloadRequest::RT_instance_data:
    break;

  default:
    nout << "Unexpected destroy_stream on " << stream->url << "\n";
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
      nout << "Failure starting stream\n";
      assert(reason != NPRES_DONE);

      P3D_instance_feed_url_stream(_p3d_inst, req->_user_id,
                                   P3D_RC_generic_error, 0, 0, NULL, 0);
      req->_notified_done = true;
    }
    break;

  case PPDownloadRequest::RT_contents_file:
    if (reason != NPRES_DONE) {
      nout << "Failure downloading " << url << "\n";

      // Couldn't download a fresh contents.xml for some reason.  If
      // there's an outstanding contents.xml file on disk, try to load
      // that one as a fallback.
      string contents_filename = _root_dir + "/contents.xml";
      if (!read_contents_file(contents_filename)) {
        nout << "Unable to read contents file " << contents_filename << "\n";
        // TODO: fail
      }
    }
    break;
    
  case PPDownloadRequest::RT_core_dll:
    if (reason != NPRES_DONE) {
      nout << "Failure downloading " << url << "\n";
      // Couldn't download from this mirror.  Try the next one.
      if (!_core_urls.empty()) {
        string url = _core_urls.back();
        _core_urls.pop_back();
        
        PPDownloadRequest *req2 = new PPDownloadRequest(PPDownloadRequest::RT_core_dll);
        start_download(url, req2);
      }
    }
    break;

  default:
    nout << "Unexpected url_notify on stream type " << req->_rtype << "\n";
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
    nout << "Unexpected stream_as_file on " << stream->url << "\n";
    return;
  }

  string filename = fname;
#ifdef __APPLE__
  // Safari seems to want to report the filename in the old-style form
  // "Macintosh HD:blah:blah:blah" instead of the new-style form
  // "/blah/blah/blah".  How annoying.

  size_t colon = filename.find(':');
  size_t slash = filename.find('/');
  if (colon < slash) {
    // This might be such a filename.

    string fname2 = "/Volumes/";
    for (size_t p = 0; p < filename.size(); ++p) {
      if (filename[p] == ':') {
        fname2 += '/';
      } else {
        fname2 += filename[p];
      }
    }

    if (access(fname2.c_str(), R_OK) == 0) {
      // Looks like we've converted it successfully.
      filename = fname2;

      // Here's another crazy hack.  In addition to the weird filename
      // format, the file that Safari tells us about appears to be a
      // temporary file that Safari's about to delete.  In order to
      // protect ourselves from this, we need to temporarily copy the
      // file somewhere else.
      char *name = tempnam(NULL, "p3d_");

      // We prefer just making a hard link; it's quick and easy.
      if (link(filename.c_str(), name) != 0) {
        // But sometimes the hard link might fail, particularly if these
        // are two different file systems.  In this case we have to open
        // the files and copy the data by hand.
        copy_file(filename, name);
      }
      
      filename = name;
      free(name);
      
      // TODO: remove this temporary file when we're done with it.
    }
  }
    
#endif  // __APPLE__

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);
  downloaded_file(req, filename);
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::handle_request
//       Access: Public
//  Description: Handles a request from the plugin, forwarding
//               it to the browser as appropriate.
////////////////////////////////////////////////////////////////////
void PPInstance::
handle_request(P3D_request *request) {
  assert(request->_instance == _p3d_inst);

  bool handled = false;

  switch (request->_request_type) {
  case P3D_RT_stop:
    if (_p3d_inst != NULL) {
      P3D_instance_finish(_p3d_inst);
      _p3d_inst = NULL;
    }
    cleanup_window();
    // Guess the browser doesn't really care.
    handled = true;
    break;

  case P3D_RT_get_url:
    {
      PPDownloadRequest *req = 
        new PPDownloadRequest(PPDownloadRequest::RT_user, 
                              request->_request._get_url._unique_id);
      start_download(request->_request._get_url._url, req);
    }
    break;

  case P3D_RT_notify:
    // We mostly ignore notifies, since these are handled by the core
    // API.  But we do check for the "onwindowopen" notify, at which
    // point we start spamming the refresh requests.
    if (strcmp(request->_request._notify._message, "onwindowopen") == 0) {
      _python_window_open = true;
      if (_got_window) {
        NPRect rect = { 0, 0, (unsigned short)_window.height, (unsigned short)_window.width };
        browser->invalidaterect(_npp_instance, &rect);
      }
    }
    break;

  case P3D_RT_refresh:
    if (_got_window) {
      NPRect rect = { 0, 0, (unsigned short)_window.height, (unsigned short)_window.width };
      browser->invalidaterect(_npp_instance, &rect);
    }
    break;

  default:
    // Some request types are not handled.
    nout << "Unhandled request: " << request->_request_type << "\n";
    break;
  };

  P3D_request_finish(request, handled);
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::generic_browser_call
//       Access: Public, Static
//  Description: This method is called from strategically-chosen
//               browser callback functions.  Its purpose is to
//               provide another hook into the main thread callback,
//               particularly if the PluginAsyncCall function isn't
//               available.
////////////////////////////////////////////////////////////////////
void PPInstance::
generic_browser_call() {
  //#ifndef HAS_PLUGIN_THREAD_ASYNC_CALL
  // If we can't ask Mozilla to call us back using
  // NPN_PluginThreadAsyncCall(), then we'll do it explicitly now,
  // since we know we're in the main thread here.
  handle_request_loop();
  //#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::handle_event
//       Access: Public
//  Description: Called by the browser as new window events are
//               generated.  Returns true if the event is handled,
//               false if ignored.
////////////////////////////////////////////////////////////////////
bool PPInstance::
handle_event(void *event) {
  bool retval = false;

  if (_p3d_inst == NULL) {
    // Ignore events that come in before we've launched the instance.
    return retval;
  }

#ifdef __APPLE__
  EventRecord *er = (EventRecord *)event;

  switch (er->what) {
  case NPEventType_GetFocusEvent:
  case NPEventType_LoseFocusEvent:
    retval = true;
    break;

  case NPEventType_AdjustCursorEvent:
    retval = true;
    break;
  }

  P3D_event_data edata;
  edata._event = er;
  if (P3D_instance_handle_event(_p3d_inst, edata)) {
    retval = true;
  }

#endif  // __APPLE__

  return retval;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::get_panda_script_object
//       Access: Public
//  Description: Returns a toplevel object that JavaScript or whatever
//               can read and/or modify to control the instance.
////////////////////////////////////////////////////////////////////
NPObject *PPInstance::
get_panda_script_object() {
  if (_script_object != NULL) {
    // NPRuntime "steals" a reference to this object.
    browser->retainobject(_script_object);
    return _script_object;
  }

  P3D_object *main = NULL;

  if (_p3d_inst != NULL) {
    main = P3D_instance_get_panda_script_object(_p3d_inst);
  }

  _script_object = PPToplevelObject::make_new(this);
  _script_object->set_main(main);

  browser->retainobject(_script_object);
  return _script_object;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::p3dobj_to_variant
//       Access: Public
//  Description: Converts the indicated P3D_object to the equivalent
//               NPVariant, and stores it in result.
////////////////////////////////////////////////////////////////////
void PPInstance::
p3dobj_to_variant(NPVariant *result, P3D_object *object) {
  switch (P3D_OBJECT_GET_TYPE(object)) {
  case P3D_OT_undefined:
    VOID_TO_NPVARIANT(*result);
    break;

  case P3D_OT_none:
    NULL_TO_NPVARIANT(*result);
    break;

  case P3D_OT_bool:
    BOOLEAN_TO_NPVARIANT(P3D_OBJECT_GET_BOOL(object), *result);
    break;

  case P3D_OT_int:
    INT32_TO_NPVARIANT(P3D_OBJECT_GET_INT(object), *result);
    break;

  case P3D_OT_float:
    DOUBLE_TO_NPVARIANT(P3D_OBJECT_GET_FLOAT(object), *result);
    break;

  case P3D_OT_string:
    {
      int size = P3D_OBJECT_GET_STRING(object, NULL, 0);
      char *buffer = (char *)browser->memalloc(size);
      P3D_OBJECT_GET_STRING(object, buffer, size);
      STRINGN_TO_NPVARIANT(buffer, size, *result);
    }
    break;

  case P3D_OT_object:
    {
      PPPandaObject *ppobj = PPPandaObject::make_new(this, object);
      OBJECT_TO_NPVARIANT(ppobj, *result);
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::variant_to_p3dobj
//       Access: Public
//  Description: Converts the indicated NPVariant to the equivalent
//               P3D_object, and returns it (newly-allocated).  The
//               caller is responsible for freeing the returned object
//               later.
////////////////////////////////////////////////////////////////////
P3D_object *PPInstance::
variant_to_p3dobj(const NPVariant *variant) {
  if (NPVARIANT_IS_VOID(*variant)) {
    return P3D_new_undefined_object();
  } else if (NPVARIANT_IS_NULL(*variant)) {
    return P3D_new_none_object();
  } else if (NPVARIANT_IS_BOOLEAN(*variant)) {
    return P3D_new_bool_object(NPVARIANT_TO_BOOLEAN(*variant));
  } else if (NPVARIANT_IS_INT32(*variant)) {
    return P3D_new_int_object(NPVARIANT_TO_INT32(*variant));
  } else if (NPVARIANT_IS_DOUBLE(*variant)) {
    return P3D_new_float_object(NPVARIANT_TO_DOUBLE(*variant));
  } else if (NPVARIANT_IS_STRING(*variant)) {
    NPString str = NPVARIANT_TO_STRING(*variant);
    return P3D_new_string_object(str.utf8characters, str.utf8length);
  } else if (NPVARIANT_IS_OBJECT(*variant)) {
    NPObject *object = NPVARIANT_TO_OBJECT(*variant);
    if (object->_class == &PPPandaObject::_object_class) {
      // This is really a PPPandaObject.
      PPPandaObject *ppobject = (PPPandaObject *)object;
      P3D_object *obj = ppobject->get_p3d_object();
      return obj;
    }

    // It's a generic NPObject of some kind.
    return new PPBrowserObject(this, object);
  }

  // Hmm, none of the above?
  return P3D_new_none_object();
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::find_host
//       Access: Private
//  Description: Scans the <contents> element for the matching <host>
//               element.
////////////////////////////////////////////////////////////////////
void PPInstance::
find_host(TiXmlElement *xcontents) {
  string host_url = PANDA_PACKAGE_HOST_URL;
  TiXmlElement *xhost = xcontents->FirstChildElement("host");
  if (xhost != NULL) {
    const char *url = xhost->Attribute("url");
    if (url != NULL && host_url == string(url)) {
      // We're the primary host.  This is the normal case.
      read_xhost(xhost);
      return;
      
    } else {
      // We're not the primary host; perhaps we're an alternate host.
      TiXmlElement *xalthost = xhost->FirstChildElement("alt_host");
      while (xalthost != NULL) {
        const char *url = xalthost->Attribute("url");
        if (url != NULL && host_url == string(url)) {
          // Yep, we're this alternate host.
          read_xhost(xhost);
          return;
        }
        xalthost = xalthost->NextSiblingElement("alt_host");
      }
    }

    // Hmm, didn't find the URL we used mentioned.  Assume we're the
    // primary host.
    read_xhost(xhost);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::read_xhost
//       Access: Private
//  Description: Reads the host data from the <host> (or <alt_host>)
//               entry in the contents.xml file.
////////////////////////////////////////////////////////////////////
void PPInstance::
read_xhost(TiXmlElement *xhost) {
  // Get the "download" URL, which is the source from which we
  // download everything other than the contents.xml file.
  const char *download_url = xhost->Attribute("download_url");
  if (download_url != NULL) {
    _download_url_prefix = download_url;
  } else {
    _download_url_prefix = PANDA_PACKAGE_HOST_URL;
  }
  if (!_download_url_prefix.empty()) {
    if (_download_url_prefix[_download_url_prefix.size() - 1] != '/') {
      _download_url_prefix += "/";
    }
  }
        
  TiXmlElement *xmirror = xhost->FirstChildElement("mirror");
  while (xmirror != NULL) {
    const char *url = xmirror->Attribute("url");
    if (url != NULL) {
      add_mirror(url);
    }
    xmirror = xmirror->NextSiblingElement("mirror");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::add_mirror
//       Access: Private
//  Description: Adds a new URL to serve as a mirror for this host.
//               The mirrors will be consulted first, before
//               consulting the host directly.
////////////////////////////////////////////////////////////////////
void PPInstance::
add_mirror(string mirror_url) {
  // Ensure the URL ends in a slash.
  if (!mirror_url.empty() && mirror_url[mirror_url.size() - 1] != '/') {
    mirror_url += '/';
  }
  
  // Add it to the _mirrors list, but only if it's not already
  // there.
  if (find(_mirrors.begin(), _mirrors.end(), mirror_url) == _mirrors.end()) {
    _mirrors.push_back(mirror_url);
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::choose_random_mirrors
//       Access: Public
//  Description: Selects num_mirrors elements, chosen at random, from
//               the _mirrors list.  Adds the selected mirrors to
//               result.  If there are fewer than num_mirrors elements
//               in the list, adds only as many mirrors as we can get.
////////////////////////////////////////////////////////////////////
void PPInstance::
choose_random_mirrors(vector<string> &result, int num_mirrors) {
  vector<size_t> selected;

  size_t num_to_select = min(_mirrors.size(), (size_t)num_mirrors);
  while (num_to_select > 0) {
    size_t i = (size_t)(((double)rand() / (double)RAND_MAX) * _mirrors.size());
    while (find(selected.begin(), selected.end(), i) != selected.end()) {
      // Already found this i, find a new one.
      i = (size_t)(((double)rand() / (double)RAND_MAX) * _mirrors.size());
    }
    selected.push_back(i);
    result.push_back(_mirrors[i]);
    --num_to_select;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::request_ready
//       Access: Private, Static
//  Description: This function is attached as an asynchronous callback
//               to each instance; it will be notified when the
//               instance has a request ready.  This function may be
//               called in a sub-thread.
////////////////////////////////////////////////////////////////////
void PPInstance::
request_ready(P3D_instance *instance) {
  PPInstance *inst = (PPInstance *)(instance->_user_data);
  assert(inst != NULL);

#ifdef HAS_PLUGIN_THREAD_ASYNC_CALL
  // Since we are running at least Gecko 1.9, and we have this very
  // useful function, let's use it to ask the browser to call us back
  // in the main thread.
  browser->pluginthreadasynccall(inst->_npp_instance, browser_sync_callback, NULL);
#else  // HAS_PLUGIN_THREAD_ASYNC_CALL

  // If we're using an older version of Gecko, we have to do this some
  // other, OS-dependent way.

#ifdef _WIN32
  // Use a Windows message to forward this event to the main thread.

  // Get the window handle for the window associated with this
  // instance.
  const NPWindow *win = inst->get_window();
  if (win != NULL && win->type == NPWindowTypeWindow) {
    PostMessage((HWND)(win->window), WM_USER, 0, 0);
  }

#else
  // On Mac and Linux, we ignore this asynchronous event, and rely on
  // detecting it within HandleEvent() and similar callbacks.

#endif  // _WIN32

#endif  // HAS_PLUGIN_THREAD_ASYNC_CALL
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::start_download
//       Access: Private
//  Description: Initiates a download request.
////////////////////////////////////////////////////////////////////
void PPInstance::
start_download(const string &url, PPDownloadRequest *req) {
  nout << "start_download: " << url << "\n";
  if (url.substr(0, 7) == "file://") {
    // If we're "downloading" a file URL, just go read the file directly.
    downloaded_file(req, get_filename_from_url(url));
    delete req;
  } else {
    // Otherwise, ask the browser to download it.
    browser->geturlnotify(_npp_instance, url.c_str(), NULL, req);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::read_contents_file
//       Access: Private
//  Description: Reads the contents.xml file and starts the core API
//               DLL downloading, if necessary.
////////////////////////////////////////////////////////////////////
bool PPInstance::
read_contents_file(const string &contents_filename) {
  TiXmlDocument doc(contents_filename.c_str());
  if (!doc.LoadFile()) {
    return false;
  }

  TiXmlElement *xcontents = doc.FirstChildElement("contents");
  if (xcontents != NULL) {
    // Look for the <host> entry; it might point us at a different
    // download URL, and it might mention some mirrors.
    find_host(xcontents);

    // Now look for the core API package.
    TiXmlElement *xpackage = xcontents->FirstChildElement("package");
    while (xpackage != NULL) {
      const char *name = xpackage->Attribute("name");
      if (name != NULL && strcmp(name, "coreapi") == 0) {
        const char *platform = xpackage->Attribute("platform");
        if (platform != NULL && strcmp(platform, DTOOL_PLATFORM) == 0) {
          get_core_api(xpackage);
          return true;
        }
      }
    
      xpackage = xpackage->NextSiblingElement("package");
    }
  }

  // Couldn't find the coreapi package description.
  nout << "No coreapi package defined in contents file for "
       << DTOOL_PLATFORM << "\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::get_filename_from_url
//       Access: Private, Static
//  Description: Returns the actual filename referenced by a file://
//               url.
////////////////////////////////////////////////////////////////////
string PPInstance::
get_filename_from_url(const string &url) {
  string filename = url.substr(7);

  // Strip off a trailing query string.
  size_t query = filename.find('?');
  if (query != string::npos) {
    filename = filename.substr(0, query);
  }

#ifdef _WIN32 
  // On Windows, we have to munge the filename specially, because it's
  // been URL-munged.  It might begin with a leading slash as well as
  // a drive letter.  Clean up that nonsense.
  if (filename.length() >= 3 && 
      (filename[0] == '/' || filename[0] == '\\') &&
      isalpha(filename[1]) && filename[2] == ':') {
    filename = filename.substr(1);
  }
#endif  // _WIN32

  return filename;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::downloaded_file
//       Access: Private
//  Description: Called to receive the fully-downloaded contents of a
//               URL.
////////////////////////////////////////////////////////////////////
void PPInstance::
downloaded_file(PPDownloadRequest *req, const string &filename) {
  switch (req->_rtype) {
  case PPDownloadRequest::RT_contents_file:
    // Now we have the contents.xml file.  Read this to get the
    // filename and md5 hash of our core API DLL.
    if (read_contents_file(filename)) {
      // Successfully read.  Copy it into its normal place.
      string contents_filename = _root_dir + "/contents.xml";
      copy_file(filename, contents_filename);
      
    } else {
      // Error reading the contents.xml file, or in loading the core
      // API that it references.
      nout << "Unable to read contents file " << filename << "\n";

      // If there's an outstanding contents.xml file on disk, try to
      // load that one as a fallback.
      string contents_filename = _root_dir + "/contents.xml";
      if (!read_contents_file(contents_filename)) {
        nout << "Unable to read contents file " << contents_filename << "\n";
        // TODO: fail
      }
    }
    break;

  case PPDownloadRequest::RT_core_dll:
    // This is the core API DLL (or dylib or whatever).  Now that
    // we've downloaded it, we can load it.
    downloaded_plugin(filename);
    break;

  case PPDownloadRequest::RT_user:
    // Normally, RT_user requests won't come here, unless we
    // short-circuited the browser by "downloading" a file:// url.  In
    // any case, we'll now open the file and feed it to the user.
    feed_file(req, filename);
    break;

  default:
    // Don't know what this is.
    nout << "Unexpected downloaded file, type " << (int)req->_rtype << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::feed_file
//       Access: Private
//  Description: Opens the named file (extracted from a file:// URL)
//               and feeds its contents to the core API.
////////////////////////////////////////////////////////////////////
void PPInstance::
feed_file(PPDownloadRequest *req, const string &filename) {
  StreamingFileData *file_data = new StreamingFileData(req, filename, _p3d_inst);
  _file_datas.push_back(file_data);
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::get_core_api
//       Access: Private
//  Description: Checks the core API DLL file against the
//               specification in the contents file, and downloads it
//               if necessary.
////////////////////////////////////////////////////////////////////
void PPInstance::
get_core_api(TiXmlElement *xpackage) {
  _core_api_dll.load_xml(xpackage);

  if (_core_api_dll.quick_verify(_root_dir)) {
    // The DLL file is good.  Just load it.
    do_load_plugin();

  } else {
    // The DLL file needs to be downloaded.  Build up our list of
    // URL's to attempt to download it from, in reverse order.
    string url;

    // Our last act of desperation: hit the original host, with a
    // query uniquifier, to break through any caches.
    ostringstream strm;
    strm << _download_url_prefix << _core_api_dll.get_filename()
         << "?" << time(NULL);
    url = strm.str();
    _core_urls.push_back(url);

    // Before we try that, we'll hit the original host, without a
    // uniquifier.
    url = _download_url_prefix;
    url += _core_api_dll.get_filename();
    _core_urls.push_back(url);

    // And before we try that, we'll try two mirrors, at random.
    vector<string> mirrors;
    choose_random_mirrors(mirrors, 2);
    for (vector<string>::iterator si = mirrors.begin();
         si != mirrors.end(); 
         ++si) {
      url = (*si) + _core_api_dll.get_filename();
      _core_urls.push_back(url);
    }

    // Now pick the first URL off the list, and try it.
    assert(!_core_urls.empty());
    url = _core_urls.back();
    _core_urls.pop_back();

    PPDownloadRequest *req = new PPDownloadRequest(PPDownloadRequest::RT_core_dll);
    start_download(url, req);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::downloaded_plugin
//       Access: Private
//  Description: The core API DLL has been successfully downloaded;
//               copy it into place.
////////////////////////////////////////////////////////////////////
void PPInstance::
downloaded_plugin(const string &filename) {
  // We could have been downloading this file as a stream, but that
  // would cause problems with multiple instances downloading the
  // plugin at the same time.  Instead, we let them all download the
  // file asfile, and then only one of them is allowed to copy it into
  // place.

  if (is_plugin_loaded()) {
    // Some other instance got there first.  Just get started.
    create_instance();
    return;
  }

  // Make sure the DLL was correctly downloaded before continuing.
  if (!_core_api_dll.quick_verify_pathname(filename)) {
    nout << "After download, " << _core_api_dll.get_filename() << " is no good.\n";

    // That DLL came out wrong.  Try the next URL.
    if (!_core_urls.empty()) {
      string url = _core_urls.back();
      _core_urls.pop_back();
      
      PPDownloadRequest *req = new PPDownloadRequest(PPDownloadRequest::RT_core_dll);
      start_download(url, req);
      return;
    }

    // TODO: fail
    return;
  }

  // Copy the file onto the target.
  string pathname = _core_api_dll.get_pathname(_root_dir);
  if (!copy_file(filename, pathname)) {
    nout << "Couldn't copy " << pathname << "\n";
    return;
  }

  if (!_core_api_dll.quick_verify(_root_dir)) {
    nout << "After copying, " << pathname << " is no good.\n";
    // TODO: fail
    return;
  }

  // We downloaded and installed it successfully.  Now load it.
  do_load_plugin();
  return;
}


////////////////////////////////////////////////////////////////////
//     Function: PPInstance::do_load_plugin
//       Access: Private
//  Description: Once the core API DLL has been downloaded, loads it
//               into memory and starts the instance.
////////////////////////////////////////////////////////////////////
void PPInstance::
do_load_plugin() {
  string pathname = _core_api_dll.get_pathname(_root_dir);

#ifdef P3D_PLUGIN_P3D_PLUGIN
  // This is a convenience macro for development.  If defined and
  // nonempty, it indicates the name of the plugin DLL that we will
  // actually run, even after downloading a possibly different
  // (presumably older) version.  Its purpose is to simplify iteration
  // on the plugin DLL.
  string override_filename = P3D_PLUGIN_P3D_PLUGIN;
  if (!override_filename.empty()) {
    pathname = override_filename;
  }
#endif  // P3D_PLUGIN_P3D_PLUGIN

  nout << "Attempting to load core API from " << pathname << "\n";
  if (!load_plugin(pathname, "", "", true, "", "", "", false, false, nout)) {
    nout << "Unable to launch core API in " << pathname << "\n";
    return;
  }
  create_instance();
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

  P3D_token *tokens = NULL;
  if (!_tokens.empty()) {
    tokens = &_tokens[0];
  }
  _p3d_inst = P3D_new_instance(request_ready, tokens, _tokens.size(), 
                               0, NULL, this);

  if (_p3d_inst != NULL) {
    // Now get the browser's window object, to pass to the plugin.
    NPObject *window_object = NULL;
    if (browser->getvalue(_npp_instance, NPNVWindowNPObject,
                          &window_object) == NPERR_NO_ERROR) {
      PPBrowserObject *pobj = new PPBrowserObject(this, window_object);
      P3D_instance_set_browser_script_object(_p3d_inst, pobj);
      browser->releaseobject(window_object);
    } else {
      nout << "Couldn't get window_object\n";
    }

    if (_script_object != NULL) {
      // Now that we have a true instance, initialize our
      // script_object with the proper P3D_object pointer.
      P3D_object *main = P3D_instance_get_panda_script_object(_p3d_inst);
      _script_object->set_main(main);
    }

    if (_got_instance_url) {
      P3D_instance_start(_p3d_inst, false, _instance_url.c_str());
    }

    if (_got_window) {
      send_window();
    }
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

  int x = _window.x;
  int y = _window.y;

  P3D_window_handle parent_window;
  if (_window.type == NPWindowTypeWindow) {
    // We have a "windowed" plugin.  Parent our window to the one we
    // were given.  In this case, we should also reset the offset to
    // (0, 0), since the window we were given is already placed in the
    // right spot.
#ifdef _WIN32
    parent_window._hwnd = (HWND)(_window.window);
    x = 0;
    y = 0;

#elif defined(__APPLE__)
    NP_Port *port = (NP_Port *)_window.window;
    parent_window._port = port->port;

#elif defined(HAVE_X11)
    // We make it an 'unsigned long' instead of 'Window'
    // to avoid nppanda3d.so getting a dependency on X11.
    parent_window._xwindow = (unsigned long)(_window.window);
    x = 0;
    y = 0;
#endif

  } else {
    // We have a "windowless" plugin.  Parent our window directly to
    // the browser window.
#ifdef _WIN32
    parent_window._hwnd = 0;
    HWND hwnd;
    if (browser->getvalue(_npp_instance, NPNVnetscapeWindow,
                          &hwnd) == NPERR_NO_ERROR) {
      parent_window._hwnd = hwnd;
    }

#elif defined(__APPLE__)
    NP_Port *port = (NP_Port *)_window.window;
    parent_window._port = port->port;

#elif defined(HAVE_X11)
    parent_window._xwindow = 0;
    unsigned long win;
    if (browser->getvalue(_npp_instance, NPNVnetscapeWindow,
                          &win) == NPERR_NO_ERROR) {
      parent_window._xwindow = win;
    }
#endif
  }

#ifdef HAVE_X11
  // In the case of X11, grab the display as well.
  parent_window._xdisplay = 0;
  void* disp;
  if (browser->getvalue(_npp_instance, NPNVxDisplay,
                        &disp) == NPERR_NO_ERROR) {
    parent_window._xdisplay = disp;
  }
#endif

  P3D_window_type window_type = P3D_WT_embedded;
  if (_window.window == NULL) {
    // No parent window: it must be a hidden window.
    window_type = P3D_WT_hidden;
  } else if (_window.width == 0 || _window.height == 0) {
    // No size: hidden.
    window_type = P3D_WT_hidden;
  }    

  P3D_instance_setup_window
    (_p3d_inst, window_type,
     x, y, _window.width, _window.height,
     parent_window);
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::cleanup_window
//       Access: Private
//  Description: Called at instance shutdown, this restores the parent
//               window to its original state.
////////////////////////////////////////////////////////////////////
void PPInstance::
cleanup_window() {
  if (_got_window) {
#ifdef _WIN32
    // Restore the parent window to its own window handler.
    HWND hwnd = (HWND)_window.window;
    SetWindowLongPtr(hwnd, GWL_WNDPROC, _orig_window_proc);
    InvalidateRect(hwnd, NULL, true);
#endif  // _WIN32
    _got_window = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::copy_file
//       Access: Private
//  Description: Copies the data in the file named by from_filename
//               into the file named by to_filename.
////////////////////////////////////////////////////////////////////
bool PPInstance::
copy_file(const string &from_filename, const string &to_filename) {
  mkfile_complete(to_filename, nout);
  ifstream in(from_filename.c_str(), ios::in | ios::binary);
  ofstream out(to_filename.c_str(), ios::out | ios::binary);
        
  static const size_t buffer_size = 4096;
  char buffer[buffer_size];
  
  in.read(buffer, buffer_size);
  size_t count = in.gcount();
  while (count != 0) {
    out.write(buffer, count);
    if (out.fail()) {
      return false;
    }
    in.read(buffer, buffer_size);
    count = in.gcount();
  }

  if (!in.eof()) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::output_np_variant
//       Access: Public
//  Description: Outputs the variant value.
////////////////////////////////////////////////////////////////////
void PPInstance::
output_np_variant(ostream &out, const NPVariant &result) {
  if (NPVARIANT_IS_NULL(result)) {
    out << "null";
  } else if (NPVARIANT_IS_VOID(result)) {
    out << "void";
  } else if (NPVARIANT_IS_BOOLEAN(result)) {
    out << "bool " << NPVARIANT_TO_BOOLEAN(result);
  } else if (NPVARIANT_IS_INT32(result)) {
    out << "int " << NPVARIANT_TO_INT32(result);
  } else if (NPVARIANT_IS_DOUBLE(result)) {
    out << "double " << NPVARIANT_TO_DOUBLE(result);
  } else if (NPVARIANT_IS_STRING(result)) {
    NPString str = NPVARIANT_TO_STRING(result);
    out << "string " << string(str.utf8characters, str.utf8length);
  } else if (NPVARIANT_IS_OBJECT(result)) {
    NPObject *child = NPVARIANT_TO_OBJECT(result);
    out << "object " << child;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::handle_request_loop
//       Access: Private, Static
//  Description: Checks for any new requests from the plugin, and
//               dispatches them to the appropriate PPInstance.  This
//               function is called only in the main thread.
////////////////////////////////////////////////////////////////////
void PPInstance::
handle_request_loop() {
  if (!is_plugin_loaded()) {
    return;
  }

  P3D_instance *p3d_inst = P3D_check_request(false);
  while (p3d_inst != (P3D_instance *)NULL) {
    P3D_request *request = P3D_instance_get_request(p3d_inst);
    if (request != (P3D_request *)NULL) {
      PPInstance *inst = (PPInstance *)(p3d_inst->_user_data);
      assert(inst != NULL);
      inst->handle_request(request);
    }
    p3d_inst = P3D_check_request(false);
  }

  // Also check to see if we have any file_data objects that have
  // finished and may be deleted.
  size_t num_file_datas = _file_datas.size();
  size_t i = 0;
  while (i < num_file_datas) {
    if (!_file_datas[i]->is_done()) {
      // This one keeps going.
      ++i;
    } else {
      // This one is done.
      delete _file_datas[i];
      _file_datas.erase(_file_datas.begin() + i);
      --num_file_datas;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::browser_sync_callback
//       Access: Private, Static
//  Description: This callback hook is passed to
//               NPN_PluginThreadAsyncCall() (if that function is
//               available) to forward a request to the main thread.
//               The callback is actually called in the main thread.
////////////////////////////////////////////////////////////////////
void PPInstance::
browser_sync_callback(void *) {
  handle_request_loop();
}


#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::window_proc
//       Access: Private, Static
//  Description: We bind this function to the parent window we were
//               given in set_window, so we can spin the request_loop
//               when needed.  This is only in the Windows case; other
//               platforms rely on explicit windows events.
////////////////////////////////////////////////////////////////////
LONG PPInstance::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifndef HAS_PLUGIN_THREAD_ASYNC_CALL
  // Since we're here in the main thread, call handle_request_loop()
  // to see if there are any new requests to be serviced by the main
  // thread.
  handle_request_loop();
#endif

  switch (msg) {
  case WM_ERASEBKGND:
    // Eat the WM_ERASEBKGND message, so the browser's intervening
    // window won't overdraw on top of our own window.
    return true;

  case WM_TIMER:
  case WM_USER:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}
#endif  // _WIN32

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamingFileData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPInstance::StreamingFileData::
StreamingFileData(PPDownloadRequest *req, const string &filename,
                  P3D_instance *p3d_inst) :
  _p3d_inst(p3d_inst),
  _user_id(req->_user_id),
  _filename(filename),
  _file(filename.c_str(), ios::in | ios::binary)
{
  // First, seek to the end to get the file size.
  _file.seekg(0, ios::end);
  _file_size = _file.tellg();
  _total_count = 0;

  // Then return to the beginning to read the data.
  _file.seekg(0, ios::beg);

  // Now start up the thread.
  _thread_done = false;
  _thread_continue = true;
  INIT_THREAD(_thread);

  SPAWN_THREAD(_thread, thread_run, this);
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamingFileData::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPInstance::StreamingFileData::
~StreamingFileData() {
  // Time to stop.
  _thread_continue = false;

  JOIN_THREAD(_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamingFileData::is_done
//       Access: Public
//  Description: Returns true if the file has been fully read and this
//               object is ready to be deleted, or false if there is
//               more work to do.
////////////////////////////////////////////////////////////////////
bool PPInstance::StreamingFileData::
is_done() const {
  return _thread_done;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamingFileData::thread_run
//       Access: Private
//  Description: The main function of the file thread.  This reads the
//               file contents and feeds it to the core API.
////////////////////////////////////////////////////////////////////
void PPInstance::StreamingFileData::
thread_run() {
  static const size_t buffer_size = 81920;
  //static const size_t buffer_size = 512;
  char buffer[buffer_size];

  _file.read(buffer, buffer_size);
  size_t count = _file.gcount();
  while (count != 0) {
    _total_count += count;
    bool download_ok = P3D_instance_feed_url_stream
      (_p3d_inst, _user_id, P3D_RC_in_progress,
       0, _file_size, (const unsigned char *)buffer, count);

    if (!download_ok) {
      // Never mind.
      _thread_done = true;
      return;
    }

    if (!_thread_continue) {
      // Interrupted by the main thread.  Presumably we're being shut
      // down.
      _thread_done = true;
      return;
    }

    // So far, so good.  Read some more.
    _file.read(buffer, buffer_size);
    count = _file.gcount();

    // This is useful for development, to slow things down enough to
    // see the progress bar move.
#ifdef _WIN32
    Sleep(10);
#else
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    select(0, NULL, NULL, NULL, &tv);
#endif
  }

  // End of file.
  P3D_result_code result = P3D_RC_done;
  if (_file.fail() && !_file.eof()) {
    // Got an error while reading.
    result = P3D_RC_generic_error;
  }

  P3D_instance_feed_url_stream
    (_p3d_inst, _user_id, result, 0, _total_count, NULL, 0);

  // All done.
  _thread_done = true;
}
