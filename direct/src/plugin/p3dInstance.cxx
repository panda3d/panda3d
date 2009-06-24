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

#include <sstream>
#include <algorithm>

#ifdef _WIN32
typedef P3DWinSplashWindow SplashWindowType;
#else
typedef P3DSplashWindow SplashWindowType;
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

  DESTROY_LOCK(_request_lock);

  // Tell all of the packages that we're no longer in business for
  // them.
  Packages::iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    (*pi)->cancel_instance(this);
  }
  _packages.clear();

  if (_splash_window != NULL) {
    nout << "Deleting splash window in destructor\n" << flush;
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

  // Maybe create the splash window.
  if (!_instance_window_opened && _got_wparams) {
    if (_splash_window == NULL) {
      _splash_window = new SplashWindowType(this);
    }
  }

  // This also sets up some internal data based on the contents of the
  // above file and the associated tokens.

  // For the moment, all sessions will be unique.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  ostringstream strm;
  strm << inst_mgr->get_unique_session_index();
  _session_key = strm.str();

  _python_version = "python24";
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
      _splash_window = new SplashWindowType(this);
    } else {
      _splash_window->set_wparams(_wparams);
    }
  }

  // Update the instance in the sub-process.
  if (_session != NULL) {
    TiXmlDocument *doc = new TiXmlDocument;
    TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "setup_window");
    xcommand->SetAttribute("id", get_instance_id());
    TiXmlElement *xwparams = _wparams.make_xml();
    
    doc->LinkEndChild(decl);
    doc->LinkEndChild(xcommand);
    xcommand->LinkEndChild(xwparams);

    _session->send_command(doc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::has_property
//       Access: Public
//  Description: Returns true if the instance has the named property,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
has_property(const string &property_name) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_property
//       Access: Public
//  Description: Returns the value of the named property, or empty
//               string if there is no such property.  Properties are
//               created by the script run within the instance; they
//               are used for communicating between scripting
//               languages (for instance, communication between the
//               Python-based Panda application, and the Javascript on
//               the containing web page).
////////////////////////////////////////////////////////////////////
string P3DInstance::
get_property(const string &property_name) const {
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_property
//       Access: Public
//  Description: Changes the value of the named property.  It is an
//               error to call this on a property that does not
//               already exist.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_property(const string &property_name, const string &value) {
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
    if (result->_request_type == P3D_RT_notify) {
      // If we received a notify request, process the notification
      // immediately--it might be interesting to this instance.
      const char *message = result->_request._notify._message;
      if (strcmp(message, "window_opened") == 0) {
        // The process told us that it just succesfully opened its
        // window.
        nout << "Instance " << this << " got window_opened\n" << flush;
        _instance_window_opened = true;
        if (_splash_window != NULL) {
          nout << "Deleting splash window\n" << flush;
          delete _splash_window;
          _splash_window = NULL;
        }
      }
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
  nout << "Instance " << this << " add_request(" << request->_request_type
       << ")\n" << flush;
  request->_instance = this;

  ACQUIRE_LOCK(_request_lock);
  _pending_requests.push_back(request);
  _request_pending = true;
  RELEASE_LOCK(_request_lock);

  // Asynchronous notification for anyone who cares.
  if (_func != NULL) {
    _func(this);
  }

  // Synchronous notification for pollers.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->signal_request_ready();
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
    nout << "completed download " << unique_id << "\n" << flush;
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

  nout << "beginning download " << download_id << ": " << download->get_url()
       << "\n" << flush;

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
  xinstance->SetAttribute("id", _instance_id);

  TiXmlElement *xfparams = _fparams.make_xml();
  xinstance->LinkEndChild(xfparams);

  if (_got_wparams) {
    TiXmlElement *xwparams = _wparams.make_xml();
    xinstance->LinkEndChild(xwparams);
  }

  return xinstance;
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
