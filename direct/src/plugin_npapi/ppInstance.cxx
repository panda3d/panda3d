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
#include "ppBrowserObject.h"
#include "startup.h"
#include "p3d_plugin_config.h"
#include "find_root_dir.h"
#include "mkdir_complete.h"

#include <fstream>
#include <string.h>  // strcmp()

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

  _started_instance_data = false;
  _got_instance_data = false;
  _got_window = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPInstance::
~PPInstance() {
#ifdef _WIN32
  if (_got_window) {
    // Restore the parent window to its own window handler.
    HWND hwnd = (HWND)_window.window;
    SetWindowLongPtr(hwnd, GWL_WNDPROC, _orig_window_proc);
  }
#endif  // _WIN32

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
    string url = P3D_PLUGIN_DOWNLOAD;
    url += "contents.xml";
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

  case PPDownloadRequest::RT_contents_file:
    if (reason != NPRES_DONE) {
      logfile << "Failure downloading " << url << "\n";
      // TODO: fail
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

    // Here's another temporary hack.  In addition to the weird
    // filename format, the file that Safari tells us about appears to
    // be a temporary file that Safari's about to delete.  In order to
    // protect ourselves from this, we need to temporarily copy the
    // file somewhere else.
    char *name = tempnam(NULL, "p3d_");

    // We prefer just making a hard link; it's quick and easy.
    if (link(filename.c_str(), name) == 0) {
      logfile << "linked " << filename << " to " << name << "\n";
    } else {
      // But sometimes the hard link might fail, particularly if these
      // are two different file systems.  In this case we have to open
      // the files and copy the data by hand.
      ifstream in(filename.c_str(), ios::in | ios::binary);
      ofstream out(name, ios::out | ios::binary);

      static const size_t buffer_size = 4096;
      char buffer[buffer_size];
      
      in.read(buffer, buffer_size);
      size_t count = in.gcount();
      while (count != 0) {
        out.write(buffer, count);
        in.read(buffer, buffer_size);
        count = in.gcount();
      }
      logfile << "copied " << filename << " to " << name << "\n";
    }

    filename = name;
    free(name);

    // TODO: remove this temporary file when we're done with it.
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
    {
      if (_script_object != NULL &&
          strcmp(request->_request._notify._message, "onpythonload") == 0) {
        // Now that Python is running, initialize our script_object
        // with the proper P3D object pointer.
        P3D_object *obj = P3D_instance_get_panda_script_object(_p3d_inst);
        _script_object->set_p3d_object(obj);
      }
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
//     Function: PPInstance::handle_request_loop
//       Access: Public, Static
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

  P3D_object *obj = NULL;

  if (_p3d_inst != NULL) {
    obj = P3D_instance_get_panda_script_object(_p3d_inst);
  }

  _script_object = PPPandaObject::make_new(this, obj);

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
      logfile << "Found nested Panda Object " << obj << "\n" << flush;
      return obj;
    }

    // It's a generic NPObject of some kind.
    return new PPBrowserObject(this, object);
  }

  // Hmm, none of the above?
  return P3D_new_none_object();
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::start_download
//       Access: Private
//  Description: Initiates a download request.
////////////////////////////////////////////////////////////////////
void PPInstance::
start_download(const string &url, PPDownloadRequest *req) {
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
read_contents_file(const string &filename) {
  TiXmlDocument doc(filename.c_str());
  if (!doc.LoadFile()) {
    return false;
  }

  TiXmlElement *xpackage = doc.FirstChildElement("package");
  while (xpackage != NULL) {
    const char *name = xpackage->Attribute("name");
    if (name != NULL && strcmp(name, "coreapi") == 0) {
      get_core_api(xpackage);
      return true;
    }
    
    xpackage = xpackage->NextSiblingElement("package");
  }

  // Couldn't find the core package description.
  logfile << "No core package defined in contents file.\n" << flush;
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
    if (!read_contents_file(filename)) {
      logfile << "Unable to read contents file\n";
      // TODO: fail
    }
    break;

  case PPDownloadRequest::RT_core_dll:
    // This is the core API DLL (or dylib or whatever).  Now that
    // we've downloaded it, we can load it.
    downloaded_plugin(filename);
    break;

  case PPDownloadRequest::RT_instance_data:
    // This is the instance data, e.g. the p3d filename.  Now we can
    // launch the instance.
    _got_instance_data = true;
    _p3d_filename = filename;
    create_instance();
    break;

  case PPDownloadRequest::RT_user:
    // Normally, RT_user requests won't come here, unless we
    // short-circuited the browser by "downloading" a file:// url.  In
    // any case, we'll now open the file and feed it to the user.
    feed_file(req, filename);
    break;

  default:
    // Don't know what this is.
    logfile << "Unexpected downloaded file, type " << (int)req->_rtype << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::feed_file
//       Access: Private
//  Description: Opens the named file (extracted from a file:// URL)
//               and feeds its contents to the plugin.
////////////////////////////////////////////////////////////////////
void PPInstance::
feed_file(PPDownloadRequest *req, const string &filename) {
  ifstream file(filename.c_str(), ios::in | ios::binary);

  // First, seek to the end to get the file size.
  file.seekg(0, ios::end);
  size_t file_size = file.tellg();

  // Then return to the beginning to read the data.
  file.seekg(0, ios::beg);

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  file.read(buffer, buffer_size);
  size_t count = file.gcount();
  size_t total_count = 0;
  while (count != 0) {
    bool download_ok = P3D_instance_feed_url_stream
      (_p3d_inst, req->_user_id, P3D_RC_in_progress,
       0, file_size, (const unsigned char *)buffer, count);

    if (!download_ok) {
      // Never mind.
      return;
    }
    file.read(buffer, buffer_size);
    count = file.gcount();
    total_count += count;
  }
  
  P3D_result_code result = P3D_RC_done;
  if (file.fail() && !file.eof()) {
    // Got an error while reading.
    result = P3D_RC_generic_error;
  }

  P3D_instance_feed_url_stream
    (_p3d_inst, req->_user_id, result, 0, total_count, NULL, 0);
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

  _root_dir = find_root_dir();

  if (_core_api_dll.quick_verify(_root_dir)) {
    // The DLL file is good.  Just load it.
    do_load_plugin();

  } else {
    // The DLL file needs to be downloaded.  Go get it.
    string url = P3D_PLUGIN_DOWNLOAD;
    url += _core_api_dll.get_filename();
    
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

  // Copy the file onto the target.
  string pathname = _core_api_dll.get_pathname(_root_dir);
  mkfile_complete(pathname);

  ifstream in(filename.c_str(), ios::in | ios::binary);
  ofstream out(pathname.c_str(), ios::out | ios::binary);

  static const size_t buffer_size = 4096;
  static char buffer[buffer_size];

  in.read(buffer, buffer_size);
  size_t count = in.gcount();
  while (count != 0) {
    out.write(buffer, count);
    in.read(buffer, buffer_size);
    count = in.gcount();
  }

  if (!out) {
    logfile << "Could not write " << pathname << "\n";
    // TODO: fail
    return;
  }
  in.close();
  out.close();

  if (_core_api_dll.quick_verify(_root_dir)) {
    // We downloaded and installed it successfully.  Now load it.
    do_load_plugin();
    return;
  }

  logfile << "After download, " << pathname << " is no good.\n";
  // TODO: fail
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

  if (!load_plugin(pathname)) {
    logfile << "Unable to launch core API in " << pathname << "\n" << flush;
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

  if (!_got_instance_data) {
    // No instance data yet.
    return;
  }

  if (!_got_window) {
    // No window yet.
    return;
  }

  _p3d_inst = P3D_new_instance(request_ready, this);

  if (_p3d_inst != NULL) {
    // Now get the browser's window object, to pass to the plugin.
    NPObject *window_object = NULL;
    if (browser->getvalue(_npp_instance, NPNVWindowNPObject,
                          &window_object) == NPERR_NO_ERROR) {
      PPBrowserObject *pobj = new PPBrowserObject(this, window_object);
      P3D_instance_set_browser_script_object(_p3d_inst, pobj);
      browser->releaseobject(window_object);
    } else {
      logfile << "Couldn't get window_object\n" << flush;
    }
    
    P3D_token *tokens = NULL;
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
#endif
#ifdef HAVE_X11
    // We make it an 'unsigned long' instead of 'Window'
    // to avoid nppanda3d.so getting a dependency on X11.
    parent_window._xwindow = (unsigned long)(_window.window);
#endif
    x = 0;
    y = 0;
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
#endif
#ifdef HAVE_X11
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

  P3D_instance_setup_window
    (_p3d_inst, P3D_WT_embedded,
     x, y, _window.width, _window.height,
     parent_window);
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


#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: window_proc
//       Access: Private, Static
//  Description: We bind this function to the parent window we were
//               given in set_window, so we can spin the request_loop
//               when needed.  This is only in the Windows case; other
//               platforms rely on explicit windows events.
////////////////////////////////////////////////////////////////////
LONG PPInstance::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  // Since we're here in the main thread, call handle_request_loop()
  // to see if there are any new requests to be serviced by the main
  // thread.

  // This might end up recursing repeatedly into
  // handle_request_loop().  Not sure if this is bad or not.
  // *Something* appears to be a little unstable.
  handle_request_loop();

  switch (msg) {
  case WM_ERASEBKGND:
    // Eat the WM_ERASEBKGND message, so the browser's intervening
    // window won't overdraw on top of our own window.
    return true;

  case WM_TIMER:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}
#endif  // _WIN32
