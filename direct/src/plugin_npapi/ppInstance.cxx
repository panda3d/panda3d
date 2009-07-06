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
  logfile << "constructing " << this << "\n" << flush;
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
    logfile
      << " " << i << ": " << token._keyword << " = " << token._value << "\n";
    _tokens.push_back(token);
  }

  _started_instance_data = false;
  _got_instance_data = false;
  _got_window = false;

  if (!is_plugin_loaded()) {
    // Go download the contents file, so we can download the core DLL.
    string url = P3D_PLUGIN_DOWNLOAD;
    url += "contents.xml";
    PPDownloadRequest *req = new PPDownloadRequest(PPDownloadRequest::RT_contents_file);
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
  case PPDownloadRequest::RT_contents_file:
    // This is the initial contents.xml file.  We'll just download
    // this directoy to a file, since it is small and this is easy.
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

  // Here's another temporary hack.  In addition to the weird filename
  // format, the file that Safari tells us about appears to be a
  // temporary file that Safari's about to delete.  In order to
  // protect ourselves from this, we need to either open the file
  // immediately, or copy it somewhere else.  The instance_data
  // filename can't be copied, so in the short term, we implement this
  // quick hack: if we're just downloading from "file://", then remap
  // the filename to point to the source file.
  if (strncmp(stream->url, "file://", 7) == 0) {
    filename = stream->url + 7;
    logfile << "converted filename again to " << filename << "\n";
  }

#endif  // __APPLE__

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);
  switch (req->_rtype) {
  case PPDownloadRequest::RT_contents_file:
    // Now we have the contents.xml file.  Read this to get the
    // filename and md5 hash of our core API DLL.
    logfile << "got contents file " << filename << "\n" << flush;
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
  logfile
    << "handle_request: " << request << ", " << request->_request_type
    << " within " << this << "\n" << flush;
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

  case P3D_RT_notify:
    {
      logfile << "Got P3D_RT_notify: " << request->_request._notify._message
              << "\n" << flush;

      if (_script_object != NULL &&
          strcmp(request->_request._notify._message, "onpythonload") == 0) {
        // Now that Python is running, initialize our script_object
        // with the proper P3D object pointer.
        P3D_object *obj = P3D_instance_get_panda_script_object(_p3d_inst);
        logfile << "late obj = " << obj << "\n" << flush;
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
//     Function: PPInstance::get_panda_script_object
//       Access: Public
//  Description: Returns a toplevel object that JavaScript or whatever
//               can read and/or modify to control the instance.
////////////////////////////////////////////////////////////////////
NPObject *PPInstance::
get_panda_script_object() {
  logfile << "get_panda_script_object\n" << flush;
  if (_script_object != NULL) {
    logfile << "returning _script_object ref = " << _script_object->referenceCount << "\n";
    return _script_object;
  }

  P3D_object *obj = NULL;

  if (_p3d_inst != NULL) {
    obj = P3D_instance_get_panda_script_object(_p3d_inst);
    logfile << "obj = " << obj << "\n" << flush;
  }

  _script_object = PPPandaObject::make_new(this, obj);
  logfile << "_script_object ref = " << _script_object->referenceCount << "\n";
  browser->retainobject(_script_object);
  logfile << "after retain, _script_object ref = " << _script_object->referenceCount << "\n";
  logfile << "ppobj = " << _script_object << "\n" << flush;
  return _script_object;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::object_to_variant
//       Access: Private
//  Description: Converts the indicated P3D_object to the equivalent
//               NPVariant, and stores it in result.
////////////////////////////////////////////////////////////////////
void PPInstance::
object_to_variant(NPVariant *result, const P3D_object *object) {
  switch (P3D_OBJECT_GET_TYPE(object)) {
  case P3D_OT_none:
    VOID_TO_NPVARIANT(*result);
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
      PPPandaObject *ppobj = PPPandaObject::make_new(this, P3D_OBJECT_COPY(object));
      OBJECT_TO_NPVARIANT(ppobj, *result);
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::variant_to_object
//       Access: Private
//  Description: Converts the indicated NPVariant to the equivalent
//               P3D_object, and returns it (newly-allocated).  The
//               caller is responsible for freeing the returned object
//               later.
////////////////////////////////////////////////////////////////////
P3D_object *PPInstance::
variant_to_object(const NPVariant *variant) {
  if (NPVARIANT_IS_VOID(*variant) ||
      NPVARIANT_IS_NULL(*variant)) {
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
    // TODO.
    return P3D_new_none_object();
  }

  // Hmm, none of the above?
  return P3D_new_none_object();
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
    browser->geturlnotify(_npp_instance, url.c_str(), NULL, req);
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
    logfile << "Successfully downloaded " << pathname << "\n";
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
  logfile << "loaded core API from " << pathname << "\n" << flush;
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
      logfile << "Got window_object " << window_object << "\n" << flush;
      PPBrowserObject *pobj = new PPBrowserObject(this, window_object);
      P3D_instance_set_browser_script_object(_p3d_inst, pobj);
      browser->releaseobject(window_object);
    } else {
      logfile << "Couldn't get window_object\n" << flush;
    }
    
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
  if (_window.type == NPWindowTypeWindow) {
    parent_window._hwnd = (HWND)(_window.window);
  } else {
    // Hmm, it's just a drawable; but we didn't ask for a windowless
    // plugin.
    parent_window._hwnd = 0;
  }
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

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::show_np_variant
//       Access: Private
//  Description: Outputs the variant value.
////////////////////////////////////////////////////////////////////
void PPInstance::
show_np_variant(const NPVariant &result) {
  if (NPVARIANT_IS_NULL(result)) {
    logfile << "null";
  } else if (NPVARIANT_IS_VOID(result)) {
    logfile << "void";
  } else if (NPVARIANT_IS_BOOLEAN(result)) {
    logfile << "bool " << NPVARIANT_TO_BOOLEAN(result);
  } else if (NPVARIANT_IS_INT32(result)) {
    logfile << "int " << NPVARIANT_TO_INT32(result);
  } else if (NPVARIANT_IS_DOUBLE(result)) {
    logfile << "double " << NPVARIANT_TO_DOUBLE(result);
  } else if (NPVARIANT_IS_STRING(result)) {
    NPString str = NPVARIANT_TO_STRING(result);
    logfile << "string " << string(str.utf8characters, str.utf8length);
  } else if (NPVARIANT_IS_OBJECT(result)) {
    NPObject *child = NPVARIANT_TO_OBJECT(result);
    logfile << "object " << child;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::np_variant_to_object
//       Access: Private
//  Description: Returns a freshly-allocated P3D_object corresponding
//               to the indicated NPVariant.
////////////////////////////////////////////////////////////////////
P3D_object *PPInstance::
np_variant_to_object(const NPVariant &result) {
  if (NPVARIANT_IS_NULL(result)) {
    return NULL;
  } else if (NPVARIANT_IS_VOID(result)) {
    return P3D_new_none_object();
  } else if (NPVARIANT_IS_BOOLEAN(result)) {
    return P3D_new_bool_object(NPVARIANT_TO_BOOLEAN(result));
  } else if (NPVARIANT_IS_INT32(result)) {
    return P3D_new_int_object(NPVARIANT_TO_INT32(result));
  } else if (NPVARIANT_IS_DOUBLE(result)) {
    return P3D_new_float_object(NPVARIANT_TO_DOUBLE(result));
  } else if (NPVARIANT_IS_STRING(result)) {
    NPString str = NPVARIANT_TO_STRING(result);
    return P3D_new_string_object(str.utf8characters, str.utf8length);
  } else if (NPVARIANT_IS_OBJECT(result)) {
    // TODO?
    return P3D_new_none_object();
    // NPVARIANT_TO_OBJECT(result);
  }

  // Huh, what is this?
  return NULL;
}
