// Filename: p3dInstance.cxx
// Created by:  drose (29May09)
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

#include "p3dInstance.h"
#include "p3dInstanceManager.h"
#include "p3dDownload.h"
#include "p3dSession.h"
#include "p3dPackage.h"
#include "p3dSplashWindow.h"
#include "p3dWinSplashWindow.h"
#include "p3dX11SplashWindow.h"
#include "p3dObject.h"
#include "p3dUndefinedObject.h"

#include <sstream>
#include <algorithm>

#ifdef _WIN32
typedef P3DWinSplashWindow SplashWindowType;
#else
#ifdef HAVE_X11
typedef P3DX11SplashWindow SplashWindowType;
#else
typedef P3DSplashWindow SplashWindowType;
#endif
#endif

int P3DInstance::_next_instance_id = 0;

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstance::
P3DInstance(P3D_request_ready_func *func, void *user_data) :
  _func(func)
{
  _browser_script_object = NULL;
  _user_data = user_data;
  _request_pending = false;
  _got_fparams = false;
  _got_wparams = false;

  _instance_id = _next_instance_id;
  ++_next_instance_id;

  INIT_LOCK(_request_lock);

  _session = NULL;
  _splash_window = NULL;
  _instance_window_opened = false;
  _requested_stop = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstance::
~P3DInstance() {
  assert(_session == NULL);

  P3D_OBJECT_XDECREF(_browser_script_object);

  DESTROY_LOCK(_request_lock);

  // Tell all of the packages that we're no longer in business for
  // them.
  Packages::iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    (*pi)->cancel_instance(this);
  }
  _packages.clear();

  if (_splash_window != NULL) {
    delete _splash_window;
    _splash_window = NULL;
  }

  // TODO: empty _pending_requests queue and _downloads map.

  // TODO: Is it possible for someone to delete an instance while a
  // download is still running?  Who will crash when this happens?
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_fparams
//       Access: Public
//  Description: Sets up the initial file parameters for the instance.
//               Normally this is only called once.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_fparams(const P3DFileParams &fparams) {
  _got_fparams = true;
  _fparams = fparams;

  // This also sets up some internal data based on the contents of the
  // above file and the associated tokens.

  // For the moment, all sessions will be unique.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  ostringstream strm;
  strm << inst_mgr->get_unique_session_index();
  _session_key = strm.str();

  // TODO.
  _python_version = "python24";


  // Maybe create the splash window.
  if (!_instance_window_opened && _got_wparams) {
    if (_splash_window == NULL) {
      make_splash_window();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_wparams
//       Access: Public
//  Description: Changes the window parameters, e.g. to resize or
//               reposition the window; or sets the parameters for the
//               first time, creating the initial window.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_wparams(const P3DWindowParams &wparams) {
  _got_wparams = true;
  _wparams = wparams;

  // Update or create the splash window.
  if (!_instance_window_opened && _got_fparams) {
    if (_splash_window == NULL) {
      make_splash_window();
    } else {
      _splash_window->set_wparams(_wparams);
    }
  }

  // Update the instance in the sub-process.
  if (_session != NULL) {
    TiXmlDocument *doc = new TiXmlDocument;
    TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "setup_window");
    xcommand->SetAttribute("instance_id", get_instance_id());
    TiXmlElement *xwparams = _wparams.make_xml();
    
    doc->LinkEndChild(decl);
    doc->LinkEndChild(xcommand);
    xcommand->LinkEndChild(xwparams);

    _session->send_command(doc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_panda_script_object
//       Access: Public
//  Description: Returns a pointer to the top-level scriptable object
//               of the instance, to be used by JavaScript code in the
//               browser to control this program.
////////////////////////////////////////////////////////////////////
P3D_object *P3DInstance::
get_panda_script_object() const {
  assert(_session != NULL);

  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "pyobj");
  xcommand->SetAttribute("op", "get_panda_script_object");
  doc->LinkEndChild(decl);
  doc->LinkEndChild(xcommand);
  TiXmlDocument *response = _session->command_and_response(doc);

  P3D_object *result = NULL;
  if (response != NULL) {
    TiXmlElement *xresponse = response->FirstChildElement("response");
    if (xresponse != NULL) {
      TiXmlElement *xvalue = xresponse->FirstChildElement("value");
      if (xvalue != NULL) {
        result = _session->xml_to_p3dobj(xvalue);
      }
    }
    delete response;
  }

  if (result == NULL) {
    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    result = inst_mgr->new_undefined_object();
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_browser_script_object
//       Access: Public
//  Description: Stores a pointer to the top-level window object
//               of the browser, to be used by Panda code to control
//               JavaScript.  The new object's reference count is
//               incremented, and the previous object's is
//               decremented.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_browser_script_object(P3D_object *browser_script_object) {
  if (browser_script_object != _browser_script_object) {
    P3D_OBJECT_XDECREF(_browser_script_object);
    _browser_script_object = browser_script_object;
    if (_browser_script_object != NULL) {
      P3D_OBJECT_INCREF(_browser_script_object);
    }

    if (_session != NULL) {
      send_browser_script_object();
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::has_request
//       Access: Public
//  Description: Returns true if the instance has any pending requests
//               at the time of this call, false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
has_request() {
  ACQUIRE_LOCK(_request_lock);
  bool any_requests = !_pending_requests.empty();
  RELEASE_LOCK(_request_lock);
  return any_requests;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_request
//       Access: Public
//  Description: Returns a newly-allocated P3D_request corresponding
//               to the pending request for the host, or NULL if there
//               is no pending request.  If the return value is
//               non-NULL, it should eventually be passed back to
//               finish_request() for cleanup.
////////////////////////////////////////////////////////////////////
P3D_request *P3DInstance::
get_request() {
  P3D_request *result = NULL;
  ACQUIRE_LOCK(_request_lock);
  if (!_pending_requests.empty()) {
    result = _pending_requests.front();
    _pending_requests.pop_front();
    _request_pending = !_pending_requests.empty();
  }
  RELEASE_LOCK(_request_lock);

  if (result != NULL) {
    switch (result->_request_type) {
    case P3D_RT_notify:
      handle_notify_request(result);
      break;

    case P3D_RT_script:
      handle_script_request(result);
      // Completely eat this request; don't return it to the caller.
      finish_request(result, true);
      return get_request();

    default:
      // Other kinds of requests don't require special handling at
      // this level; pass it up unmolested.
      break;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_request
//       Access: Public
//  Description: May be called in any thread to add a new P3D_request
//               to the pending_request queue for this instance.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_request(P3D_request *request) {
  request->_instance = this;

  ACQUIRE_LOCK(_request_lock);
  _pending_requests.push_back(request);
  _request_pending = true;
  RELEASE_LOCK(_request_lock);

  // Tell the world we've got a new request.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->signal_request_ready(this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::finish_request
//       Access: Public
//  Description: Deallocates a previously-returned request from
//               get_request().  If handled is true, the request has
//               been handled by the host; otherwise, it has been
//               ignored.
////////////////////////////////////////////////////////////////////
void P3DInstance::
finish_request(P3D_request *request, bool handled) {
  assert(request != NULL);

  switch (request->_request_type) {
  case P3D_RT_stop:
    break;

  case P3D_RT_get_url:
    free((char *)request->_request._get_url._url);
    break;

  case P3D_RT_post_url:
    free((char *)request->_request._post_url._url);
    free((char *)request->_request._post_url._post_data);
    break;

  case P3D_RT_notify:
    free((char *)request->_request._notify._message);
    break;

  case P3D_RT_script:
    {
      P3D_OBJECT_DECREF(request->_request._script._object);
      if (request->_request._script._property_name != NULL) {
        free((char *)request->_request._script._property_name);
      }
      for (int i = 0; i < request->_request._script._num_values; ++i) {
        P3D_OBJECT_DECREF(request->_request._script._values[i]);
      }
    }
    break;
  }

  delete request;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::feed_url_stream
//       Access: Public
//  Description: Called by the host in response to a get_url or
//               post_url request, this sends the data retrieved from
//               the requested URL, a piece at a time.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
feed_url_stream(int unique_id,
                P3D_result_code result_code,
                int http_status_code, 
                size_t total_expected_data,
                const unsigned char *this_data, 
                size_t this_data_size) {
  Downloads::iterator di = _downloads.find(unique_id);
  if (di == _downloads.end()) {
    nout << "Unexpected feed_url_stream for " << unique_id << "\n"
         << flush;
    // Don't know this request.
    return false;
  }

  P3DDownload *download = (*di).second;
  bool download_ok = download->feed_url_stream
    (result_code, http_status_code, total_expected_data,
     this_data, this_data_size);

  if (!download_ok || download->get_download_finished()) {
    // All done.
    _downloads.erase(di);
    delete download;
  }

  return download_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_package
//       Access: Public
//  Description: Adds the package to the list of packages used by this
//               instance.  The instance will share responsibility for
//               downloading the package will any of the other
//               instances that use the same package.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_package(P3DPackage *package) {
  assert(find(_packages.begin(), _packages.end(), package) == _packages.end());

  _packages.push_back(package);
  package->set_instance(this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::start_download
//       Access: Public
//  Description: Adds a newly-allocated P3DDownload object to the
//               download queue, and issues the request to start it
//               downloading.  As the download data comes in, it will
//               be fed to the download object.  After
//               download_finished() has been called, the P3DDownload
//               object will be deleted.
////////////////////////////////////////////////////////////////////
void P3DInstance::
start_download(P3DDownload *download) {
  assert(download->get_download_id() == 0);

  int download_id = _next_instance_id;
  ++_next_instance_id;
  download->set_download_id(download_id);

  bool inserted = _downloads.insert(Downloads::value_type(download_id, download)).second;
  assert(inserted);

  P3D_request *request = new P3D_request;
  request->_request_type = P3D_RT_get_url;
  request->_request._get_url._url = strdup(download->get_url().c_str());
  request->_request._get_url._unique_id = download_id;

  add_request(request);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::request_stop
//       Access: Public
//  Description: Asks the host to shut down this particular instance,
//               presumably because the user has indicated it should
//               exit.
////////////////////////////////////////////////////////////////////
void P3DInstance::
request_stop() {
  if (!_requested_stop) {
    _requested_stop = true;
    P3D_request *request = new P3D_request;
    request->_request_type = P3D_RT_stop;
    add_request(request);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::make_xml
//       Access: Public
//  Description: Returns a newly-allocated XML structure that
//               corresponds to the data within this instance.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DInstance::
make_xml() {
  assert(_got_fparams);

  TiXmlElement *xinstance = new TiXmlElement("instance");
  xinstance->SetAttribute("instance_id", _instance_id);

  TiXmlElement *xfparams = _fparams.make_xml();
  xinstance->LinkEndChild(xfparams);

  if (_got_wparams) {
    TiXmlElement *xwparams = _wparams.make_xml();
    xinstance->LinkEndChild(xwparams);
  }

  return xinstance;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::send_browser_script_object
//       Access: Private
//  Description: Sends the XML sequence to inform the session of our
//               browser's toplevel window object.
////////////////////////////////////////////////////////////////////
void P3DInstance::
send_browser_script_object() {
  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "pyobj");
  xcommand->SetAttribute("op", "set_browser_script_object");
  if (_browser_script_object != NULL) {
    xcommand->LinkEndChild(_session->p3dobj_to_xml(_browser_script_object));
  }
  
  doc->LinkEndChild(decl);
  doc->LinkEndChild(xcommand);
  
  _session->send_command(doc);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::handle_notify_request
//       Access: Private
//  Description: Called (in the main thread) when a notify request is
//               received from the subprocess.
////////////////////////////////////////////////////////////////////
void P3DInstance::
handle_notify_request(P3D_request *request) {
  assert(request->_request_type == P3D_RT_notify);

  // We look for certain notify events that have particular meaning
  // to this instance.
  const char *message = request->_request._notify._message;
  if (strcmp(message, "onwindowopen") == 0) {
    // The process told us that it just succesfully opened its
    // window.  Tear down the splash window.
    _instance_window_opened = true;
    if (_splash_window != NULL) {
      delete _splash_window;
      _splash_window = NULL;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::handle_script_request
//       Access: Private
//  Description: Called (in the main thread) when a script request is
//               received from the subprocess.
////////////////////////////////////////////////////////////////////
void P3DInstance::
handle_script_request(P3D_request *request) {
  assert(request->_request_type == P3D_RT_script);

  P3D_object *object = request->_request._script._object;
  bool needs_response = request->_request._script._needs_response;
  int unique_id = request->_request._script._unique_id;
  switch (request->_request._script._op) {
  case P3D_SO_get_property:
    {
      P3D_object *result = P3D_OBJECT_GET_PROPERTY(object, request->_request._script._property_name);

      // We've got the property value; feed it back down to the
      // subprocess.
      TiXmlDocument *doc = new TiXmlDocument;
      TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
      TiXmlElement *xcommand = new TiXmlElement("command");
      xcommand->SetAttribute("cmd", "script_response");
      xcommand->SetAttribute("unique_id", unique_id);
      
      doc->LinkEndChild(decl);
      doc->LinkEndChild(xcommand);
      if (result != NULL) {
        xcommand->LinkEndChild(_session->p3dobj_to_xml(result));
        P3D_OBJECT_DECREF(result);
      }

      if (needs_response) {
        _session->send_command(doc);
      } else {
        delete doc;
      }
    }
    break;

  case P3D_SO_set_property:
    {
      bool result;
      if (request->_request._script._num_values == 1) {
        result = P3D_OBJECT_SET_PROPERTY(object, request->_request._script._property_name,
                                         request->_request._script._values[0]);
      } else {
        // Wrong number of values.  Error.
        result = false;
      }

      // Feed the result back down to the subprocess.
      TiXmlDocument *doc = new TiXmlDocument;
      TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
      TiXmlElement *xcommand = new TiXmlElement("command");
      xcommand->SetAttribute("cmd", "script_response");
      xcommand->SetAttribute("unique_id", unique_id);
      
      doc->LinkEndChild(decl);
      doc->LinkEndChild(xcommand);

      TiXmlElement *xvalue = new TiXmlElement("value");
      xvalue->SetAttribute("type", "bool");
      xvalue->SetAttribute("value", (int)result);
      xcommand->LinkEndChild(xvalue);
      
      if (needs_response) {
        _session->send_command(doc);
      } else {
        delete doc;
      }
    }
    break;

  case P3D_SO_del_property:
    {
      bool result = P3D_OBJECT_SET_PROPERTY(object, request->_request._script._property_name,
                                            NULL);

      // Feed the result back down to the subprocess.
      TiXmlDocument *doc = new TiXmlDocument;
      TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
      TiXmlElement *xcommand = new TiXmlElement("command");
      xcommand->SetAttribute("cmd", "script_response");
      xcommand->SetAttribute("unique_id", unique_id);
      
      doc->LinkEndChild(decl);
      doc->LinkEndChild(xcommand);

      TiXmlElement *xvalue = new TiXmlElement("value");
      xvalue->SetAttribute("type", "bool");
      xvalue->SetAttribute("value", (int)result);
      xcommand->LinkEndChild(xvalue);
      
      if (needs_response) {
        _session->send_command(doc);
      } else {
        delete doc;
      }
    }
    break;

  case P3D_SO_call:
    {
      P3D_object *result = 
        P3D_OBJECT_CALL(object, request->_request._script._property_name,
                        request->_request._script._values,
                        request->_request._script._num_values);

      // Feed the result back down to the subprocess.
      TiXmlDocument *doc = new TiXmlDocument;
      TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
      TiXmlElement *xcommand = new TiXmlElement("command");
      xcommand->SetAttribute("cmd", "script_response");
      xcommand->SetAttribute("unique_id", unique_id);
      
      doc->LinkEndChild(decl);
      doc->LinkEndChild(xcommand);

      if (result != NULL) {
        xcommand->LinkEndChild(_session->p3dobj_to_xml(result));
        P3D_OBJECT_DECREF(result);
      }
      
      if (needs_response) {
        _session->send_command(doc);
      } else {
        delete doc;
      }
    }
    break;

  case P3D_SO_eval:
    {
      P3D_object *result;
      if (request->_request._script._num_values == 1) {
        P3D_object *expression = request->_request._script._values[0];
        int size = P3D_OBJECT_GET_STRING(expression, NULL, 0);
        char *buffer = new char[size + 1];
        P3D_OBJECT_GET_STRING(expression, buffer, size + 1);
        result = P3D_OBJECT_EVAL(object, buffer);
        logfile << " eval " << *object << ": " << buffer << ", result = " << result << "\n";
        delete[] buffer;
      } else {
        // Wrong number of values.  Error.
        result = NULL;
      }

      // Feed the result back down to the subprocess.
      TiXmlDocument *doc = new TiXmlDocument;
      TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
      TiXmlElement *xcommand = new TiXmlElement("command");
      xcommand->SetAttribute("cmd", "script_response");
      xcommand->SetAttribute("unique_id", unique_id);
      
      doc->LinkEndChild(decl);
      doc->LinkEndChild(xcommand);

      if (result != NULL) {
        xcommand->LinkEndChild(_session->p3dobj_to_xml(result));
        P3D_OBJECT_DECREF(result);
      }

      logfile << "eval  response: " << *doc << "\n";
      
      if (needs_response) {
        _session->send_command(doc);
      } else {
        delete doc;
      }
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::make_splash_window
//       Access: Private
//  Description: Creates the splash window to be displayed at startup.
//               This method is called as soon as we have received
//               both _fparams and _wparams.
////////////////////////////////////////////////////////////////////
void P3DInstance::
make_splash_window() {
  assert(_splash_window == NULL);

  _splash_window = new SplashWindowType(this);

  string splash_image_url = _fparams.lookup_token("splash_img");
  if (!_fparams.has_token("splash_img")) {
    // No specific splash image is specified; get the default splash
    // image.
    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    splash_image_url = inst_mgr->get_download_url();
    splash_image_url += "coreapi/splash.jpg";
  }

  if (splash_image_url.empty()) {
    // No splash image.  Never mind.
    return;
  }

  // Make a temporary file to receive the splash image.
  char *name = tempnam(NULL, "p3d_");
  string filename = name;
  free(name);

  // Start downloading the requested splash image.
  SplashDownload *download = new SplashDownload(this);
  download->set_url(splash_image_url);
  download->set_filename(filename);

  start_download(download);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::install_progress
//       Access: Private
//  Description: Notified as the _panda3d package is downloaded.
////////////////////////////////////////////////////////////////////
void P3DInstance::
install_progress(P3DPackage *package, double progress) {
  if (_splash_window != NULL) {
    _splash_window->set_install_label("Installing Panda3D");
    _splash_window->set_install_progress(progress);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::SplashDownload::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstance::SplashDownload::
SplashDownload(P3DInstance *inst) :
  _inst(inst)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::SplashDownload::download_finished
//       Access: Protected, Virtual
//  Description: Intended to be overloaded to generate a callback
//               when the download finishes, either successfully or
//               otherwise.  The bool parameter is true if the
//               download was successful.
////////////////////////////////////////////////////////////////////
void P3DInstance::SplashDownload::
download_finished(bool success) {
  P3DFileDownload::download_finished(success);
  if (success) {
    // We've successfully downloaded the splash image.  Put it
    // onscreen if our splash window still exists.
    if (_inst->_splash_window != NULL) {
      _inst->_splash_window->set_image_filename(get_filename(), true);
    }
  }
}
