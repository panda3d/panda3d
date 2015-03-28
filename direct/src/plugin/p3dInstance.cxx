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
#include "p3dOsxSplashWindow.h"
#include "p3dX11SplashWindow.h"
#include "p3dObject.h"
#include "p3dMainObject.h"
#include "p3dUndefinedObject.h"
#include "p3dMultifileReader.h"
#include "p3dTemporaryFile.h"
#include "parse_color.h"

#include <sstream>
#include <algorithm>

#ifdef __APPLE__
#include <sys/mman.h>
#include <ApplicationServices/ApplicationServices.h>

// Lifted from NSEvent.h (which is Objective-C).
enum {
   NSAlphaShiftKeyMask = 1 << 16,
   NSShiftKeyMask      = 1 << 17,
   NSControlKeyMask    = 1 << 18,
   NSAlternateKeyMask  = 1 << 19,
   NSCommandKeyMask    = 1 << 20,
   NSNumericPadKeyMask = 1 << 21,
   NSHelpKeyMask       = 1 << 22,
   NSFunctionKeyMask   = 1 << 23,
   NSDeviceIndependentModifierFlagsMask = 0xffff0000U
};

#endif  // __APPLE__

#ifdef _WIN32
typedef P3DWinSplashWindow SplashWindowType;
#elif defined(__APPLE__) && !__LP64__
typedef P3DOsxSplashWindow SplashWindowType;
#elif defined(HAVE_X11)
typedef P3DX11SplashWindow SplashWindowType;
#else
typedef P3DSplashWindow SplashWindowType;
#endif

// The amount of time (in seconds) over which we average the total
// download time, for smoothing out the time estimate.
static const double time_average = 10.0;

// These are the various image files we might download for use in the
// splash window.  This list must match the ImageType enum.
const char *P3DInstance::_image_type_names[P3DInstance::IT_num_image_types] = {
  "download",
  "unauth",
  "ready",
  "failed",
  "launch",
  "active",
  "auth_ready",
  "auth_rollover",
  "auth_click",
  "play_ready",
  "play_rollover",
  "play_click",
  "none",  // Not really used.
};

static void
write_str(ostream &out, const wchar_t *str) {
  const wchar_t *p = str;
  while (*p != 0) {
    out << (int)*p << ' ';
    ++p;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstance::
P3DInstance(P3D_request_ready_func *func, 
            const P3D_token tokens[], size_t num_tokens, 
            int argc, const char *argv[], void *user_data) :
  _func(func)
{
  _dom_object = NULL;
  _main_object = new P3DMainObject;
  _main_object->set_instance(this);
  _user_data = user_data;
  _request_pending = false;
  _total_time_reports = 0;
  _temp_p3d_filename = NULL;
  _image_package = NULL;
  _current_background_image = IT_none;
  _current_button_image = IT_none;
  _got_fparams = false;
  _got_wparams = false;
  _p3d_trusted = false;
  _xpackage = NULL;
  _certlist_package = NULL;
  _p3dcert_package = NULL;

  _fparams.set_tokens(tokens, num_tokens);
  _fparams.set_args(argc, argv);

  nout << "Creating P3DInstance " << this << ": ";
  for (int i = 0; i < _fparams.get_num_tokens(); ++i) {
    nout << " " << _fparams.get_token_keyword(i)
         << "=\"" << _fparams.get_token_value(i) << "\"";
  }
  nout << "\n";

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  _instance_id = inst_mgr->get_unique_id();
  _hidden = (_fparams.lookup_token_int("hidden") != 0);
  _matches_run_origin = true;
  _matches_script_origin = false;
  _allow_python_dev = false;
  _keep_user_env = (_fparams.lookup_token_int("keep_user_env") != 0);
  _auto_start = (_fparams.lookup_token_int("auto_start") != 0);
  _stop_on_ready = (_fparams.lookup_token_int("stop_on_ready") != 0);
  _auto_install = true;
  if (_fparams.has_token("auto_install")) {
    _auto_install = (_fparams.lookup_token_int("auto_install") != 0);
  }
  _auth_button_clicked = false;
  _failed = false;
  _session = NULL;
  _auth_session = NULL;
  _panda3d_package = NULL;
  _splash_window = NULL;
  _instance_window_opened = false;
  _instance_window_attached = false;
  _stuff_to_download = false;
  _download_package_index = 0;
  _prev_downloaded = 0;
  _total_download_size = 0;
  _total_downloaded = 0;
  _packages_specified = false;
  _download_started = false;
  _download_complete = false;
  _instance_started = false;
  
  INIT_LOCK(_request_lock);
  _requested_stop = false;

#ifdef __APPLE__
  _shared_fd = -1;
  _shared_mmap_size = 0;
  _swbuffer = NULL;
  _reversed_buffer = NULL;
  _buffer_data = NULL;
  _data_provider = NULL;
  _buffer_color_space = NULL;
  _buffer_image = NULL;

  // We have to start with _mouse_active true; firefox doesn't send
  // activate events.
  _mouse_active = true;
  _modifiers = 0;
  _frame_timer = NULL;
#endif  // __APPLE__

  // Set some initial properties.
  _main_object->set_float_property("instanceDownloadProgress", 0.0);
  _main_object->set_float_property("downloadProgress", 0.0);
  _main_object->set_undefined_property("downloadElapsedSeconds");
  _main_object->set_undefined_property("downloadElapsedFormatted");
  _main_object->set_undefined_property("downloadRemainingSeconds");
  _main_object->set_undefined_property("downloadRemainingFormatted");
  _main_object->set_string_property("downloadPackageName", "");
  _main_object->set_string_property("downloadPackageDisplayName", "");
  _main_object->set_bool_property("downloadComplete", false);
  _main_object->set_string_property("status", "initial");

  ostringstream stream;
  stream << inst_mgr->get_plugin_major_version() << "."
         << inst_mgr->get_plugin_minor_version() << "."
         << inst_mgr->get_plugin_sequence_version();
  if (!inst_mgr->get_plugin_official_version()) {
    stream << "c";
  }
  
  // The plugin version as a single number, with three digits reserved
  // for each component.
  int numeric_version = 
    inst_mgr->get_plugin_major_version() * 1000000 + 
    inst_mgr->get_plugin_minor_version() * 1000 + 
    inst_mgr->get_plugin_sequence_version();
  if (!inst_mgr->get_plugin_official_version()) { 
    // Subtract 1 if we are not an official version.
    --numeric_version;
  }

  _main_object->set_string_property("pluginVersionString", stream.str());
  _main_object->set_int_property("pluginMajorVersion", inst_mgr->get_plugin_major_version());
  _main_object->set_int_property("pluginMinorVersion", inst_mgr->get_plugin_minor_version());
  _main_object->set_int_property("pluginSequenceVersion", inst_mgr->get_plugin_sequence_version());
  _main_object->set_bool_property("pluginOfficialVersion", inst_mgr->get_plugin_official_version());
  _main_object->set_int_property("pluginNumericVersion", numeric_version);
  _main_object->set_string_property("pluginDistributor", inst_mgr->get_plugin_distributor());
  _main_object->set_string_property("coreapiHostUrl", inst_mgr->get_coreapi_host_url());
  time_t timestamp = inst_mgr->get_coreapi_timestamp();
  _main_object->set_int_property("coreapiTimestamp", (int)timestamp);
  const char *timestamp_string = ctime(&timestamp);
  if (timestamp_string == NULL) {
    timestamp_string = "";
  }
  _main_object->set_string_property("coreapiTimestampString", timestamp_string);
  _main_object->set_string_property("coreapiVersionString", inst_mgr->get_coreapi_set_ver());

  _main_object->set_bool_property("trustedEnvironment", (int)inst_mgr->get_trusted_environment());
  _main_object->set_bool_property("consoleEnvironment", (int)inst_mgr->get_console_environment());

  // We'll start off with the "download" image displayed in the splash
  // window (when it opens), until we get stuff downloaded.
  set_background_image(IT_download);

  // We'd better ask for the image package up front, even if it turns
  // out we don't need it for this particular app.  We'll probably use
  // it eventually, and it's good to have it loaded early, so we can
  // put up a splash image (for instance, the above IT_download image)
  // while we download the real contents.
  P3DHost *host = inst_mgr->get_host(inst_mgr->get_host_url());
  _image_package = host->get_package("images", "", "", "");
  if (_image_package != NULL) {
    _image_package->add_instance(this);
  }

  // Check if the window size has been explicitly set to 0.  This
  // means we have an explicitly hidden plugin, and we should be
  // prepared not to get a wparams from the browser.
  if (_fparams.has_token("width") && _fparams.has_token("height") &&
      (_fparams.lookup_token_int("width") == 0 ||
       _fparams.lookup_token_int("height") == 0)) {
    P3D_window_handle dummy_handle;
    memset(&dummy_handle, 0, sizeof(dummy_handle));
    P3DWindowParams wparams(P3D_WT_hidden, 0, 0, 0, 0, dummy_handle);
    set_wparams(wparams);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstance::
~P3DInstance() {
  assert(_session == NULL);
  cleanup();

  if (_dom_object != NULL) {
    P3D_OBJECT_DECREF(_dom_object);
    _dom_object = NULL;
  }

  if (_main_object != NULL) {
    nout << "panda_script_object ref = "
         << _main_object->_ref_count << "\n";
    _main_object->set_instance(NULL);
    P3D_OBJECT_DECREF(_main_object);
    _main_object = NULL;
  }

  Downloads::iterator di;
  for (di = _downloads.begin(); di != _downloads.end(); ++di) {
    P3DDownload *download = (*di).second;
    if (download->get_instance() == this) {
      download->set_instance(NULL);
    }
    p3d_unref_delete(download);
  }
  _downloads.clear();

  DESTROY_LOCK(_request_lock);

  // TODO: Is it possible for someone to delete an instance while a
  // download is still running?  Who will crash when this happens?
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::cleanup
//       Access: Public
//  Description: Invalidates the instance and removes any structures
//               prior to deleting.
////////////////////////////////////////////////////////////////////
void P3DInstance::
cleanup() {
  _failed = true;

  if (_auth_session != NULL) {
    _auth_session->shutdown(false);
    p3d_unref_delete(_auth_session);
    _auth_session = NULL;
  }
    
  for (int i = 0; i < (int)IT_num_image_types; ++i) {
    _image_files[i].cleanup();
  }

  // Tell all of the packages that we're no longer in business for
  // them.
  Packages::iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    (*pi)->remove_instance(this);
  }
  _packages.clear();
  if (_image_package != NULL) {
    _image_package->remove_instance(this);
    _image_package = NULL;
  }

  if (_certlist_package != NULL) {
    _certlist_package->remove_instance(this);
    _certlist_package = NULL;
  }

  if (_p3dcert_package != NULL) {
    _p3dcert_package->remove_instance(this);
    _p3dcert_package = NULL;
  }

  if (_splash_window != NULL) {
    delete _splash_window;
    _splash_window = NULL;
  }

  if (_temp_p3d_filename != NULL) {
    delete _temp_p3d_filename;
    _temp_p3d_filename = NULL;
  }
  
  if (_xpackage != NULL) {
    delete _xpackage;
    _xpackage = NULL;
  }

#ifdef __APPLE__
  if (_frame_timer != NULL) {
    CFRunLoopTimerInvalidate(_frame_timer);
    CFRelease(_frame_timer);
    _frame_timer = NULL;
  }

  free_swbuffer();
#endif    

  TiXmlDocument *doc = NULL;
  ACQUIRE_LOCK(_request_lock);
  RawRequests::iterator ri;
  for (ri = _raw_requests.begin(); ri != _raw_requests.end(); ++ri) {
    doc = (*ri);
    delete doc;
  }
  _raw_requests.clear();
  RELEASE_LOCK(_request_lock);

  BakedRequests::iterator bi;
  for (bi = _baked_requests.begin(); bi != _baked_requests.end(); ++bi) {
    P3D_request *request = (*bi);
    finish_request(request, false);
  }
  _baked_requests.clear();

  Downloads::iterator di;
  for (di = _downloads.begin(); di != _downloads.end(); ++di) {
    P3DDownload *download = (*di).second;
    download->cancel();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_p3d_url
//       Access: Public
//  Description: Specifies a URL that should be contacted to download
//               the instance data.  Normally this, or
//               set_p3d_filename() or make_p3d_stream(), is only
//               called once.
//
//               The instance data at the other end of this URL is
//               key.  We can't start the instance until we have
//               downloaded the instance file and examined the
//               p3d_info.xml, and we know what Python version we need
//               and so forth.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_p3d_url(const string &p3d_url) {
  if (p3d_url.empty()) {
    nout << "No p3d URL specified.  Cannot run.\n";
    set_failed();
    return;
  }
  _fparams.set_p3d_url(p3d_url);

  // Save the last part of the URL as the p3d_basename, for reporting
  // purposes or whatever.
  determine_p3d_basename(p3d_url);

  // Make a temporary file to receive the instance data.
  assert(_temp_p3d_filename == NULL);
  _temp_p3d_filename = new P3DTemporaryFile(".p3d");
  _stuff_to_download = true;

  // Maybe it's time to open a splash window now.
  make_splash_window();

  // Mark the time we started downloading, so we'll know when to reveal
  // the progress bar, and we can predict the total download time.
#ifdef _WIN32
  _start_dl_tick = GetTickCount();
#else
  gettimeofday(&_start_dl_timeval, NULL);
#endif
  _show_dl_instance_progress = false;

  // Start downloading the data.
  InstanceDownload *download = new InstanceDownload(this);
  download->set_url(p3d_url);
  download->set_filename(_temp_p3d_filename->get_filename());
  if (_fparams.has_token("p3d_size")) {
    download->set_total_expected_data(_fparams.lookup_token_int("p3d_size"));
  }

  _main_object->set_string_property("status", "downloading_instance");
  start_download(download);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::make_p3d_stream
//       Access: Public
//  Description: Indicates an intention to transmit the p3d data as a
//               stream.  Should return a new unique stream ID to
//               receive it.
////////////////////////////////////////////////////////////////////
int P3DInstance::
make_p3d_stream(const string &p3d_url) {
  _fparams.set_p3d_url(p3d_url);

  // Save the last part of the URL as the p3d_basename, for reporting
  // purposes or whatever.
  determine_p3d_basename(p3d_url);

  // Make a temporary file to receive the instance data.
  assert(_temp_p3d_filename == NULL);
  _temp_p3d_filename = new P3DTemporaryFile(".p3d");
  _stuff_to_download = true;

  // Maybe it's time to open a splash window now.
  make_splash_window();

  // Mark the time we started downloading, so we'll know when to reveal
  // the progress bar.
#ifdef _WIN32
  _start_dl_tick = GetTickCount();
#else
  gettimeofday(&_start_dl_timeval, NULL);
#endif
  _show_dl_instance_progress = false;

  // Start downloading the data.
  InstanceDownload *download = new InstanceDownload(this);
  download->set_url(p3d_url);
  download->set_filename(_temp_p3d_filename->get_filename());
  if (_fparams.has_token("p3d_size")) {
    download->set_total_expected_data(_fparams.lookup_token_int("p3d_size"));
  }

  _main_object->set_string_property("status", "downloading_instance");
  return start_download(download, false);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_p3d_filename
//       Access: Public
//  Description: Specifies the file that contains the instance data.
//               Normally this is only called once.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_p3d_filename(const string &p3d_filename, const int &p3d_offset) {
  determine_p3d_basename(p3d_filename);
  priv_set_p3d_filename(p3d_filename, p3d_offset);
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
  bool prev_got_wparams = _got_wparams;
  _got_wparams = true;
  _wparams = wparams;

  nout << "set_wparams: " << wparams.get_window_type()
       << " " << wparams.get_win_width() << " " << wparams.get_win_height()
       << "\n";

  if (_hidden || _wparams.get_win_width() == 0 || _wparams.get_win_height() == 0) {
    // If we're a hidden app, or if the window has no size, then it is
    // really a hidden window, regardless of what type it claims to
    // be.
    _wparams.set_window_type(P3D_WT_hidden);
  }

  if (_wparams.get_window_type() != P3D_WT_hidden) {
    // Update or create the splash window.
    if (_splash_window != NULL) {
      _splash_window->set_wparams(_wparams);
    } else {
      make_splash_window();
    }
  }
  
  // It doesn't make much sense to go further than this point
  // if the instance is already in the failed state.
  if (is_failed()) {
    return;
  }
  
  if (_wparams.get_window_type() != P3D_WT_hidden) {
#ifdef __APPLE__
    // On Mac, we have to communicate the results of the rendering
    // back via shared memory, instead of directly parenting windows
    // to the browser.  Set up this mechanism.
    int x_size = _wparams.get_win_width();
    int y_size = _wparams.get_win_height();
    if (x_size != 0 && y_size != 0) {
      if (_swbuffer == NULL || _swbuffer->get_x_size() != x_size ||
          _swbuffer->get_y_size() != y_size) {
        // We need to open a new shared buffer.
        alloc_swbuffer();
      }
      
      if (_swbuffer == NULL) {
        nout << "Could not open swbuffer\n";
      }
    }
#endif   // __APPLE__
  }

  // Update the instance in the sub-process.
  if (_session != NULL) {
    TiXmlDocument *doc = new TiXmlDocument;
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "setup_window");
    xcommand->SetAttribute("instance_id", get_instance_id());
    TiXmlElement *xwparams = _wparams.make_xml(this);
    
    doc->LinkEndChild(xcommand);
    xcommand->LinkEndChild(xwparams);

    _session->send_command(doc);
  }

  if (!prev_got_wparams) {
    // If this was the first set_wparams call, try to start the app.
    if (_p3d_trusted && get_packages_ready()) {
      ready_to_start();
    }
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
  nout << "get_panda_script_object\n";
  return _main_object;
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
  nout << "set_browser_script_object\n";
  if (browser_script_object != _dom_object) {
    P3D_OBJECT_XDECREF(_dom_object);
    _dom_object = browser_script_object;
    if (_dom_object != NULL) {
      P3D_OBJECT_INCREF(_dom_object);
    }

    if (_session != NULL) {
      send_browser_script_object();
    }
  }

  // Query the origin: protocol, hostname, and port.  We'll use this to
  // limit access to the scripting interfaces for a particular p3d
  // file.
  _origin_protocol.clear();
  _origin_hostname.clear();
  _origin_port.clear();
  if (_dom_object != NULL) {
    P3D_object *location = P3D_OBJECT_GET_PROPERTY(_dom_object, "location");
    if (location != NULL) {
      P3D_object *protocol = P3D_OBJECT_GET_PROPERTY(location, "protocol");
      if (protocol != NULL) {
        int size = P3D_OBJECT_GET_STRING(protocol, NULL, 0);
        char *buffer = new char[size];
        P3D_OBJECT_GET_STRING(protocol, buffer, size);
        _origin_protocol = string(buffer, size);
        delete [] buffer;
        P3D_OBJECT_DECREF(protocol);
      }

      P3D_object *hostname = P3D_OBJECT_GET_PROPERTY(location, "hostname");
      if (hostname != NULL) {
        int size = P3D_OBJECT_GET_STRING(hostname, NULL, 0);
        char *buffer = new char[size];
        P3D_OBJECT_GET_STRING(hostname, buffer, size);
        _origin_hostname = string(buffer, size);
        delete [] buffer;
        P3D_OBJECT_DECREF(hostname);
      }

      P3D_object *port = P3D_OBJECT_GET_PROPERTY(location, "port");
      if (port != NULL) {
        int size = P3D_OBJECT_GET_STRING(port, NULL, 0);
        char *buffer = new char[size];
        P3D_OBJECT_GET_STRING(port, buffer, size);
        _origin_port = string(buffer, size);
        delete [] buffer;
        P3D_OBJECT_DECREF(port);
      }

      if (_origin_hostname.empty() && _origin_protocol == "file:") {
        _origin_hostname = "localhost";
      }

      if (_origin_port.empty()) {
        // Maybe the actual URL doesn't include the port, in which
        // case it is implicit.
        if (_origin_protocol == "http:") {
          _origin_port = "80";
        } else if (_origin_protocol == "https:") {
          _origin_port = "443";
        }
      }

      P3D_OBJECT_DECREF(location);
    }
  }

  nout << "origin is " << _origin_protocol << "//" << _origin_hostname;
  if (!_origin_port.empty()) {
    nout << ":" << _origin_port;
  }
  nout << "\n";
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::has_request
//       Access: Public
//  Description: Returns true if the instance has any pending requests
//               at the time of this call, false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
has_request() {
  return _request_pending;
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
  bake_requests();
  if (_baked_requests.empty()) {
    // No requests ready.
    _request_pending = false;
    return NULL;
  }

  P3D_request *request = _baked_requests.front();
  _baked_requests.pop_front();
  _request_pending = !_baked_requests.empty();

  if (request != NULL) {
    switch (request->_request_type) {
    case P3D_RT_notify:
      {
        // Also eval the associated HTML token, if any.
        string message = request->_request._notify._message;
        string expression = _fparams.lookup_token(message);
        nout << "notify: " << message << " " << expression << "\n";
        if (!expression.empty() && _dom_object != NULL) {
          P3D_object *result = P3D_OBJECT_EVAL(_dom_object, expression.c_str());
          P3D_OBJECT_XDECREF(result);
        }
      }
      break;

    case P3D_RT_stop:
      {
        // We also send an implicit message when Python requests itself
        // to shutdown.
        _main_object->set_pyobj(NULL);
        _main_object->set_string_property("status", "stopped");

        string message = "onpythonstop";
        string expression = _fparams.lookup_token(message);
        nout << "notify: " << message << " " << expression << "\n";
        if (!expression.empty() && _dom_object != NULL) {
          P3D_object *result = P3D_OBJECT_EVAL(_dom_object, expression.c_str());
          P3D_OBJECT_XDECREF(result);
        }
      }
      break;

    case P3D_RT_callback:
      {
        // And when the callback request is extracted, we make the
        // callback.
        P3D_callback_func *func = request->_request._callback._func;
        void *data = request->_request._callback._data;
        (*func)(data);
      }
      break;

    case P3D_RT_forget_package:
      {
        // We're being asked to forget a particular package.
        const char *host_url = request->_request._forget_package._host_url;
        const char *package_name = request->_request._forget_package._package_name;
        const char *package_version = request->_request._forget_package._package_version;
        if (package_version == NULL) {
          package_version = "";
        }

        P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
        P3DHost *host = inst_mgr->get_host(host_url);
        if (package_name != NULL) {
          P3DPackage *package = host->get_package(package_name, package_version, _session_platform, "");
          host->forget_package(package);
        } else {
          // If a NULL package name is given, forget the whole host.
          inst_mgr->forget_host(host);
        }
      }
      break;

    default:
      break;
    }
  }

  return request;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::bake_requests
//       Access: Public
//  Description: Copies requests from the _raw_requests queue, which
//               is built up in one or more sub-threads, into the
//               _baked_requests queue, which is publicly presented
//               to the browser.  Along the way, some requests (like
//               script requests) are handled immediately.
//
//               At the end of this call, _baked_requests will contain
//               the current set of requests pending for the browser.
//
//               This method should only be called in the main thread.
////////////////////////////////////////////////////////////////////
void P3DInstance::
bake_requests() {
  while (true) {
    // Get the latest request from the read thread.
    TiXmlDocument *doc = NULL;
    ACQUIRE_LOCK(_request_lock);
    if (!_raw_requests.empty()) {
      doc = _raw_requests.front();
      _raw_requests.pop_front();
    }
    RELEASE_LOCK(_request_lock);
    
    if (doc == NULL) {
      // No more requests to process right now.
      return;
    }

    // Now we've got a request in XML form; convert it to P3D_request
    // form.
    TiXmlElement *xrequest = doc->FirstChildElement("request");
    assert(xrequest != (TiXmlElement *)NULL);
    P3D_request *request = make_p3d_request(xrequest);
    delete doc;
    
    if (request != NULL) {
      _baked_requests.push_back(request);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_raw_request
//       Access: Public
//  Description: May be called in any thread to add a new XML request
//               to the pending_request queue for this instance.  The
//               XML document will be deleted when the request is
//               eventually handled.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_raw_request(TiXmlDocument *doc) {
  ACQUIRE_LOCK(_request_lock);
  _raw_requests.push_back(doc);
  _request_pending = true;
  RELEASE_LOCK(_request_lock);

  // We don't decode the XML yet, since we might be running in any
  // thread here.  We'll decode it in the main thread, where it's
  // safe.

  // Tell the world we've got a new request.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->signal_request_ready(this);
  if (_session != NULL) {
    _session->signal_request_ready(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_baked_request
//       Access: Public
//  Description: May be called in the main thread only to add a new
//               request to the baked_request queue.  This request
//               queue is directly passed on the browser without
//               further processing at this level.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_baked_request(P3D_request *request) {
  assert(request->_instance == NULL);
  request->_instance = this;
  ref();

  _baked_requests.push_back(request);
  _request_pending = true;

  // Tell the world we've got a new request.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->signal_request_ready(this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::finish_request
//       Access: Public, Static
//  Description: Deallocates a previously-returned request from
//               get_request().  If handled is true, the request has
//               been handled by the host; otherwise, it has been
//               ignored.
////////////////////////////////////////////////////////////////////
void P3DInstance::
finish_request(P3D_request *request, bool handled) {
  assert(request != NULL);
  if (request->_instance == NULL) {
    nout << "Ignoring empty request " << request << "\n";
    return;
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (inst_mgr->validate_instance(request->_instance) == NULL) {
    //    nout << "Ignoring unknown request " << request << "\n";
    return;
  }

  switch (request->_request_type) {
  case P3D_RT_stop:
    break;

  case P3D_RT_get_url:
    if (request->_request._get_url._url != NULL) {
      free((char *)request->_request._get_url._url);
      request->_request._get_url._url = NULL;
    }
    break;

  case P3D_RT_notify:
    if (request->_request._notify._message != NULL) {
      free((char *)request->_request._notify._message);
      request->_request._notify._message = NULL;
    }
    break;

  case P3D_RT_forget_package:
    if (request->_request._forget_package._host_url != NULL) {
      free((char *)request->_request._forget_package._host_url);
      request->_request._forget_package._host_url = NULL;
    }
    if (request->_request._forget_package._package_name != NULL) {
      free((char *)request->_request._forget_package._package_name);
      request->_request._forget_package._package_name = NULL;
    }
    if (request->_request._forget_package._package_version != NULL) {
      free((char *)request->_request._forget_package._package_version);
      request->_request._forget_package._package_version = NULL;
    }
    break;

  default:
    break;
  }

  p3d_unref_delete(((P3DInstance *)request->_instance));
  request->_instance = NULL;

  delete request;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::feed_url_stream
//       Access: Public
//  Description: Called by the host in response to a get_url request,
//               this sends the data retrieved from the requested URL,
//               a piece at a time.
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
    nout << "Unexpected feed_url_stream for " << unique_id << "\n";
    // Don't know this request.
    return false;
  }

  P3DDownload *download = (*di).second;
  assert(download->get_instance() == this);
  bool download_ok = download->feed_url_stream
    (result_code, http_status_code, total_expected_data,
     this_data, this_data_size);

  if (!download_ok || download->get_download_finished()) {
    // All done.
    if (download->get_instance() == this) {
      download->set_instance(NULL);
    }
    _downloads.erase(di);
    p3d_unref_delete(download);
  }

  return download_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::handle_event
//       Access: Public
//  Description: Responds to the os-generated window event.  Returns
//               true if the event is handled, false if ignored.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
handle_event(const P3D_event_data &event) {
  bool retval = false;
  if (_splash_window != NULL) {
    if (_splash_window->handle_event(event)) {
      retval = true;
    }
  }

#if defined(__APPLE__)
  if (event._event_type == P3D_ET_osx_event_record) {
    retval = handle_event_osx_event_record(event);
  } else if (event._event_type == P3D_ET_osx_cocoa) {
    retval = handle_event_osx_cocoa(event);
  } else {
    assert(false);
  }
#endif  // __APPLE__

  return retval;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_log_pathname
//       Access: Public
//  Description: Returns the log filename for this particular session,
//               if the session was started and if it has a log file.
//               Returns empty string if the session never started or
//               if it lacks a log file.
//
//               This is the same value returned by
//               P3DSession::get_log_pathname(), except that it
//               remains valid even after the session has closed.
////////////////////////////////////////////////////////////////////
const string &P3DInstance::
get_log_pathname() const {
  if (_session != NULL) {
    return _session->_log_pathname;
  }
  return _log_pathname;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_package
//       Access: Public
//  Description: Adds the package to the list of packages used by this
//               instance.  The instance will share responsibility for
//               downloading the package with any of the other
//               instances that use the same package.
//
//               The seq value should be the expected minimum
//               package_seq value for the indicated package.  If the
//               given seq value is higher than the package_seq value
//               in the contents.xml file cached for the host, it is a
//               sign that the contents.xml file is out of date and
//               needs to be redownloaded.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_package(const string &name, const string &version, const string &seq,
            P3DHost *host) {
  string alt_host = _fparams.lookup_token("alt_host");

  // Look up in the p3d_info.xml file to see if this p3d file has
  // a specific alt_host indication for this host_url.
  string alt_host_url = find_alt_host_url(host->get_host_url(), alt_host);
  if (!alt_host_url.empty()) {
    // If it does, we go ahead and switch to that host now,
    // instead of bothering to contact the original host.
    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    host = inst_mgr->get_host(alt_host_url);
    alt_host.clear();
  }

  if (!host->has_contents_file()) {
    // Since we haven't downloaded this host's contents.xml file yet,
    // get its additional host information.
    get_host_info(host);
  }

  P3DPackage *package = host->get_package(name, version, _session_platform,
                                          seq, alt_host);
  add_package(package);
}
    
////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_package
//       Access: Public
//  Description: Adds the package to the list of packages used by this
//               instance.  The instance will share responsibility for
//               downloading the package with any of the other
//               instances that use the same package.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_package(P3DPackage *package) {
  if (find(_packages.begin(), _packages.end(), package) != _packages.end()) {
    // Already have this package.
    return;
  }

  if (package->get_package_name() == "panda3d") {
    _panda3d_package = package;
  }

  _packages.push_back(package);

  // This call must be at the end of this method, because it might
  // ultimately start the application before it returns (if this was
  // the last required package).
  package->add_instance(this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::remove_package
//       Access: Public
//  Description: Indicates that the given package is destructing and
//               this instance should no longer retain a pointer to
//               it.  This is normally called only by the P3DPackage
//               destructor, and it invalidates the instance.
////////////////////////////////////////////////////////////////////
void P3DInstance::
remove_package(P3DPackage *package) {
  Packages::iterator pi = find(_packages.begin(), _packages.end(), package);
  if (pi != _packages.end()) {
    _packages.erase(pi);
  }
  pi = find(_downloading_packages.begin(), _downloading_packages.end(), package);
  if (pi != _downloading_packages.end()) {
    _downloading_packages.erase(pi);
  }
  if (package == _image_package) {
    _image_package = NULL;
  }
  if (package == _certlist_package) {
    _certlist_package = NULL;
  }
  if (package == _p3dcert_package) {
    _p3dcert_package = NULL;
  }
  if (package == _panda3d_package) {
    _panda3d_package = NULL;
  }

  set_failed();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_packages_info_ready
//       Access: Public
//  Description: Returns true if all of the packages required by the
//               instance have their information available and are
//               ready to be downloaded, false if one or more of them
//               is still waiting for information (or has failed).
////////////////////////////////////////////////////////////////////
bool P3DInstance::
get_packages_info_ready() const {
  if (!_packages_specified) {
    // We haven't even specified the full set of required packages yet.
    return false;
  }

  Packages::const_iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    if (!(*pi)->get_info_ready()) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_packages_ready
//       Access: Public
//  Description: Returns true if all of the packages required by the
//               instance (as specified in previous calls to
//               add_package()) have been fully downloaded and are
//               ready to run, or false if one or more of them still
//               requires downloading.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
get_packages_ready() const {
  if (!_packages_specified) {
    // We haven't even specified the full set of required packages yet.
    return false;
  }

  Packages::const_iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    if (!(*pi)->get_ready()) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_packages_failed
//       Access: Public
//  Description: Returns true if any of the packages required by the
//               instance have failed to download (and thus we will
//               never be ready).
////////////////////////////////////////////////////////////////////
bool P3DInstance::
get_packages_failed() const {
  Packages::const_iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    if ((*pi)->get_failed()) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::start_download
//       Access: Public
//  Description: Adds a newly-allocated P3DDownload object to the
//               download queue, and issues the request to start it
//               downloading.  As the download data comes in, it will
//               be fed to the download object.
//
//               This increments the P3DDownload object's reference
//               count, and will decrement it (and possibly delete the
//               object) after download_finished() has been called.
//
//               add_request should be true to actually request the
//               URL from the plugin, or false not to.  Normally, this
//               should always be set true, except in the one special
//               case of make_p3d_stream(), in which case the plugin
//               is already prepared to send the stream and doesn't
//               need to have it requested.
//
//               Returns the unique ID of this stream.
////////////////////////////////////////////////////////////////////
int P3DInstance::
start_download(P3DDownload *download, bool add_request) {
  assert(download->get_download_id() == 0);
  assert(!download->get_url().empty());

  if (is_failed()) {
    // Can't download anything more after failure.
    download->cancel();
    return 0;
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  int download_id = inst_mgr->get_unique_id();
  download->set_download_id(download_id);
  download->set_instance(this);

  download->ref();
  bool inserted = _downloads.insert(Downloads::value_type(download_id, download)).second;
  assert(inserted);

  // add_request will be false only for the initial p3d stream, which
  // the plugin already knows about.  For all other download streams,
  // add_request is true in order to ask the plugin for the stream.
  if (add_request) {
    P3D_request *request = new P3D_request;
    request->_instance = NULL;
    request->_request_type = P3D_RT_get_url;
    request->_request._get_url._url = strdup(download->get_url().c_str());
    request->_request._get_url._unique_id = download_id;
    
    add_baked_request(request);
  }

  return download_id;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::request_stop_sub_thread
//       Access: Public
//  Description: Asks the host to shut down this particular instance,
//               presumably because the user has indicated it should
//               exit.  This call may be made in any thread.
////////////////////////////////////////////////////////////////////
void P3DInstance::
request_stop_sub_thread() {
  // Atomically check _requested_stop.
  bool add_request = false;
  ACQUIRE_LOCK(_request_lock);
  if (!_requested_stop) {
    _requested_stop = true;
    add_request = true;
  }
  RELEASE_LOCK(_request_lock);

  // If we haven't requested a stop already, do it now.
  if (add_request) {
    TiXmlDocument *doc = new TiXmlDocument;
    TiXmlElement *xrequest = new TiXmlElement("request");
    xrequest->SetAttribute("rtype", "stop");
    doc->LinkEndChild(xrequest);
    
    add_raw_request(doc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::request_stop_main_thread
//       Access: Public
//  Description: Asks the host to shut down this particular instance,
//               presumably because the user has indicated it should
//               exit.  This call may only be made in the main thread.
////////////////////////////////////////////////////////////////////
void P3DInstance::
request_stop_main_thread() {
  // Atomically check _requested_stop.
  bool add_request = false;
  ACQUIRE_LOCK(_request_lock);
  if (!_requested_stop) {
    _requested_stop = true;
    add_request = true;
  }
  RELEASE_LOCK(_request_lock);

  // If we haven't requested a stop already, do it now.
  if (add_request) {
    _requested_stop = true;
    P3D_request *request = new P3D_request;
    request->_instance = NULL;
    request->_request_type = P3D_RT_stop;
    add_baked_request(request);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::request_refresh
//       Access: Public
//  Description: Asks the host to refresh the plugin window.  This is
//               only relevant for windowless plugins, for instance,
//               the way OSX plugins always run.
////////////////////////////////////////////////////////////////////
void P3DInstance::
request_refresh() {
  P3D_request *request = new P3D_request;
  request->_instance = NULL;
  request->_request_type = P3D_RT_refresh;
  add_baked_request(request);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::request_callback
//       Access: Public
//  Description: Asks the host to make a callback later.
////////////////////////////////////////////////////////////////////
void P3DInstance::
request_callback(P3D_callback_func *func, void *data) {
  P3D_request *request = new P3D_request;
  request->_instance = NULL;
  request->_request_type = P3D_RT_callback;
  request->_request._callback._func = func;
  request->_request._callback._data = data;
  add_baked_request(request);
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

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  xinstance->SetAttribute("root_dir", inst_mgr->get_root_dir());
  xinstance->SetAttribute("log_directory", inst_mgr->get_log_directory());
  xinstance->SetAttribute("verify_contents", (int)inst_mgr->get_verify_contents());

  // Tell the Panda process that it was started by a plugin that knows
  // about the new per_platform flag.
  xinstance->SetAttribute("respect_per_platform", 1);

  if (!inst_mgr->get_super_mirror().empty()) {
    xinstance->SetAttribute("super_mirror", inst_mgr->get_super_mirror());
  }


  TiXmlElement *xfparams = _fparams.make_xml();
  xinstance->LinkEndChild(xfparams);

  if (_got_wparams) {
    TiXmlElement *xwparams = _wparams.make_xml(this);
    xinstance->LinkEndChild(xwparams);
  }

  Packages::const_iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    TiXmlElement *xpackage = (*pi)->make_xml();
    xinstance->LinkEndChild(xpackage);
  }

  TiXmlElement *xmain = _session->p3dobj_to_xml(_main_object);
  xmain->SetValue("main");
  xinstance->LinkEndChild(xmain);

  return xinstance;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::splash_button_clicked_sub_thread
//       Access: Public
//  Description: Called by the P3DSplashWindow code (maybe in a
//               sub-thread) when the user clicks the button visible
//               on the splash window.  This will forward the event to
//               the main thread via the request callback mechanism.
////////////////////////////////////////////////////////////////////
void P3DInstance::
splash_button_clicked_sub_thread() {
  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlElement *xrequest = new TiXmlElement("request");
  xrequest->SetAttribute("rtype", "notify");
  xrequest->SetAttribute("message", "buttonclick");
  doc->LinkEndChild(xrequest);

  add_raw_request(doc);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::splash_button_clicked_main_thread
//       Access: Public
//  Description: Called only in the main thread, indirectly from
//               splash_button_clicked_sub_thread(), as the result of
//               the user clicking on the button visible in the splash
//               window.
////////////////////////////////////////////////////////////////////
void P3DInstance::
splash_button_clicked_main_thread() {
  if (is_failed()) {
    // Can't click the button after we've failed.
    nout << "Ignoring click for failed instance\n";
    return;
  }

  if (!_p3d_trusted) {
    auth_button_clicked();
  } else if (_session == NULL) {
    play_button_clicked();
  } else {
    nout << "Ignoring click for already-started instance\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::auth_button_clicked
//       Access: Public
//  Description: Called to authorize the p3d file by the user clicking
//               the red "auth" button.
////////////////////////////////////////////////////////////////////
void P3DInstance::
auth_button_clicked() {
  // Delete the previous session and create a new one.
  if (_auth_session != NULL) {
    _auth_session->shutdown(false);
    p3d_unref_delete(_auth_session);
    _auth_session = NULL;
  }
  
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  _auth_session = inst_mgr->authorize_instance(this);
  _auth_session->ref();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::play_button_clicked
//       Access: Public
//  Description: Called to start the game by the user clicking the
//               green "play" button, or by JavaScript calling
//               play().
////////////////////////////////////////////////////////////////////
void P3DInstance::
play_button_clicked() {
  if (_session == NULL && _p3d_trusted) {
    set_button_image(IT_none);
    if (!_download_started) {
      // Now we initiate the download.
      _auto_install = true;
      _auto_start = true;
      if (get_packages_info_ready()) {
        ready_to_install();
      }

    } else {
      set_background_image(IT_launch);
      P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
      inst_mgr->start_instance(this);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::auth_finished_sub_thread
//       Access: Public
//  Description: Called by the P3DAuthSession code in a sub-thread
//               when the auth dialog exits (for instance, because the
//               user approved the certificate, or cancelled).
////////////////////////////////////////////////////////////////////
void P3DInstance::
auth_finished_sub_thread() {
  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlElement *xrequest = new TiXmlElement("request");
  xrequest->SetAttribute("rtype", "notify");
  xrequest->SetAttribute("message", "authfinished");
  doc->LinkEndChild(xrequest);

  add_raw_request(doc);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::auth_finished_main_thread
//       Access: Public
//  Description: Called only in the main thread, indirectly from
//               auth_finished_sub_thread(), as the result of
//               the user closing the auth dialog.
////////////////////////////////////////////////////////////////////
void P3DInstance::
auth_finished_main_thread() {
  // Set this flag to indicate that the user has clicked on the red
  // "auth" button.  This eliminates the need to click on the green
  // "start" button.
  _auth_button_clicked = true;

  // After the authorization program has returned, check the signature
  // again.
  check_p3d_signature();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::uninstall_packages
//       Access: Public
//  Description: Stops the instance (if it is running) and deletes any
//               packages referenced by the instance.  This is
//               normally called by JavaScript, via
//               P3DMainObject::call_uninstall().
////////////////////////////////////////////////////////////////////
bool P3DInstance::
uninstall_packages() {
  if (_packages.empty()) {
    // If we have no packages (for instance, because we're untrusted),
    // we can't uninstall anything.
    nout << "Uninstall failed: no packages.\n";
    return false;
  }

  nout << "Uninstalling " << _packages.size() << " packages\n";

  Packages::const_iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    P3DPackage *package = (*pi);
    if (package != _image_package && package != _certlist_package &&
        package != _p3dcert_package) {
      package->uninstall();
    }
  }

  // Also clean up the start directory, if we have a custom start dir.
  // We won't do this if verify_contents is 'none'.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (inst_mgr->get_verify_contents() != P3D_VC_never) {
    string start_dir_suffix = get_start_dir_suffix();
    if (!start_dir_suffix.empty()) {
      string start_dir = inst_mgr->get_root_dir() + "/start" + start_dir_suffix;
      nout << "Cleaning up start directory " << start_dir << "\n";
      inst_mgr->delete_directory_recursively(start_dir);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::uninstall_host
//       Access: Public
//  Description: Stops the instance (if it is running) and deletes all
//               packages downloaded from any of the host(s)
//               referenced by the instance.  This is a more
//               aggressive uninstall than uninstall_packages().  This
//               is normally called by JavaScript, via
//               P3DMainObject::call_uninstall().
////////////////////////////////////////////////////////////////////
bool P3DInstance::
uninstall_host() {
  if (_packages.empty()) {
    // If we have no packages (for instance, because we're untrusted),
    // we can't uninstall anything.
    nout << "Uninstall failed: no packages.\n";
    return false;
  }

  uninstall_packages();

  // Collect the set of hosts referenced by this instance.
  set<P3DHost *> hosts;
  Packages::const_iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    P3DPackage *package = (*pi);
    if (package != _image_package && package != _certlist_package &&
        package != _p3dcert_package) {
      hosts.insert(package->get_host());
    }
  }
  nout << "Uninstalling " << hosts.size() << " hosts\n";

  // Uninstall all of them.
  set<P3DHost *>::iterator hi;
  for (hi = hosts.begin(); hi != hosts.end(); ++hi) {
    P3DHost *host = (*hi);
    host->uninstall();
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::priv_set_p3d_filename
//       Access: Private
//  Description: The private implementation of set_p3d_filename(),
//               this does all the work except for updating
//               p3d_basename.  It is intended to be called
//               internally, and might be passed a temporary filename.
////////////////////////////////////////////////////////////////////
void P3DInstance::
priv_set_p3d_filename(const string &p3d_filename, const int &p3d_offset) {
  if (!_fparams.get_p3d_filename().empty()) {
    nout << "p3d_filename already set to: " << _fparams.get_p3d_filename()
         << ", trying to set to " << p3d_filename << "\n";
    return;
  }

  _fparams.set_p3d_filename(p3d_filename);
  // The default for p3d_offset is -1, which means not to change it.
  if (p3d_offset >= 0) {
    _fparams.set_p3d_offset(p3d_offset);
  }
  _got_fparams = true;

  _main_object->set_float_property("instanceDownloadProgress", 1.0);

  // Generate a special notification: onpluginload, indicating the
  // plugin has read its parameters and is ready to be queried (even
  // if Python has not yet started).
  send_notify("onpluginload");

  if (!_mf_reader.open_read(_fparams.get_p3d_filename(), _fparams.get_p3d_offset())) {
    if (_fparams.get_p3d_offset() == 0) {
      nout << "Couldn't read " << _fparams.get_p3d_filename() << "\n";
    } else {
      nout << "Couldn't read " << _fparams.get_p3d_filename()
           << " at offset " << _fparams.get_p3d_offset() << "\n";
    }
    set_failed();
    return;
  }

  check_p3d_signature();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::determine_p3d_basename
//       Access: Private
//  Description: Determines _p3d_basename from the indicated URL.
////////////////////////////////////////////////////////////////////
void P3DInstance::
determine_p3d_basename(const string &p3d_url) {
  string file_part = p3d_url;
  size_t question = file_part.find('?');
  if (question != string::npos) {
    file_part = file_part.substr(0, question);
  }
  size_t slash = file_part.rfind('/');
  if (slash != string::npos) {
    file_part = file_part.substr(slash + 1);
  }
  _p3d_basename = file_part;

  nout << "p3d_basename = " << _p3d_basename << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::check_matches_origin
//       Access: Private
//  Description: Returns true if the indicated origin_match string,
//               one of either run_origin or script_origin from the
//               p3d_info.xml file, matches the origin of the page
//               that embedded the p3d file.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
check_matches_origin(const string &origin_match) {
  // First, separate the string up at the semicolons.
  size_t p = 0;
  size_t semicolon = origin_match.find(';');
  while (semicolon != string::npos) {
    if (check_matches_origin_one(origin_match.substr(p, semicolon - p))) {
      return true;
    }
    p = semicolon + 1;
    semicolon = origin_match.find(';', p);
  }
  if (check_matches_origin_one(origin_match.substr(p))) {
    return true;
  }

  // It doesn't match any of the semicolon-delimited strings within
  // origin_match.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::check_matches_origin_one
//       Access: Private
//  Description: Called for each semicolon-delimited string within
//               origin_match passed to check_matches_origin().
////////////////////////////////////////////////////////////////////
bool P3DInstance::
check_matches_origin_one(const string &origin_match) {
  // Do we have a protocol?
  size_t p = 0;
  size_t colon = origin_match.find(':');
  if (colon + 1 < origin_match.length() && origin_match[colon + 1] == '/') {
    // Yes.  It should therefore match the protocol we have in the origin.
    string protocol = origin_match.substr(0, colon + 1);
    if (!check_matches_component(_origin_protocol, protocol)) {
      return false;
    }
    p = colon + 2;
    // We'll support both http://hostname and http:/hostname, in case
    // the user is sloppy.
    if (p < origin_match.length() && origin_match[p] == '/') {
      ++p;
    }
    colon = origin_match.find(':', p);
  }

  // Do we have a port?
  if (colon < origin_match.length() && isdigit(origin_match[colon + 1])) {
    // Yes.  It should therefore match the port we have in the origin.
    string port = origin_match.substr(colon + 1);
    if (!check_matches_component(_origin_port, port)) {
      return false;
    }
  }

  // The hostname should also match what we have in the origin.
  string hostname = origin_match.substr(p, colon - p);
  if (!check_matches_hostname(_origin_hostname, hostname)) {
    return false;
  }

  // Everything matches.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::check_matches_hostname
//       Access: Private
//  Description: Matches the hostname of check_matches_origin:
//               the individual components of the hostname are matched
//               independently, with '**.' allowed at the beginning to
//               indicate zero or more prefixes.  Returns true on
//               match, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
check_matches_hostname(const string &orig, const string &match) {
  // First, separate both strings up at the dots.
  vector<string> orig_components;
  separate_components(orig_components, orig);

  vector<string> match_components;
  separate_components(match_components, match);

  // If the first component of match is "**", it means we accept any
  // number, zero or more, of components at the beginning of the
  // hostname.
  if (!match_components.empty() && match_components[0] == "**") {
    // Remove the leading "**"
    match_components.erase(match_components.begin());
    // Then remove any extra components from the beginning of
    // orig_components; we won't need to check them.
    if (orig_components.size() > match_components.size()) {
      size_t num_to_remove = orig_components.size() - match_components.size();
      orig_components.erase(orig_components.begin(), orig_components.begin() + num_to_remove);
    }
  }

  // Now match the remaining components one-to-one.
  if (match_components.size() != orig_components.size()) {
    return false;
  }

  vector<string>::const_iterator p = orig_components.begin();
  vector<string>::const_iterator p2 = match_components.begin();

  while (p != orig_components.end()) {
    assert(p2 != match_components.end());
    if (!check_matches_component(*p, *p2)) {
      return false;
    }
    ++p;
    ++p2;
  }

  assert(p2 == match_components.end());
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::separate_components
//       Access: Private
//  Description: Separates the indicated hostname into its components
//               at the dots.
////////////////////////////////////////////////////////////////////
void P3DInstance::
separate_components(vector<string> &components, const string &str) {
  size_t p = 0;
  size_t dot = str.find('.');
  while (dot != string::npos) {
    components.push_back(str.substr(p, dot - p));
    p = dot + 1;
    dot = str.find('.', p);
  }
  components.push_back(str.substr(p));
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::check_matches_component
//       Access: Private
//  Description: Matches a single component of check_matches_origin:
//               either protocol or port, or a single component of the
//               hostname.  Case-insensitive, and supports the '*'
//               wildcard operator to match the entire component.
//               Returns true on match, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
check_matches_component(const string &orig, const string &match) {
  if (match == "*") {
    return true;
  }

  // Case-insensitive compare.
  if (orig.length() != match.length()) {
    return false;
  }

  string::const_iterator p = orig.begin();
  string::const_iterator p2 = match.begin();

  while (p != orig.end()) {
    assert(p2 != match.end());
    if (tolower(*p) != tolower(*p2)) {
      return false;
    }
    ++p;
    ++p2;
  }

  assert(p2 == match.end());
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::check_p3d_signature
//       Access: Private
//  Description: Checks the signature(s) encoded in the p3d file, and
//               looks to see if any of them are recognized.
//
//               If the signature is recognized, calls
//               mark_p3d_trusted(); otherwise, calls
//               mark_p3d_untrusted().
////////////////////////////////////////////////////////////////////
void P3DInstance::
check_p3d_signature() {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (inst_mgr->get_trusted_environment()) {
    // If we're in a trusted environment (e.g. the panda3d command
    // line, where we've already downloaded the p3d file separately),
    // then everything is approved.
    mark_p3d_trusted();
    return;
  }

  // See if we've previously approved the certificate--any
  // certificate--that's signing this p3d file.
  int num_signatures = _mf_reader.get_num_signatures();
  for (int i = 0; i < num_signatures; ++i) {
    const P3DMultifileReader::CertChain &chain = _mf_reader.get_signature(i);

    // Here's a certificate that has signed this multifile.
    X509 *cert = chain[0]._cert;

    // Look up the certificate to see if we've stored a copy in our
    // certs dir.
    if (inst_mgr->find_cert(cert)) {
      mark_p3d_trusted();
      return;
    }
  }

  // Check the list of pre-approved certificates.
  if (_certlist_package == NULL) {
    // We have to go download this package.
    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    P3DHost *host = inst_mgr->get_host(inst_mgr->get_host_url());
    _certlist_package = host->get_package("certlist", "", "", "");
    if (_certlist_package != NULL) {
      _certlist_package->add_instance(this);
    }
    
    // When the package finishes downloading, we will come back here.
    return;
  }

  if (!_certlist_package->get_ready() && !_certlist_package->get_failed()) {
    // Wait for it to finish downloading.
    return;
  }

  // Couldn't find any approved certificates.
  mark_p3d_untrusted();
  return;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::mark_p3d_untrusted
//       Access: Private
//  Description: This is called internally when it has been determined
//               that the p3d file can't (yet) be trusted, for
//               instance because it lacks a signature, or because it
//               is signed by an unrecognized certificate.  This puts
//               up the red "auth" button and waits for the user to
//               approve the app before continuing.
////////////////////////////////////////////////////////////////////
void P3DInstance::
mark_p3d_untrusted() {
  // Failed test.
  nout << "p3d untrusted\n";
  if (is_failed()) {
    return;
  }

  if (_p3dcert_package == NULL) {
    // We have to go download this package.
    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    P3DHost *host = inst_mgr->get_host(inst_mgr->get_host_url());
    _p3dcert_package = host->get_package("p3dcert", "", "", "");
    if (_p3dcert_package != NULL) {
      _p3dcert_package->add_instance(this);
    }
    
    // When the package finishes downloading, we will come back here.
    return;
  }

  if (_p3dcert_package->get_failed()) {
    // Oh, too bad for us.  We're dependent on this package which we
    // weren't able to download for some reason.
    set_failed();
  }

  if (!_p3dcert_package->get_ready()) {
    // Wait for it to finish downloading.
    return;
  }

  // OK, we've got the authorization program; we can put up the red
  // button now.

  // Notify JS that we've got no trust of the p3d file.
  _main_object->set_bool_property("trusted", false);
  send_notify("onunauth");
  set_background_image(IT_unauth);
  set_button_image(IT_auth_ready);
  make_splash_window();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::mark_p3d_trusted
//       Access: Private
//  Description: This is called internally when it has been determined
//               that the p3d file can be trusted and started.  When
//               this is called, the p3d file will be examined and
//               made ready to start; it will not be started until
//               this is called.
////////////////////////////////////////////////////////////////////
void P3DInstance::
mark_p3d_trusted() {
  nout << "p3d trusted\n";
  // Only call this once.
  if (_p3d_trusted) {
    nout << "mark_p3d_trusted() called twice on " << _fparams.get_p3d_filename()
         << "\n";
    return;
  }

  // Extract the application desc file from the p3d file.
  stringstream sstream;
  if (!_mf_reader.extract_one(sstream, "p3d_info.xml")) {
    nout << "No p3d_info.xml file found in " << _fparams.get_p3d_filename() << "\n";
    set_failed();

  } else {
    sstream.seekg(0);
    TiXmlDocument doc;
    sstream >> doc;

    scan_app_desc_file(&doc);
  }

  // Now we've got no further need to keep the _mf_reader open.
  _mf_reader.close();

  // For the moment, all sessions will be unique.  TODO: support
  // multiple instances per session.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  ostringstream strm;
  strm << inst_mgr->get_unique_id();
  _session_key = strm.str();

  // Notify JS that we've accepted the trust of the p3d file.
  _main_object->set_bool_property("trusted", true);
  send_notify("onauth");

  // Now that we're all set up, grab the panda3d package.  We need to
  // examine this before we can start to download the remaining
  // packages.
  add_panda3d_package();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::scan_app_desc_file
//       Access: Private
//  Description: Reads the p3d_info.xml file at instance startup, to
//               determine the set of required packages and so forth.
////////////////////////////////////////////////////////////////////
void P3DInstance::
scan_app_desc_file(TiXmlDocument *doc) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  TiXmlElement *xpackage = doc->FirstChildElement("package");
  if (xpackage == NULL) {
    return;
  }
  assert(_xpackage == NULL);
  _xpackage = (TiXmlElement *)xpackage->Clone();

  TiXmlElement *xconfig = _xpackage->FirstChildElement("config");
  if (xconfig != NULL) {
    int hidden = 0;
    if (xconfig->QueryIntAttribute("hidden", &hidden) == TIXML_SUCCESS) {
      if (hidden != 0) {
        _hidden = true;
      }
    }

    const char *log_basename = xconfig->Attribute("log_basename");
    if (log_basename != NULL) {
      _log_basename = log_basename;
    }

    const char *prc_name = xconfig->Attribute("prc_name");
    if (prc_name != NULL) {
      _prc_name = prc_name;
    }

    const char *start_dir = xconfig->Attribute("start_dir");
    if (start_dir != NULL) {
      _start_dir = start_dir;
    }

    const char *run_origin = xconfig->Attribute("run_origin");
    if (run_origin != NULL) {
      _matches_run_origin = check_matches_origin(run_origin);
    }

    const char *script_origin = xconfig->Attribute("script_origin");
    if (script_origin != NULL) {
      _matches_script_origin = check_matches_origin(script_origin);
    }

    int allow_python_dev = 0;
    if (xconfig->QueryIntAttribute("allow_python_dev", &allow_python_dev) == TIXML_SUCCESS) {
      _allow_python_dev = (allow_python_dev != 0);
    }

    int keep_user_env = 0;
    if (xconfig->QueryIntAttribute("keep_user_env", &keep_user_env) == TIXML_SUCCESS) {
      _keep_user_env = (keep_user_env != 0);
    }

    int auto_start = 0;
    if (xconfig->QueryIntAttribute("auto_start", &auto_start) == TIXML_SUCCESS) {
      _auto_start = (auto_start != 0);
    }

    int auto_install = 0;
    if (xconfig->QueryIntAttribute("auto_install", &auto_install) == TIXML_SUCCESS) {
      _auto_install = (auto_install != 0);
    }
  }

  nout << "_matches_run_origin = " << _matches_run_origin << "\n";
  nout << "_matches_script_origin = " << _matches_script_origin << "\n";

  if (inst_mgr->get_trusted_environment()) {
    // If we're in a trusted environment, it is as if the origin
    // always matches.
    _matches_run_origin = true;
    _matches_script_origin = true;
  }

  if (_auth_button_clicked) {
    // But finally, if the user has already clicked through the red
    // "auth" button, no need to present him/her with another green
    // "play" button as well.
    _auto_install = true;
    _auto_start = true;
  }

  nout << "_auto_install = " << _auto_install 
       << ", _auto_start = " << _auto_start 
       << ", _stop_on_ready = " << _stop_on_ready
       << "\n";

  if (_hidden && _got_wparams) {
    _wparams.set_window_type(P3D_WT_hidden);
  }

  if (!_matches_run_origin) {
    nout << "Cannot run " << _p3d_basename << " from origin " 
         << _origin_protocol << "//" << _origin_hostname
         << ":" << _origin_port << "\n";
    set_failed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_panda3d_package
//       Access: Private
//  Description: Adds the "panda3d" package only.  This package must
//               be downloaded first, and its desc file examined,
//               before we can begin downloading the other packages.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_panda3d_package() {
  assert(!_packages_specified);
  assert(_panda3d_package == NULL);
  if (_xpackage == NULL) {
    return;
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  TiXmlElement *xrequires = _xpackage->FirstChildElement("requires");
  while (xrequires != NULL) {
    const char *name = xrequires->Attribute("name");
    const char *host_url = xrequires->Attribute("host");
    if (name != NULL && host_url != NULL && strcmp(name, "panda3d") == 0) {
      const char *version = xrequires->Attribute("version");
      if (version == NULL) {
        version = "";
      }
      const char *seq = xrequires->Attribute("seq");
      if (seq == NULL) {
        seq = "";
      }
      P3DHost *host = inst_mgr->get_host(host_url);
      add_package(name, version, seq, host);
      return;
    }

    xrequires = xrequires->NextSiblingElement("requires");
  }

  nout << "No panda3d package found in " << _fparams.get_p3d_filename() << "\n";
  set_failed();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_packages
//       Access: Private
//  Description: Adds the set of packages required by this p3d file to
//               the _packages member.  If _auto_install is true, this
//               will also start downloading them.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_packages() {
  assert(!_packages_specified);
  if (_xpackage == NULL) {
    return;
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  TiXmlElement *xrequires = _xpackage->FirstChildElement("requires");
  while (xrequires != NULL) {
    const char *name = xrequires->Attribute("name");
    const char *host_url = xrequires->Attribute("host");
    if (name != NULL && host_url != NULL) {
      const char *version = xrequires->Attribute("version");
      if (version == NULL) {
        version = "";
      }
      const char *seq = xrequires->Attribute("seq");
      if (seq == NULL) {
        seq = "";
      }
      P3DHost *host = inst_mgr->get_host(host_url);
      add_package(name, version, seq, host);
    }

    xrequires = xrequires->NextSiblingElement("requires");
  }

  _packages_specified = true;

  consider_start_download();

  // Now that we've scanned the p3d file, and prepared the list of
  // packages, it's safe to set the trusted flag.
  _p3d_trusted = true;

  // If the packages are already downloaded, start the instance
  // rolling.
  if (get_packages_ready()) {
    mark_download_complete();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::find_alt_host_url
//       Access: Private
//  Description: Looks in the p3d_info.xml file for the alt_host
//               associated with the indicated host_url, if any.
//               Returns empty string if there is no match.
////////////////////////////////////////////////////////////////////
string P3DInstance::
find_alt_host_url(const string &host_url, const string &alt_host) {
  TiXmlElement *xhost = _xpackage->FirstChildElement("host");
  while (xhost != NULL) {
    const char *url = xhost->Attribute("url");
    if (url != NULL && host_url == url) {
      // This matches the host.  Now do we have a matching alt_host
      // keyword for this host?
      TiXmlElement *xalt_host = xhost->FirstChildElement("alt_host");
      while (xalt_host != NULL) {
        const char *keyword = xalt_host->Attribute("keyword");
        if (keyword != NULL && alt_host == keyword) {
          const char *alt_host_url = xalt_host->Attribute("url");
          if (alt_host_url != NULL) {
            return alt_host_url;
          }
        }
        xalt_host = xalt_host->NextSiblingElement("alt_host");
      }
    }
    xhost = xhost->NextSiblingElement("host");
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_host_info
//       Access: Private
//  Description: Looks in the p3d_info.xml file for the auxiliary host
//               information for the selected host.  Some of this
//               information is helpful to have before the host has
//               read its own contents.xml file (particularly the
//               host_dir specification).
////////////////////////////////////////////////////////////////////
void P3DInstance::
get_host_info(P3DHost *host) {
  // We should only call this function if we haven't already read the
  // host's more-authoritative contents.xml file.
  assert(!host->has_contents_file());

  TiXmlElement *xhost = _xpackage->FirstChildElement("host");
  while (xhost != NULL) {
    const char *url = xhost->Attribute("url");
    if (url != NULL && host->get_host_url() == url) {
      // Found the entry for this particular host.
      host->read_xhost(xhost);
      return;
    }
    xhost = xhost->NextSiblingElement("host");
  }

  // Didn't find an entry for this host; oh well.
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_start_dir_suffix
//       Access: Public
//  Description: Determines the local path to the appropriate start
//               directory for this instance, within the generic
//               "start" directory.  Returns empty string if this
//               instance doesn't specify a custom start directory.
//
//               If this is nonempty, it will begin with a slash--the
//               intention is to append this to the end of the generic
//               start_dir path.
////////////////////////////////////////////////////////////////////
string P3DInstance::
get_start_dir_suffix() const {
  string start_dir_suffix;

  string start_dir = get_fparams().lookup_token("start_dir");
  if (start_dir.empty()) {
    start_dir = _start_dir;

    if (!start_dir.empty()) {
      // If the start_dir is taken from the p3d file (and not from the
      // HTML tokens), then we also append the alt_host name to the
      // start_dir, so that each alt_host variant will run in a
      // different directory.
      string alt_host = get_fparams().lookup_token("alt_host");
      if (!alt_host.empty()) {
        start_dir += "_";
        start_dir += alt_host;
      }
    }
  }
  if (!start_dir.empty()) {
    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    inst_mgr->append_safe_dir(start_dir_suffix, start_dir);
  }

  return start_dir_suffix;
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
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "pyobj");
  xcommand->SetAttribute("op", "set_browser_script_object");
  if (_dom_object != NULL) {
    xcommand->LinkEndChild(_session->p3dobj_to_xml(_dom_object));
  }
  
  doc->LinkEndChild(xcommand);
  
  _session->send_command(doc);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::make_p3d_request
//       Access: Private
//  Description: Creates a new P3D_request structure from the XML.
//               Returns NULL if no request is needed.
////////////////////////////////////////////////////////////////////
P3D_request *P3DInstance::
make_p3d_request(TiXmlElement *xrequest) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_request *request = NULL;

  const char *rtype = xrequest->Attribute("rtype");
  if (rtype != NULL) {
    if (strcmp(rtype, "notify") == 0) {
      const char *message = xrequest->Attribute("message");
      if (message != NULL) {
        // A notify message from Python code.
        request = new P3D_request;
        request->_instance = NULL;
        request->_request_type = P3D_RT_notify;
        request->_request._notify._message = strdup(message);
        handle_notify_request(message);
      }

    } else if (strcmp(rtype, "script") == 0) {
      // We don't actually build a P3D_request for a script request;
      // we always just handle it immediately.
      const char *operation = xrequest->Attribute("operation");
      TiXmlElement *xobject = xrequest->FirstChildElement("object");
      const char *property_name = xrequest->Attribute("property_name");
      int needs_response = 0;
      xrequest->Attribute("needs_response", &needs_response);
      int unique_id = 0;
      xrequest->Attribute("unique_id", &unique_id);
        
      P3D_object *value = NULL;
      TiXmlElement *xvalue = xrequest->FirstChildElement("value");
      if (xvalue != NULL) {
        value = _session->xml_to_p3dobj(xvalue);
      }
      if (value == NULL) {
        value = inst_mgr->new_none_object();
      }

      if (operation != NULL && xobject != NULL) {
        P3D_object *object = _session->xml_to_p3dobj(xobject);

        if (property_name == NULL) {
          property_name = "";
        }

        handle_script_request(operation, object, property_name, value,
                              (needs_response != 0), unique_id);
      }

      P3D_OBJECT_DECREF(value);

    } else if (strcmp(rtype, "drop_p3dobj") == 0) {
      int object_id;
      if (xrequest->QueryIntAttribute("object_id", &object_id) == TIXML_SUCCESS) {
        // We no longer need to keep this reference.
        _session->drop_p3dobj(object_id);
      }
    
    } else if (strcmp(rtype, "stop") == 0) {
      // A stop request from Python code.  This is kind of weird, but OK.
      request = new P3D_request;
      request->_instance = NULL;
      request->_request_type = P3D_RT_stop;

    } else if (strcmp(rtype, "forget_package") == 0) {
      const char *host_url = xrequest->Attribute("host_url");
      if (host_url != NULL) {
        // A Python-level request to remove a package from the cache.
        request = new P3D_request;
        request->_instance = NULL;
        request->_request_type = P3D_RT_forget_package;
        request->_request._forget_package._host_url = strdup(host_url);
        request->_request._forget_package._package_name = NULL;
        request->_request._forget_package._package_version = NULL;

        const char *package_name = xrequest->Attribute("package_name");
        const char *package_version = xrequest->Attribute("package_version");
        if (package_name != NULL) {
          request->_request._forget_package._package_name = strdup(package_name);
          if (package_version != NULL) {
            request->_request._forget_package._package_version = strdup(package_version);
          }
        }
      }

    } else {
      nout << "Ignoring request of type " << rtype << "\n";
    }
  }

  if (request != NULL) {
    assert(request->_instance == NULL);
    request->_instance = this;
    ref();
  }
  return request;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::handle_notify_request
//       Access: Private
//  Description: Called (in the main thread) when a notify request is
//               received from the subprocess.
////////////////////////////////////////////////////////////////////
void P3DInstance::
handle_notify_request(const string &message) {
  // We look for certain notify events that have particular meaning
  // to this instance.
  if (message == "onpythonload") {
    // Once Python is up and running, we can get the actual main
    // object from the Python side, and merge it with our own.

    TiXmlDocument *doc = new TiXmlDocument;
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "pyobj");
    xcommand->SetAttribute("op", "get_panda_script_object");
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

    if (result != NULL) {
      if (_matches_script_origin) {
        // We only actually merge the objects if this web page is
        // allowed to call our scripting functions.
        _main_object->set_pyobj(result);
      } else {
        // Otherwise, we just do a one-time application of the
        // toplevel properties down to Python.
        _main_object->apply_properties(result);
      }
      P3D_OBJECT_DECREF(result);
    }

    _main_object->set_string_property("status", "starting");

  } else if (message == "onwindowopen") {
    // The process told us that it just successfully opened its
    // window, for the first time.  Hide the splash window.
    _instance_window_opened = true;
    if (_splash_window != NULL) {
      _splash_window->set_visible(false);
    }

    // Guess we won't be using (most of) these images any more.
    for (int i = 0; i < (int)IT_num_image_types; ++i) {
      if (i != IT_active) {
        _image_files[i].cleanup();
      }
    }

    _main_object->set_string_property("status", "open");

  } else if (message == "onwindowattach") {
    // The graphics window has been attached to the browser frame
    // (maybe initially, maybe later).  Hide the splash window.

    // We don't actually hide the splash window immediately on OSX,
    // because on that platform, we can hide it as soon as we render
    // the first frame, avoiding an empty frame in that little period
    // of time between the window opening and the first frame being
    // drawn.
    _instance_window_attached = true;
#ifndef __APPLE__
    if (_splash_window != NULL) {
      _splash_window->set_visible(false);
    }
#endif  // __APPLE__

#ifdef __APPLE__
    // Start a timer to update the frame repeatedly.  This seems to be
    // steadier than waiting for nullEvent.
    if (_frame_timer == NULL) {
      CFRunLoopTimerContext timer_context;
      memset(&timer_context, 0, sizeof(timer_context));
      timer_context.info = this;
      _frame_timer = CFRunLoopTimerCreate
        (NULL, 0, 1.0 / 60.0, 0, 0, timer_callback, &timer_context);
      CFRunLoopRef run_loop = CFRunLoopGetCurrent();
      CFRunLoopAddTimer(run_loop, _frame_timer, kCFRunLoopCommonModes);
    }
#endif  // __APPLE__

  } else if (message == "onwindowdetach") {
    // The graphics window has been removed from the browser frame.
    // Restore the splash window.
    _instance_window_opened = true;
    _instance_window_attached = false;
    set_background_image(IT_active);
    if (_splash_window != NULL) {
      _splash_window->set_visible(true);
    }

#ifdef __APPLE__
    // Stop the frame timer; we don't need it any more.
    if (_frame_timer != NULL) {
      CFRunLoopTimerInvalidate(_frame_timer);
      CFRelease(_frame_timer);
      _frame_timer = NULL;
    }
#endif  // __APPLE__

  } else if (message == "buttonclick") {
    // We just got a special "button click" message from the
    // sub-thread.  This case is a little unusual, as it came from the
    // splash window and not from Python (we presumably haven't even
    // started Python yet).  We use this as a sneaky way to forward
    // the event from the sub-thread to the main thread.
    splash_button_clicked_main_thread();

  } else if (message == "authfinished") {
    // Similarly for the "auth finished" message.
    auth_finished_main_thread();

  } else if (message == "keyboardfocus") {
    if (_splash_window != NULL) {
      _splash_window->request_keyboard_focus();
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
handle_script_request(const string &operation, P3D_object *object, 
                      const string &property_name, P3D_object *value,
                      bool needs_response, int unique_id) {

  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "script_response");
  xcommand->SetAttribute("unique_id", unique_id);
  
  doc->LinkEndChild(xcommand);

  if (operation == "get_property") {
    P3D_object *result = P3D_OBJECT_GET_PROPERTY(object, property_name.c_str());

    // We've got the property value; feed it back down to the
    // subprocess.
    
    if (result != NULL) {
      xcommand->LinkEndChild(_session->p3dobj_to_xml(result));
      P3D_OBJECT_DECREF(result);
    }

  } else if (operation == "set_property") {
    bool result = 
      P3D_OBJECT_SET_PROPERTY(object, property_name.c_str(), true, value);
    
    TiXmlElement *xvalue = new TiXmlElement("value");
    xvalue->SetAttribute("type", "bool");
    xvalue->SetAttribute("value", (int)result);
    xcommand->LinkEndChild(xvalue);

  } else if (operation == "del_property") {
    bool result = P3D_OBJECT_SET_PROPERTY(object, property_name.c_str(), true, NULL);
    
    TiXmlElement *xvalue = new TiXmlElement("value");
    xvalue->SetAttribute("type", "bool");
    xvalue->SetAttribute("value", (int)result);
    xcommand->LinkEndChild(xvalue);

  } else if (operation == "has_method") {
    bool result = P3D_OBJECT_HAS_METHOD(object, property_name.c_str());
    
    TiXmlElement *xvalue = new TiXmlElement("value");
    xvalue->SetAttribute("type", "bool");
    xvalue->SetAttribute("value", (int)result);
    xcommand->LinkEndChild(xvalue);

  } else if (operation == "call") {
    // Convert the single value parameter into an array of parameters.
    P3D_object **values = &value;
    int num_values = 1;
    if (value->_class == &P3DObject::_object_class) {
      P3DObject *p3dobj = (P3DObject *)value;
      if (p3dobj->get_object_array_size() != -1) {
        values = p3dobj->get_object_array();
        num_values = p3dobj->get_object_array_size();
      }
    }

    P3D_object *result =
      P3D_OBJECT_CALL(object, property_name.c_str(), needs_response,
                      values, num_values);
    
    if (result != NULL) {
      xcommand->LinkEndChild(_session->p3dobj_to_xml(result));
      P3D_OBJECT_DECREF(result);
    }

  } else if (operation == "eval") {
    P3D_object *result;
    int size = P3D_OBJECT_GET_STRING(value, NULL, 0);
    char *buffer = new char[size + 1];
    P3D_OBJECT_GET_STRING(value, buffer, size + 1);
    result = P3D_OBJECT_EVAL(object, buffer);
    delete[] buffer;
    
    if (result != NULL) {
      xcommand->LinkEndChild(_session->p3dobj_to_xml(result));
      P3D_OBJECT_DECREF(result);
    }
  }

  if (needs_response) {
    _session->send_command(doc);
  } else {
    delete doc;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_failed
//       Access: Private
//  Description: Sets the "failed" indication to display sadness to
//               the user--we're unable to launch the instance for
//               some reason.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_failed() {
  set_button_image(IT_none);
  set_background_image(IT_failed);
  _main_object->set_string_property("status", "failed");

  if (!_failed) {
    _failed = true;
    make_splash_window();
    send_notify("onfail");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::make_splash_window
//       Access: Private
//  Description: Creates the splash window to be displayed at startup,
//               if it's time.
////////////////////////////////////////////////////////////////////
void P3DInstance::
make_splash_window() {
  // Should we make the splash window visible?
  bool make_visible = true;
  if (_instance_window_opened) {
    // Not once we've opened the main window.
    make_visible = false;
  }

  if (_wparams.get_window_type() != P3D_WT_embedded && 
      !_stuff_to_download && _auto_install &&
      (_auto_start || _stop_on_ready) && _p3d_trusted) {
    // If it's a toplevel or fullscreen window, then we don't want a
    // splash window unless we have stuff to download, or a button to
    // display.
    make_visible = false;
  }

  if (is_failed()) {
    // But, if we've failed to launch somehow, we need to let the user
    // know.
    make_visible = true;
  }

  if (_splash_window != NULL) {
    // Already got one.
    _splash_window->set_visible(make_visible);
    return;
  }
  if (!_got_wparams) {
    // Don't know where to put it yet.
    return;
  }
  if (_wparams.get_window_type() == P3D_WT_hidden && !is_failed()) {
    // We're hidden, and so is the splash window.  (But if we've got a
    // failure case to report, we don't care and create the splash
    // window anyway.)
    return;
  }

  _splash_window = new SplashWindowType(this, make_visible);

  // Get the splash window colors.  We must set these *before* we call
  // set_wparams.
  if (_fparams.has_token("fgcolor")) {
    int r, g, b;
    if (parse_color(r, g, b, _fparams.lookup_token("fgcolor"))) {
      _splash_window->set_fgcolor(r, g, b);
    } else {
      nout << "parse failure on fgcolor " << _fparams.lookup_token("fgcolor") << "\n";
    }
  }
  if (_fparams.has_token("bgcolor")) {
    int r, g, b;
    if (parse_color(r, g, b, _fparams.lookup_token("bgcolor"))) {
      _splash_window->set_bgcolor(r, g, b);
    } else {
      nout << "parse failure on bgcolor " << _fparams.lookup_token("bgcolor") << "\n";
    }
  }
  if (_fparams.has_token("barcolor")) {
    int r, g, b;
    if (parse_color(r, g, b, _fparams.lookup_token("barcolor"))) {
      _splash_window->set_barcolor(r, g, b);
    } else {
      nout << "parse failure on barcolor " << _fparams.lookup_token("barcolor") << "\n";
    }
  }

  _splash_window->set_wparams(_wparams);
  _splash_window->set_install_label(_install_label);
  
    
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  // Go get the required images.
  for (int i = 0; i < (int)IT_none; ++i) {
    string token_keyword = string(_image_type_names[i]) + "_img";
    if (!_fparams.has_token(token_keyword) && i < (int)IT_auth_ready) {
      token_keyword = "splash_img";
    }

    string image_url = _fparams.lookup_token(token_keyword);
    if (image_url.empty()) {
      // No specific image for this type is specified; get the default
      // image.  We do this via the P3DPackage interface, so we can
      // use the cached version on disk if it's good.
      _image_files[i]._use_standard_image = true;
      
    } else {
      // We have an explicit image specified for this slot, so just
      // download it directly.  This one won't be cached locally
      // (though the browser might be free to cache it).
      _image_files[i]._use_standard_image = false;
      _image_files[i]._filename.clear();
      
      // Make a temporary file to receive the splash image.
      assert(_image_files[i]._temp_filename == NULL);
      _image_files[i]._temp_filename = new P3DTemporaryFile(".jpg");
      
      // Start downloading the requested image.
      ImageDownload *download = new ImageDownload(this, i);
      download->set_url(image_url);
      download->set_filename(_image_files[i]._temp_filename->get_filename());
      
      start_download(download);
    }
  }

  if (_current_background_image != IT_none) {
    _splash_window->set_image_filename(_image_files[_current_background_image]._filename, P3DSplashWindow::IP_background);
  }

  if (_current_button_image != IT_none) {
    _splash_window->set_image_filename(_image_files[_current_button_image]._filename, P3DSplashWindow::IP_button_ready);
    _splash_window->set_image_filename(_image_files[_current_button_image + 1]._filename, P3DSplashWindow::IP_button_rollover);
    _splash_window->set_image_filename(_image_files[_current_button_image + 2]._filename, P3DSplashWindow::IP_button_click); 
    _splash_window->set_button_active(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_background_image
//       Access: Private
//  Description: Specifies the particular image that should be
//               displayed as the background image in the splash
//               window.  Specify IT_none to take the background image
//               away.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_background_image(ImageType image_type) {
  if (is_failed()) {
    // Can't change the background again after we've failed.
    return;
  }

  if (image_type != _current_background_image) {
    nout << "setting background to " << _image_type_names[image_type]
         << ", splash_window = " << _splash_window << "\n";
    // Remove the previous image.
    _image_files[_current_background_image]._image_placement = P3DSplashWindow::IP_none;

    // Install the new image.
    _current_background_image = image_type;
    if (_current_background_image != IT_none) {
      _image_files[_current_background_image]._image_placement = P3DSplashWindow::IP_background;
    }

    // Update the splash window.
    if (_splash_window != NULL) {
      _splash_window->set_image_filename(_image_files[_current_background_image]._filename, P3DSplashWindow::IP_background);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_button_image
//       Access: Private
//  Description: Specifies the particular image that should be
//               displayed as the button image in the splash
//               window.  Specify IT_none to take the button image
//               away.
//
//               This actually defines a trilogy of button images:
//               ready, rollover, click.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_button_image(ImageType image_type) {
  if (is_failed()) {
    // Can't set the button again after we've failed.
    return;
  }

  if (image_type != _current_button_image) {
    nout << "setting button to " << _image_type_names[image_type] << "\n";
    // Remove the previous image.
    _image_files[_current_button_image]._image_placement = P3DSplashWindow::IP_none;
    if (_current_button_image != IT_none) {
      _image_files[_current_button_image + 1]._image_placement = P3DSplashWindow::IP_none;
      _image_files[_current_button_image + 2]._image_placement = P3DSplashWindow::IP_none;
    }

    // Install the new image.
    _current_button_image = image_type;
    if (_current_button_image != IT_none) {
      _image_files[_current_button_image]._image_placement = P3DSplashWindow::IP_button_ready;
      _image_files[_current_button_image + 1]._image_placement = P3DSplashWindow::IP_button_rollover;
      _image_files[_current_button_image + 2]._image_placement = P3DSplashWindow::IP_button_click;
    }

    // Update the splash window.
    if (_splash_window != NULL) {
      if (_current_button_image != IT_none) {
        _splash_window->set_image_filename(_image_files[_current_button_image]._filename, P3DSplashWindow::IP_button_ready);
        _splash_window->set_image_filename(_image_files[_current_button_image + 1]._filename, P3DSplashWindow::IP_button_rollover);
        _splash_window->set_image_filename(_image_files[_current_button_image + 2]._filename, P3DSplashWindow::IP_button_click);
        _splash_window->set_button_active(true);
      } else {
        _splash_window->set_button_active(false);
      }
    }

  } else {
    // We're not changing the button graphic, but we might be
    // re-activating it.
    if (_splash_window != NULL) {
      if (_current_button_image != IT_none) {
        _splash_window->set_button_active(true);
      } else {
        _splash_window->set_button_active(false);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::report_package_info_ready
//       Access: Private
//  Description: Notified when a package information has been
//               successfully downloaded and the package is idle,
//               waiting for activate_download() to be called.
////////////////////////////////////////////////////////////////////
void P3DInstance::
report_package_info_ready(P3DPackage *package) {
  nout << "report_package_info_ready: " << package->get_package_name() << "\n";
  if (package == _image_package || package == _certlist_package ||
      package == _p3dcert_package) {
    // A special case: these packages get immediately downloaded,
    // without waiting for anything else.
    if (package == _certlist_package || package == _p3dcert_package) {
      // If we're downloading one of the two cert packages, though,
      // put up a progress bar.
      make_splash_window();
      if (_splash_window != NULL) {
        _splash_window->set_install_progress(0.0, true, 0);
      }
      if (package == _certlist_package) {
        set_install_label("Getting Certificates");
      } else {          
        set_install_label("Getting Authorization Dialog");
      }
    }

    package->activate_download();
    return;
  }

  if (package == _panda3d_package && !_packages_specified) {
    // Another special case.  Once the special panda3d package is
    // ready to download (and we know what platform it belongs to), we
    // can begin to download the remaining required packages.
    string package_platform = package->get_package_platform();
    if (!package_platform.empty() && _session_platform.empty()) {
      // From now on, all platform-specific files downloaded by this
      // session will be for this platform.
      _session_platform = package_platform;
    }
    if (_session_platform != package_platform) {
      nout << "Error: session is " << _session_platform
           << ", but we somehow got panda3d for " << package_platform << "\n";
      set_failed();
      return;
    }

    add_packages();
  }

  // Now that the package's info is ready, we know its set of required
  // packages, and we can add these to the list.
  P3DPackage::Requires::const_iterator ri;
  for (ri = package->_requires.begin(); ri != package->_requires.end(); ++ri) {
    const P3DPackage::RequiredPackage &rp = (*ri);
    add_package(rp._package_name, rp._package_version, rp._package_seq, 
                rp._host);
  }

  consider_start_download();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::consider_start_download
//       Access: Private
//  Description: When all package info files have been obtained,
//               begins downloading stuff.
////////////////////////////////////////////////////////////////////
void P3DInstance::
consider_start_download() {
  if (get_packages_info_ready()) {
    // All packages are ready to go.  Let's start some download
    // action.
    _downloading_packages.clear();
    _prev_downloaded = 0;
    _total_download_size = 0;
    Packages::const_iterator pi;
    for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
      P3DPackage *package = (*pi);
      if (package->get_info_ready()) {
        if (!package->get_ready()) {
          _downloading_packages.push_back(package);
          _total_download_size += package->get_download_size();
        } else {
          _prev_downloaded += package->get_download_size();
        }
      }
    }
    ready_to_install();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::ready_to_install
//       Access: Private
//  Description: Called when it's time to start the package download
//               process.
////////////////////////////////////////////////////////////////////
void P3DInstance::
ready_to_install() {
  if (_downloading_packages.empty() && _download_complete) {
    // We have already been here.  Ignore it.
    
  } else if (!_auto_install && !_download_started) {
    // Not authorized to download yet.  We're waiting for the user
    // to acknowledge the download.
    set_background_image(IT_ready);
    set_button_image(IT_play_ready);
    
  } else {
    _download_started = true;
    _download_complete = false;
    _download_package_index = 0;
    _total_downloaded = 0;

    // Record the time we started the package download, so we can
    // report downloadElapsedTime and predict downloadRemainingTime.
#ifdef _WIN32
    _start_dl_tick = GetTickCount();
#else
    gettimeofday(&_start_dl_timeval, NULL);
#endif

    nout << "Beginning install of " << _downloading_packages.size()
         << " packages, total " << _total_download_size
         << " bytes required (" << _prev_downloaded
         << " previously downloaded).\n";
    
    if (_downloading_packages.size() > 0) {
      _stuff_to_download = true;
      
      // Maybe it's time to open a splash window now.
      make_splash_window();
    }
    
    _main_object->set_string_property("status", "downloading");
    _main_object->set_int_property("numDownloadingPackages", _downloading_packages.size());
    _main_object->set_int_property("totalDownloadSize", _total_download_size);
    _main_object->set_int_property("downloadElapsedSeconds", 0);
    _main_object->set_undefined_property("downloadRemainingSeconds");

    double progress = 0.0;
    if (_prev_downloaded != 0) {
      // We might start off with more than 0 progress, if we've
      // already downloaded some of it previously.
      progress = (_prev_downloaded) / (_total_download_size + _prev_downloaded);
      progress = min(progress, 1.0);

      _main_object->set_float_property("downloadProgress", progress);
    }
    if (_splash_window != NULL) {
      _splash_window->set_install_progress(progress, true, 0);
    }

    send_notify("ondownloadbegin");
    
    start_next_download();
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::start_next_download
//       Access: Private
//  Description: Checks whether all packages are ready and waiting to
//               be downloaded; if so, starts the next package in
//               sequence downloading.
////////////////////////////////////////////////////////////////////
void P3DInstance::
start_next_download() {
  while (_download_package_index < (int)_downloading_packages.size()) {
    P3DPackage *package = _downloading_packages[_download_package_index];
    if (package->get_failed()) {
      send_notify("ondownloadfail");
      set_failed();
      return;
    }

    if (!package->get_ready()) {
      // This package is ready to download.  Begin.
      string name = package->get_formatted_name();
      _main_object->set_string_property("downloadPackageName", package->get_package_name());
      _main_object->set_string_property("downloadPackageDisplayName", name);
      _main_object->set_int_property("downloadPackageNumber", _download_package_index + 1);
      _main_object->set_int_property("downloadPackageSize", package->get_download_size());
      set_install_label("Installing " + name);

      nout << "Installing " << package->get_package_name()
           << ", package " << _download_package_index + 1
           << " of " << _downloading_packages.size()
           << ", " << package->get_download_size()
           << " bytes.\n";

      package->activate_download();
      send_notify("ondownloadnext");
      return;
    }
    
    // This package has been downloaded.  Move to the next.
    _total_downloaded += package->get_download_size();
    ++_download_package_index;
  }

  // Looks like we're all done downloading.  Launch!
  _downloading_packages.clear();

  if (get_packages_ready()) {
    mark_download_complete();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::mark_download_complete
//       Access: Private
//  Description: Called internally when all files needed to launch
//               have been downloaded.
////////////////////////////////////////////////////////////////////
void P3DInstance::
mark_download_complete() {
  if (_failed) {
    return;
  }

  if (!_download_complete) {
    _download_complete = true;
    _main_object->set_bool_property("downloadComplete", true);
    _main_object->set_string_property("status", "downloadcomplete");
    send_notify("ondownloadcomplete");
  }
  
  // Take down the download progress bar.
  if (_splash_window != NULL) {
    _splash_window->set_install_progress(0.0, true, 0);
  }
  set_install_label("");

  if (_got_wparams && _p3d_trusted) {
    ready_to_start();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::ready_to_start
//       Access: Private
//  Description: Called internally when we have got the wparams and
//               fparams and we have downloaded all required packages.
////////////////////////////////////////////////////////////////////
void P3DInstance::
ready_to_start() {
  if (_instance_started || is_failed()) {
    // Already started--or never mind.
    return;
  }

  _main_object->set_string_property("status", "ready");
  send_notify("onready");

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (_stop_on_ready) {
    // If we've got the "stop_on_ready" token, then exit abruptly
    // now, instead of displaying the splash window.
    request_stop_main_thread();
    return;
  }

  if (_auto_start) {
    set_background_image(IT_launch);
    inst_mgr->start_instance(this);

  } else {
    // We're fully downloaded, and waiting for the user to click play.
    set_background_image(IT_ready);
    set_button_image(IT_play_ready);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::report_instance_progress
//       Access: Private
//  Description: Notified as the instance file is downloaded.
////////////////////////////////////////////////////////////////////
void P3DInstance::
report_instance_progress(double progress, bool is_progress_known,
                         size_t received_data) {
  if (!_show_dl_instance_progress) {
    // If we haven't yet set the download label, set it after a full
    // second has elapsed.  We don't want to set it too soon, because
    // we're not really sure how long it will take to download (the
    // instance file might be already in the browser cache).
#ifdef _WIN32
    int now = GetTickCount();
    double elapsed = (double)(now - _start_dl_tick) * 0.001;
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    double elapsed = (double)(now.tv_sec - _start_dl_timeval.tv_sec) +
      (double)(now.tv_usec - _start_dl_timeval.tv_usec) / 1000000.0;
#endif

    // Put up the progress bar after 2 seconds have elapsed, if we've
    // still got some distance to go; or after 5 seconds have elapsed
    // regardless.
    if ((elapsed > 2.0 && progress < 0.7) ||
        (elapsed > 5.0)) {
      _show_dl_instance_progress = true;
      if (_fparams.has_token("p3d_install_label")) {
        set_install_label(_fparams.lookup_token("p3d_install_label"));
      } else {
        set_install_label("Getting " + _p3d_basename);
      }
    }
  }

  if (_splash_window != NULL && _show_dl_instance_progress) {
    _splash_window->set_install_progress(progress, is_progress_known, received_data);
  }
  _main_object->set_float_property("instanceDownloadProgress", progress);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::report_package_progress
//       Access: Private
//  Description: Notified as the packages required by the instance
//               file are downloaded.
////////////////////////////////////////////////////////////////////
void P3DInstance::
report_package_progress(P3DPackage *package, double progress) {
  if (package == _image_package) {
    // Ignore this.
    return;
  }
  if (package == _certlist_package || package == _p3dcert_package) {
    // This gets its own progress bar.
    if (_splash_window != NULL) {
      _splash_window->set_install_progress(progress, true, 0);
    }
    return;
  }

  if (_download_package_index >= (int)_downloading_packages.size() ||
      package != _downloading_packages[_download_package_index]) {
    // Quietly ignore a download progress report from an unexpected
    // package.
    return;
  }

  // Scale the progress into the range appropriate to this package.
  progress = (progress * package->get_download_size() + _total_downloaded + _prev_downloaded) / (_total_download_size + _prev_downloaded);
  progress = min(progress, 1.0);

  if (_splash_window != NULL) {
    _splash_window->set_install_progress(progress, true, 0);
  }
  _main_object->set_float_property("downloadProgress", progress);

  static const size_t buffer_size = 256;
  char buffer[buffer_size];

  // Get the floating-point elapsed time.
#ifdef _WIN32
  int now = GetTickCount();
  double elapsed = (double)(now - _start_dl_tick) * 0.001;
#else
  struct timeval now;
  gettimeofday(&now, NULL);
  double elapsed = (double)(now.tv_sec - _start_dl_timeval.tv_sec) +
    (double)(now.tv_usec - _start_dl_timeval.tv_usec) / 1000000.0;
#endif

  int ielapsed = (int)elapsed;
  _main_object->set_int_property("downloadElapsedSeconds", ielapsed);

  sprintf(buffer, "%d:%02d", ielapsed / 60, ielapsed % 60);
  _main_object->set_string_property("downloadElapsedFormatted", buffer);

  if (progress > 0 && (elapsed > 5.0 || progress > 0.2)) {
    double this_total = elapsed / progress;
    double this_remaining = max(this_total - elapsed, 0.0);

    // Age out any old time reports.
    double old = elapsed - min(time_average, this_remaining);
    while (!_time_reports.empty() && _time_reports.front()._report_time < old) {
      TimeReport &tr0 = _time_reports.front();
      _total_time_reports -= tr0._total;
      _time_reports.pop_front();
    }
    if (_time_reports.empty()) {
      _total_time_reports = 0.0;
    }

    // Add a new time report.
    TimeReport tr;
    tr._total = this_total;
    tr._report_time = elapsed;
    _time_reports.push_back(tr);
    _total_time_reports += tr._total;
      
    // Now get the average report.
    if (!_time_reports.empty()) {
      double total = _total_time_reports / (double)_time_reports.size();
      double remaining = max(total - elapsed, 0.0);
      int iremaining = (int)(remaining + 0.5);
      _main_object->set_int_property("downloadRemainingSeconds", iremaining);
      sprintf(buffer, "%d:%02d", iremaining / 60, iremaining % 60);
      _main_object->set_string_property("downloadRemainingFormatted", buffer);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::report_package_done
//       Access: Private
//  Description: Notified when a required package is fully downloaded,
//               or failed.
////////////////////////////////////////////////////////////////////
void P3DInstance::
report_package_done(P3DPackage *package, bool success) {
  nout << "Done installing " << package->get_package_name()
       << ": success = " << success << "\n";

  if (package == _image_package) {
    // A special case: we just downloaded the image package, so get
    // the image files out of it and point them to the splash window.
    string package_dir = package->get_package_dir();
    const TiXmlElement *xconfig = package->get_xconfig();
    if (xconfig == NULL) {
      nout << "No <config> entry in image package\n";
      return;
    }
    package->mark_used();

    for (int i = 0; i < (int)IT_none; ++i) {
      if (_image_files[i]._use_standard_image) {
        // This image indexes into the package.  Go get the standard
        // image filename.
        string token = string(_image_type_names[i]) + "_img";
        const string *basename = xconfig->Attribute(token);
        if (basename == NULL) {
          nout << "No entry in image package for " << token << "\n";
        } else {
          string image_filename = package_dir + "/" + *basename;
          _image_files[i]._filename = image_filename;
          
          // If the image should be on the window now, and the window
          // still exists, put it up.
          if (_splash_window != NULL &&
              _image_files[i]._image_placement != P3DSplashWindow::IP_none) {
            P3DSplashWindow::ImagePlacement image_placement = _image_files[i]._image_placement;
            _splash_window->set_image_filename(image_filename, image_placement);
          }
        }
      }
    }
    return;
  }

  if (package == _certlist_package) {
    // Another special case: successfully downloading certlist (or
    // failing to download it) means we can finish checking the
    // authenticity of the p3d file.

    package->mark_used();

    // Take down the download progress.
    if (_splash_window != NULL) {
      _splash_window->set_install_progress(0.0, true, 0);
    }
    set_install_label("");

    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    inst_mgr->read_certlist(package);
    check_p3d_signature();
    return;
  }

  if (package == _p3dcert_package) {
    // Another special case: successfully downloading p3dcert means we
    // can enable the auth button.

    package->mark_used();

    // Take down the download progress.
    if (_splash_window != NULL) {
      _splash_window->set_install_progress(0.0, true, 0);
    }
    set_install_label("");
    mark_p3d_untrusted();
    return;
  }

  if (success) {
    report_package_progress(package, 1.0);
    start_next_download();
  } else {
    send_notify("ondownloadfail");
    set_failed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_install_label
//       Access: Private
//  Description: Sets the install label that will be displayed on the
//               splash window, if it is present.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_install_label(const string &install_label) {
  _install_label = install_label;
  if (_splash_window != NULL) {
    _splash_window->set_install_label(_install_label);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::paint_window
//       Access: Private
//  Description: Actually paints the rendered image to the browser
//               window.  This is only needed for OSX, where the child
//               process isn't allowed to do it directly.
////////////////////////////////////////////////////////////////////
void P3DInstance::
paint_window() {
#ifdef __APPLE__
  const P3D_window_handle &handle = _wparams.get_parent_window();
  if (handle._window_handle_type == P3D_WHT_osx_port) {
#if !__LP64__
    paint_window_osx_port();
#endif

  } else if (handle._window_handle_type == P3D_WHT_osx_cgcontext) {
    const P3D_window_handle &handle = _wparams.get_parent_window();
    assert(handle._window_handle_type == P3D_WHT_osx_cgcontext);
    CGContextRef context = handle._handle._osx_cgcontext._context;

    paint_window_osx_cgcontext(context);
  }
#endif  // __APPLE__
}

#if defined(__APPLE__) && !__LP64__
////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_framebuffer_osx_port
//       Access: Private
//  Description: Fills _reversed_buffer with the pixels from the
//               current frame, suitable for rendering via the old
//               QuickDraw interface.  Returns true on success, or
//               false if there is no Panda3D window visible.  Only
//               needed on OSX.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
get_framebuffer_osx_port() {
  if (_swbuffer == NULL || !_instance_window_attached) {
    // We don't have a Panda3D window yet.
    return false;
  }

  // blit rendered framebuffer into window backing store
  int x_size = min(_wparams.get_win_width(), _swbuffer->get_x_size());
  int y_size = min(_wparams.get_win_height(), _swbuffer->get_y_size());
  size_t rowsize = _swbuffer->get_row_size();

  if (_swbuffer->ready_for_read()) {
    // Copy the new framebuffer image from the child process.
    const void *framebuffer = _swbuffer->open_read_framebuffer();
    
    // We have to reverse the image vertically first (different
    // conventions between Panda and Mac).
    for (int yi = 0; yi < y_size; ++yi) {
#ifndef __BIG_ENDIAN__
      // On a little-endian machine, we only have to reverse the order
      // of the rows.
      memcpy(_reversed_buffer + (y_size - 1 - yi) * rowsize,
             (char *)framebuffer + yi * rowsize,
             rowsize);

#else  // __BIG_ENDIAN__
      // On a big-endian machine, we need to do more work.

      // It appears that kBGRAPixelFormat, below, is ignored on
      // big-endian machines, and it is treated as KARGBPixelFormat
      // regardless of what we specify.  Vexing.  To compensate for
      // this, we have to reverse the color channels ourselves on
      // big-endian machines.

      const char *source = (const char *)framebuffer + yi * rowsize;
      const char *stop = source + x_size * 4;
      char *dest = (_reversed_buffer + (y_size - 1 - yi) * rowsize);
      while (source < stop) {
        char b = source[0];
        char g = source[1];
        char r = source[2];
        char a = source[3];
        dest[0] = a;
        dest[1] = r;
        dest[2] = g;
        dest[3] = b;
        dest += 4;
        source += 4;
      }
#endif
    }
    
    _swbuffer->close_read_framebuffer();

    if (_splash_window != NULL && _splash_window->get_visible()) {
      // If the splash window is up, time to hide it.  We've just
      // rendered a real frame.
      _splash_window->set_visible(false);
    }

  } else {
    // No frame ready.  Just re-paint the frame we had saved last
    // time.
  }

  return true;
}
#endif  // __APPLE__

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_framebuffer_osx_cgcontext
//       Access: Private
//  Description: Fills _reversed_buffer with the pixels from the
//               current frame, suitable for rendering via the new
//               CoreGraphics interface.  Returns true on success, or
//               false if there is no Panda3D window visible.  Only
//               needed on OSX.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
get_framebuffer_osx_cgcontext() {
  if (_swbuffer == NULL || !_instance_window_attached) {
    // We don't have a Panda3D window yet.
    return false;
  }

  // blit rendered framebuffer into window backing store
  int x_size = min(_wparams.get_win_width(), _swbuffer->get_x_size());
  int y_size = min(_wparams.get_win_height(), _swbuffer->get_y_size());
  size_t rowsize = _swbuffer->get_row_size();

  if (_swbuffer->ready_for_read()) {
    // Copy the new framebuffer image from the child process.
    const void *framebuffer = _swbuffer->open_read_framebuffer();
    memcpy(_reversed_buffer, framebuffer, y_size * rowsize);
    _swbuffer->close_read_framebuffer();

    if (_splash_window != NULL && _splash_window->get_visible()) {
      // If the splash window is up, time to hide it.  We've just
      // rendered a real frame.
      _splash_window->set_visible(false);
    }

  } else {
    // No frame ready.  Just re-paint the frame we had saved last
    // time.
  }

  return true;
}
#endif  // __APPLE__

#if defined(__APPLE__) && !__LP64__
////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::paint_window_osx_port
//       Access: Private
//  Description: Actually paints the rendered image to the browser
//               window, using the OSX deprecated QuickDraw
//               interfaces.
////////////////////////////////////////////////////////////////////
void P3DInstance::
paint_window_osx_port() {
  if (!get_framebuffer_osx_port()) {
    // No Panda3D window is showing.
    return;
  }

  int x_size = min(_wparams.get_win_width(), _swbuffer->get_x_size());
  int y_size = min(_wparams.get_win_height(), _swbuffer->get_y_size());
  size_t rowsize = _swbuffer->get_row_size();

  Rect src_rect = {0, 0, y_size, x_size};
  Rect ddrc_rect = {0, 0, y_size, x_size};

  QDErr err;

  GWorldPtr pGWorld;
  err = NewGWorldFromPtr(&pGWorld, k32BGRAPixelFormat, &src_rect, 0, 0, 0, 
                         _reversed_buffer, rowsize);
  if (err != noErr) {
    nout << " error in NewGWorldFromPtr, called from paint_window()\n";
    return;
  }

  const P3D_window_handle &handle = _wparams.get_parent_window();
  assert(handle._window_handle_type == P3D_WHT_osx_port);
  GrafPtr out_port = handle._handle._osx_port._port;
  GrafPtr port_save = NULL;
  Boolean port_changed = QDSwapPort(out_port, &port_save);

  // Make sure the clipping rectangle isn't in the way.  Is there a
  // better way to eliminate the cliprect from consideration?
  Rect r = { 0, 0, 0x7fff, 0x7fff }; 
  ClipRect(&r);

  CopyBits(GetPortBitMapForCopyBits(pGWorld), 
           GetPortBitMapForCopyBits(out_port), 
           &src_rect, &ddrc_rect, srcCopy, 0);
  
  if (port_changed) {
    QDSwapPort(port_save, NULL);
  }
  
  DisposeGWorld(pGWorld);
}
#endif  // __APPLE__  

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::paint_window_osx_cgcontext
//       Access: Private
//  Description: Actually paints the rendered image to the browser
//               window.  This is the newer CoreGraphics
//               implementation on OSX.
////////////////////////////////////////////////////////////////////
void P3DInstance::
paint_window_osx_cgcontext(CGContextRef context) {
  if (!get_framebuffer_osx_cgcontext()) {
    // No Panda3D window is showing.
    return;
  }

  int x_size = min(_wparams.get_win_width(), _swbuffer->get_x_size());
  int y_size = min(_wparams.get_win_height(), _swbuffer->get_y_size());

  if (_buffer_image != NULL) {
    CGRect region = { { 0, 0 }, { x_size, y_size } };
    CGContextDrawImage(context, region, _buffer_image);
  }
}
#endif  // __APPLE__

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::handle_event_osx_event_record
//       Access: Private
//  Description: Responds to the deprecated Carbon event types in Mac
//               OSX.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
handle_event_osx_event_record(const P3D_event_data &event) {
  bool retval = false;

#if defined(__APPLE__) && !__LP64__
  assert(event._event_type == P3D_ET_osx_event_record);
  EventRecord *er = event._event._osx_event_record._event;

  Point pt = er->where;

  // Need to ensure we have the correct port set, in order to
  // convert the mouse coordinates successfully via
  // GlobalToLocal().
  const P3D_window_handle &handle = _wparams.get_parent_window();
  if (handle._window_handle_type == P3D_WHT_osx_port) {
    GrafPtr out_port = handle._handle._osx_port._port;
    GrafPtr port_save = NULL;
    Boolean port_changed = QDSwapPort(out_port, &port_save);
    
    GlobalToLocal(&pt);
    
    if (port_changed) {
      QDSwapPort(port_save, NULL);
    }
  } else {
    // First, convert the coordinates from screen coordinates to
    // browser window coordinates.
    WindowRef window = handle._handle._osx_cgcontext._window;
    CGPoint cgpt = { pt.h, pt.v };
    HIPointConvert(&cgpt, kHICoordSpaceScreenPixel, NULL,
                   kHICoordSpaceWindow, window);
    
    // Then convert to plugin coordinates.
    pt.h = (short)(cgpt.x - _wparams.get_win_x());
    pt.v = (short)(cgpt.y - _wparams.get_win_y());
  }

  SubprocessWindowBuffer::Event swb_event;
  swb_event._source = SubprocessWindowBuffer::ES_none;
  swb_event._type = SubprocessWindowBuffer::ET_none;
  swb_event._code = 0;
  swb_event._flags = 0;
  add_carbon_modifier_flags(swb_event._flags, er->modifiers);

  bool trust_mouse_data = true;

  switch (er->what) {
  case mouseDown:
    swb_event._source = SubprocessWindowBuffer::ES_mouse;
    swb_event._type = SubprocessWindowBuffer::ET_button_down;
    retval = true;
    break;

  case mouseUp:
    swb_event._source = SubprocessWindowBuffer::ES_mouse;
    swb_event._type = SubprocessWindowBuffer::ET_button_up;
    retval = true;
    break;

  case keyDown:
  case keyUp:
  case autoKey:
    if (_swbuffer != NULL) {
      swb_event._source = SubprocessWindowBuffer::ES_keyboard;
      swb_event._code = er->message;
      if (er->what == keyUp) {
        swb_event._type = SubprocessWindowBuffer::ET_button_up;
      } else if (er->what == keyDown) {
        swb_event._type = SubprocessWindowBuffer::ET_button_down;
      } else {
        swb_event._type = SubprocessWindowBuffer::ET_button_again;
      }
      retval = true;
    }
    break;

  case updateEvt:
    paint_window();
    retval = true;
    break;

  case activateEvt:
    _mouse_active = ((er->modifiers & 1) != 0);
    break;

  case osEvt:
    // The mouse data sent with an "os event" seems to be in an
    // indeterminate space.
    trust_mouse_data = false;
    break;

  default:
    break;
  }

  if (_mouse_active) {
    swb_event._flags |= SubprocessWindowBuffer::EF_has_mouse;
    if (trust_mouse_data) {
      swb_event._x = pt.h;
      swb_event._y = pt.v;
      swb_event._flags |= SubprocessWindowBuffer::EF_mouse_position;
    }
  }

  if (_swbuffer != NULL) {
    _swbuffer->add_event(swb_event);
  }
#endif  // __APPLE__

  return retval;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::handle_event_osx_cocoa
//       Access: Private
//  Description: Responds to the new Cocoa event types in Mac
//               OSX.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
handle_event_osx_cocoa(const P3D_event_data &event) {
  bool retval = false;

#ifdef __APPLE__
  assert(event._event_type == P3D_ET_osx_cocoa);
  const P3DCocoaEvent &ce = event._event._osx_cocoa._event;

  SubprocessWindowBuffer::Event swb_event;
  swb_event._source = SubprocessWindowBuffer::ES_none;
  swb_event._type = SubprocessWindowBuffer::ET_none;
  swb_event._code = 0;
  swb_event._flags = 0;

  switch (ce.type) {
  case P3DCocoaEventDrawRect:
    {
      CGContextRef context = ce.data.draw.context;
      paint_window_osx_cgcontext(context);
      retval = true;
    }
    break;

  case P3DCocoaEventMouseDown:
    swb_event._source = SubprocessWindowBuffer::ES_mouse;
    swb_event._type = SubprocessWindowBuffer::ET_button_down;
    retval = true;
    break;

  case P3DCocoaEventMouseUp:
    swb_event._source = SubprocessWindowBuffer::ES_mouse;
    swb_event._type = SubprocessWindowBuffer::ET_button_up;
    retval = true;
    break;

  case P3DCocoaEventKeyDown:
    swb_event._source = SubprocessWindowBuffer::ES_keyboard;
    swb_event._code = ce.data.key.keyCode << 8;
    if (ce.data.key.isARepeat) {
      swb_event._type = SubprocessWindowBuffer::ET_button_again;
    } else {
      swb_event._type = SubprocessWindowBuffer::ET_button_down;
      if (ce.data.key.characters[0] > 0 & ce.data.key.characters[0] < 0x100) {
        swb_event._code |= ce.data.key.characters[0];
      }
    }
    _modifiers = ce.data.key.modifierFlags;
    retval = true;
    break;

  case P3DCocoaEventKeyUp:
    swb_event._source = SubprocessWindowBuffer::ES_keyboard;
    swb_event._type = SubprocessWindowBuffer::ET_button_up;
    swb_event._code = ce.data.key.keyCode << 8;
    _modifiers = ce.data.key.modifierFlags;
    retval = true;
    break;

  case P3DCocoaEventFlagsChanged:
    _modifiers = ce.data.key.modifierFlags;
    retval = true;
    break;

  case P3DCocoaEventFocusChanged:
    _mouse_active = (ce.data.focus.hasFocus != 0);
    retval = true;
    break;
  }

  add_cocoa_modifier_flags(swb_event._flags, _modifiers);

  switch (ce.type) {
  case P3DCocoaEventMouseDown:
  case P3DCocoaEventMouseMoved:
  case P3DCocoaEventMouseDragged:
    swb_event._x = (int)ce.data.mouse.pluginX;
    swb_event._y = (int)ce.data.mouse.pluginY;
    swb_event._flags |= SubprocessWindowBuffer::EF_mouse_position;
  }

  if (_mouse_active) {
    swb_event._flags |= SubprocessWindowBuffer::EF_has_mouse;
  }

  if (_swbuffer != NULL) {
    _swbuffer->add_event(swb_event);
  }
#endif  // __APPLE__

  return retval;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_carbon_modifier_flags
//       Access: Private
//  Description: OSX only: adds the appropriate bits to the Event flag
//               bitmask to correspond to the modifier buttons held in
//               the MacOS-style EventRecord::modifiers mask.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_carbon_modifier_flags(unsigned int &swb_flags, int modifiers) {
#if defined(__APPLE__) && !__LP64__
  if (modifiers & cmdKey) {
    swb_flags |= SubprocessWindowBuffer::EF_meta_held;
  }
  if (modifiers & shiftKey) {
    swb_flags |= SubprocessWindowBuffer::EF_shift_held;
  }
  if (modifiers & optionKey) {
    swb_flags |= SubprocessWindowBuffer::EF_alt_held;
  }
  if (modifiers & controlKey) {
    swb_flags |= SubprocessWindowBuffer::EF_control_held;
  }
#endif  // __APPLE__
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_cocoa_modifier_flags
//       Access: Private
//  Description: OSX only: adds the appropriate bits to the Event flag
//               bitmask to correspond to the modifier buttons held in
//               the P3DCocoaEvent modifierFlags mask.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_cocoa_modifier_flags(unsigned int &swb_flags, int modifiers) {
#ifdef __APPLE__
  if (modifiers & NSCommandKeyMask) {
    swb_flags |= SubprocessWindowBuffer::EF_meta_held;
  }
  if (modifiers & NSShiftKeyMask) {
    swb_flags |= SubprocessWindowBuffer::EF_shift_held;
  }
  if (modifiers & NSAlternateKeyMask) {
    swb_flags |= SubprocessWindowBuffer::EF_alt_held;
  }
  if (modifiers & NSControlKeyMask) {
    swb_flags |= SubprocessWindowBuffer::EF_control_held;
  }
#endif  // __APPLE__
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::send_notify
//       Access: Private
//  Description: Generates a synthetic notify message here at the C++
//               level.
//
//               Most notify messages are generated from within the
//               Python code, and don't use this method; but a few
//               have to be sent before Python has started, and those
//               come through this method.
////////////////////////////////////////////////////////////////////
void P3DInstance::
send_notify(const string &message) {
  nout << "send_notify(" << message << ")\n";
  P3D_request *request = new P3D_request;
  request->_instance = NULL;
  request->_request_type = P3D_RT_notify;
  request->_request._notify._message = strdup(message.c_str());
  add_baked_request(request);
}

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::alloc_swbuffer
//       Access: Private
//  Description: OSX only: allocates the _swbuffer and associated
//               support objects.  If it was already allocated,
//               deallocates the previous one first.
////////////////////////////////////////////////////////////////////
void P3DInstance::
alloc_swbuffer() {
  free_swbuffer();

  int x_size = _wparams.get_win_width();
  int y_size = _wparams.get_win_height();

  _swbuffer = SubprocessWindowBuffer::new_buffer
    (_shared_fd, _shared_mmap_size, _shared_filename, x_size, y_size);
  if (_swbuffer != NULL) {
    _reversed_buffer = new char[_swbuffer->get_framebuffer_size()];
    memset(_reversed_buffer, 0, _swbuffer->get_row_size());
    size_t rowsize = _swbuffer->get_row_size();
    
    _buffer_data = CFDataCreateWithBytesNoCopy(NULL, (const UInt8 *)_reversed_buffer, 
                                               y_size * rowsize, kCFAllocatorNull);
    
    _data_provider = CGDataProviderCreateWithCFData(_buffer_data);
    _buffer_color_space = CGColorSpaceCreateDeviceRGB();
    
    _buffer_image = CGImageCreate(x_size, y_size, 8, 32, rowsize, _buffer_color_space,
                                  kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little, 
                                  _data_provider, NULL, false, kCGRenderingIntentDefault);
    
  }
}
#endif  // __APPLE__

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::free_swbuffer
//       Access: Private
//  Description: OSX only: releases the _swbuffer and associated
//               support objects previously allocated by
//               alloc_swbuffer().
////////////////////////////////////////////////////////////////////
void P3DInstance::
free_swbuffer() {
  if (_swbuffer != NULL) {
    SubprocessWindowBuffer::destroy_buffer(_shared_fd, _shared_mmap_size,
                                           _shared_filename, _swbuffer);
    _swbuffer = NULL;
  }
  if (_reversed_buffer != NULL) {
    delete[] _reversed_buffer;
    _reversed_buffer = NULL;
  }

  if (_buffer_image != NULL) {
    CGImageRelease(_buffer_image);
    CGColorSpaceRelease(_buffer_color_space);
    CGDataProviderRelease(_data_provider);
    CFRelease(_buffer_data);

    _buffer_data = NULL;
    _data_provider = NULL;
    _buffer_color_space = NULL;
    _buffer_image = NULL;
  }
}
#endif  // __APPLE__
  

#ifdef __APPLE__
  ////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::timer_callback
//       Access: Private, Static
//  Description: OSX only: this callback is associated with a
//               CFRunLoopTimer, to be called periodically for
//               updating the frame.
////////////////////////////////////////////////////////////////////
void P3DInstance::
timer_callback(CFRunLoopTimerRef timer, void *info) {
  P3DInstance *self = (P3DInstance *)info;
  self->request_refresh();
}
#endif  // __APPLE__

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::ImageDownload::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstance::ImageDownload::
ImageDownload(P3DInstance *inst, int index) :
  _inst(inst),
  _index(index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::ImageDownload::download_finished
//       Access: Protected, Virtual
//  Description: Intended to be overloaded to generate a callback
//               when the download finishes, either successfully or
//               otherwise.  The bool parameter is true if the
//               download was successful.
////////////////////////////////////////////////////////////////////
void P3DInstance::ImageDownload::
download_finished(bool success) {
  P3DFileDownload::download_finished(success);
  if (success) {
    // We've successfully downloaded the image (directly, not via the
    // package interface).
    _inst->_image_files[_index]._filename = get_filename();

    // Put it onscreen if it's supposed to be onscreen now, and our
    // splash window still exists.
    if (_inst->_splash_window != NULL &&
        _inst->_image_files[_index]._image_placement != P3DSplashWindow::IP_none) {
      P3DSplashWindow::ImagePlacement image_placement = _inst->_image_files[_index]._image_placement;
      _inst->_splash_window->set_image_filename(get_filename(), image_placement);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::InstanceDownload::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstance::InstanceDownload::
InstanceDownload(P3DInstance *inst) :
  _inst(inst)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::InstanceDownload::download_progress
//       Access: Protected, Virtual
//  Description: Intended to be overloaded to generate an occasional
//               callback as new data comes in.
////////////////////////////////////////////////////////////////////
void P3DInstance::InstanceDownload::
download_progress() {
  P3DFileDownload::download_progress();
  _inst->report_instance_progress(get_download_progress(), is_download_progress_known(), get_total_data());
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::InstanceDownload::download_finished
//       Access: Protected, Virtual
//  Description: Intended to be overloaded to generate a callback
//               when the download finishes, either successfully or
//               otherwise.  The bool parameter is true if the
//               download was successful.
////////////////////////////////////////////////////////////////////
void P3DInstance::InstanceDownload::
download_finished(bool success) {
  P3DFileDownload::download_finished(success);
  if (success) {
    // We've successfully downloaded the instance data.
    _inst->report_instance_progress(1.0, true, 0);
    _inst->priv_set_p3d_filename(get_filename());
  } else {
    // Oops, no joy on the instance data.
    _inst->send_notify("ondownloadfail");
    _inst->set_failed();
  }
}
