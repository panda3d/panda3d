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

#include <sstream>
#include <algorithm>

#ifdef __APPLE__
#include <sys/mman.h>
#include <ApplicationServices/ApplicationServices.h>
#endif  // __APPLE__

#ifdef _WIN32
typedef P3DWinSplashWindow SplashWindowType;
#elif defined(__APPLE__)
typedef P3DOsxSplashWindow SplashWindowType;
#elif defined(HAVE_X11)
typedef P3DX11SplashWindow SplashWindowType;
#else
typedef P3DSplashWindow SplashWindowType;
#endif

// These are the various image files we might download for use in the
// splash window.  This list must match the ImageType enum.
const char *P3DInstance::_image_type_names[P3DInstance::IT_num_image_types] = {
  "download",
  "ready",
  "failed",
  "launch",
  "play_ready",
  "play_rollover",
  "play_click",
  "none",  // Not really used.
};

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
  _browser_script_object = NULL;
  _panda_script_object = new P3DMainObject;
  _panda_script_object->set_instance(this);
  _user_data = user_data;
  _request_pending = false;
  _temp_p3d_filename = NULL;
  _image_package = NULL;
  _current_background_image = IT_none;
  _current_button_image = IT_none;
  _got_fparams = false;
  _got_wparams = false;

  _fparams.set_tokens(tokens, num_tokens);
  _fparams.set_args(argc, argv);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  _instance_id = inst_mgr->get_unique_id();
  _hidden = false;
  _allow_python_dev = false;
  _auto_start = false;
  _session = NULL;
  _panda3d = NULL;
  _splash_window = NULL;
  _instance_window_opened = false;
  _stuff_to_download = false;
  
  INIT_LOCK(_request_lock);
  _requested_stop = false;

#ifdef __APPLE__
  _shared_fd = -1;
  _shared_mmap_size = 0;
  _swbuffer = NULL;
  _reversed_buffer = NULL;
  _mouse_active = true;
  _frame_timer = NULL;
#endif  // __APPLE__

  // Set some initial properties.
  _panda_script_object->set_float_property("instanceDownloadProgress", 0.0);
  _panda_script_object->set_float_property("downloadProgress", 0.0);
  _panda_script_object->set_string_property("downloadPackageName", "");
  _panda_script_object->set_string_property("downloadPackageDisplayName", "");
  _panda_script_object->set_bool_property("downloadComplete", false);
  _panda_script_object->set_string_property("status", "initial");
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

  nout << "panda_script_object ref = "
       << _panda_script_object->_ref_count << "\n";
  _panda_script_object->set_instance(NULL);
  P3D_OBJECT_DECREF(_panda_script_object);

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

  if (_splash_window != NULL) {
    delete _splash_window;
    _splash_window = NULL;
  }

  if (_temp_p3d_filename != NULL) {
    delete _temp_p3d_filename;
    _temp_p3d_filename = NULL;
  }

#ifdef __APPLE__
  if (_frame_timer != NULL) {
    CFRunLoopTimerInvalidate(_frame_timer);
    CFRelease(_frame_timer);
  }

  if (_swbuffer != NULL) {
    SubprocessWindowBuffer::destroy_buffer(_shared_fd, _shared_mmap_size,
                                           _shared_filename, _swbuffer);
    _swbuffer = NULL;
  }

  if (_reversed_buffer != NULL) {
    delete[] _reversed_buffer;
    _reversed_buffer = NULL;
  }
#endif    

  DESTROY_LOCK(_request_lock);

  // TODO: empty _raw_requests and _baked_requests queues, and
  // _downloads map.

  // TODO: Is it possible for someone to delete an instance while a
  // download is still running?  Who will crash when this happens?
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_p3d_url
//       Access: Public
//  Description: Specifies a URL that should be contacted to download
//               the instance data.  Normally this, or
//               set_p3d_filename(), is only called once.
//
//               The instance data at the other end of this URL is
//               key.  We can't start the instance until we have
//               downloaded the instance file and examined the
//               p3d_info.xml, and we know what Python version we need
//               and so forth.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_p3d_url(const string &p3d_url) {
  // Make a temporary file to receive the instance data.
  assert(_temp_p3d_filename == NULL);
  _temp_p3d_filename = new P3DTemporaryFile(".p3d");
  _stuff_to_download = true;

  // Maybe it's time to open a splash window now.
  make_splash_window();

  // Mark the time we started downloading, so we'll know when to set
  // the install label.
#ifdef _WIN32
  _start_dl_instance_tick = GetTickCount();
#else
  gettimeofday(&_start_dl_instance_timeval, NULL);
#endif
  _show_dl_instance_progress = false;

  // Start downloading the data.
  InstanceDownload *download = new InstanceDownload(this);
  download->set_url(p3d_url);
  download->set_filename(_temp_p3d_filename->get_filename());

  _panda_script_object->set_string_property("status", "downloading_instance");
  start_download(download);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_p3d_filename
//       Access: Public
//  Description: Specifies the file that contains the instance data.
//               Normally this is only called once.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_p3d_filename(const string &p3d_filename) {
  _fparams.set_p3d_filename(p3d_filename);

  _panda_script_object->set_float_property("instanceDownloadProgress", 1.0);

  // This also sets up some internal data based on the contents of the
  // above file and the associated tokens.

  // Extract the application desc file from the p3d file.
  P3DMultifileReader reader;
  stringstream sstream;
  if (!reader.extract_one(p3d_filename, sstream, "p3d_info.xml")) {
    nout << "No p3d_info.xml file found in " << p3d_filename << "\n";
  } else {
    sstream.seekg(0);
    TiXmlDocument doc;
    sstream >> doc;

    // This also starts required packages downloading.  When all
    // packages have been installed, we will start the instance.
    scan_app_desc_file(&doc);
  }

  // For the moment, all sessions will be unique.  TODO: support
  // multiple instances per session.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  ostringstream strm;
  strm << inst_mgr->get_unique_id();
  _session_key = strm.str();

  // Until we've done all of the above processing, we haven't fully
  // committed to having fparams.  (Setting this flag down here
  // instead of up there avoids starting the instance in
  // scan_app_desc_file(), before we've had a chance to finish
  // processing this method.)
  _got_fparams = true;

  // Generate a special notification: onpluginload, indicating the
  // plugin has read its parameters and is ready to be queried (even
  // if Python has not yet started).
  send_notify("onpluginload");

  // Now that we're all set up, start the instance if we're fully
  // downloaded.
  if (get_packages_ready() && _got_wparams) {
    ready_to_start();
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
        if (_swbuffer != NULL) {
          SubprocessWindowBuffer::destroy_buffer(_shared_fd, _shared_mmap_size,
                                                 _shared_filename, _swbuffer);
          _swbuffer = NULL;
        }
        if (_reversed_buffer != NULL) {
          delete[] _reversed_buffer;
          _reversed_buffer = NULL;
        }
        
        _swbuffer = SubprocessWindowBuffer::new_buffer
          (_shared_fd, _shared_mmap_size, _shared_filename, x_size, y_size);
        if (_swbuffer != NULL) {
          _reversed_buffer = new char[_swbuffer->get_framebuffer_size()];
        }
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

  if (get_packages_ready() && _got_fparams) {
    ready_to_start();
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
  return _panda_script_object;
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
    if (request->_request_type == P3D_RT_notify) {
      // Also eval the associated HTML token, if any.
      string message = request->_request._notify._message;
      string expression = _fparams.lookup_token(message);
      if (!expression.empty() && _browser_script_object != NULL) {
        P3D_object *result = P3D_OBJECT_EVAL(_browser_script_object, expression.c_str());
        P3D_OBJECT_XDECREF(result);
      }

    } else if (request->_request_type == P3D_RT_stop) {
      // We also send an implicit message when Python requests itself
      // to shutdown.
      _panda_script_object->set_pyobj(NULL);
      _panda_script_object->set_string_property("status", "stopped");

      string expression = _fparams.lookup_token("onpythonstop");
      if (!expression.empty() && _browser_script_object != NULL) {
        P3D_object *result = P3D_OBJECT_EVAL(_browser_script_object, expression.c_str());
        P3D_OBJECT_XDECREF(result);
      }
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
  _session->signal_request_ready(this);
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
  request->_instance = this;

  _baked_requests.push_back(request);
  _request_pending = true;

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

  case P3D_RT_notify:
    free((char *)request->_request._notify._message);
    break;
  }

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
//     Function: P3DInstance::handle_event
//       Access: Public
//  Description: Responds to the os-generated window event.  Returns
//               true if the event is handled, false if ignored.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
handle_event(P3D_event_data event) {
  bool retval = false;
  if (_splash_window != NULL) {
    if (_splash_window->handle_event(event)) {
      retval = true;
    }
  }

#ifdef _WIN32
  // This function is not used in Win32 and does nothing.

#elif defined(__APPLE__)
  EventRecord *er = event._event;

  // Need to ensure we have the correct port set, in order to
  // convert the mouse coordinates successfully via
  // GlobalToLocal().
  GrafPtr out_port = _wparams.get_parent_window()._port;
  GrafPtr port_save = NULL;
  Boolean port_changed = QDSwapPort(out_port, &port_save);
  
  Point pt = er->where;
  GlobalToLocal(&pt);

  if (port_changed) {
    QDSwapPort(port_save, NULL);
  }

  SubprocessWindowBuffer::Event swb_event;
  swb_event._source = SubprocessWindowBuffer::ES_none;
  swb_event._type = SubprocessWindowBuffer::ET_none;
  swb_event._code = 0;
  swb_event._flags = 0;
  add_modifier_flags(swb_event._flags, er->modifiers);

  switch (er->what) {
  case mouseDown:
  case mouseUp:
    {
      P3D_window_handle window = _wparams.get_parent_window();
      swb_event._source = SubprocessWindowBuffer::ES_mouse;
      if (er->what == mouseUp) {
        swb_event._type = SubprocessWindowBuffer::ET_button_up;
      } else {
        swb_event._type = SubprocessWindowBuffer::ET_button_down;
      }
      retval = true;
    }
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

  default:
    break;
  }

  if (_mouse_active) {
    swb_event._x = pt.h;
    swb_event._y = pt.v;
    swb_event._flags |= SubprocessWindowBuffer::EF_mouse_position | SubprocessWindowBuffer::EF_has_mouse;
  }

  if (_swbuffer != NULL) {
    _swbuffer->add_event(swb_event);
  }

#endif
  return retval;
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
    _panda3d = package;
  }

  _packages.push_back(package);

  // This call must be at the end of this method, because it might
  // ultimately start the application before it returns (if this was
  // the last required package).
  package->add_instance(this);
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
//               be fed to the download object.  After
//               download_finished() has been called, the P3DDownload
//               object will be deleted.
////////////////////////////////////////////////////////////////////
void P3DInstance::
start_download(P3DDownload *download) {
  assert(download->get_download_id() == 0);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  // Since we're downloading something, we might as well check all
  // contents files from this point on.
  inst_mgr->reset_verify_contents();

  int download_id = inst_mgr->get_unique_id();
  download->set_download_id(download_id);

  bool inserted = _downloads.insert(Downloads::value_type(download_id, download)).second;
  assert(inserted);

  P3D_request *request = new P3D_request;
  request->_request_type = P3D_RT_get_url;
  request->_request._get_url._url = strdup(download->get_url().c_str());
  request->_request._get_url._unique_id = download_id;

  add_baked_request(request);
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
  request->_request_type = P3D_RT_refresh;
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

  return xinstance;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::splash_button_clicked
//       Access: Public
//  Description: Called by the P3DSplashWindow code when the user
//               clicks the button visible on the splash window.
////////////////////////////////////////////////////////////////////
void P3DInstance::
splash_button_clicked() {
  if (_session == NULL) {
    play_button_clicked();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::play_button_clicked
//       Access: Public
//  Description: Called to start the game by the user clicking the
//               "play" button, or by JavaScript calling start().
////////////////////////////////////////////////////////////////////
void P3DInstance::
play_button_clicked() {
  if (_session == NULL) {
    if (_splash_window != NULL) {
      _splash_window->set_button_active(false);
    }
    set_background_image(IT_launch);
    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    inst_mgr->start_instance(this);
  }
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

  const char *log_basename = xpackage->Attribute("log_basename");
  if (log_basename != NULL) {
    _log_basename = log_basename;
  }

  TiXmlElement *xconfig = xpackage->FirstChildElement("config");
  if (xconfig != NULL) {
    int hidden = 0;
    if (xconfig->QueryIntAttribute("hidden", &hidden) == TIXML_SUCCESS) {
      _hidden = (hidden != 0);
    }

    int allow_python_dev = 0;
    if (xconfig->QueryIntAttribute("allow_python_dev", &allow_python_dev) == TIXML_SUCCESS) {
      _allow_python_dev = (allow_python_dev != 0);
    }

    int auto_start = 0;
    if (xconfig->QueryIntAttribute("auto_start", &auto_start) == TIXML_SUCCESS) {
      _auto_start = (auto_start != 0);
    }
  }

  // auto_start is true if it is set in the application itself, or in
  // the web tokens.
  if (_fparams.lookup_token_int("auto_start") != 0) {
    _auto_start = true;
  }
  nout << "_auto_start = " << _auto_start << "\n";

  // But auto_start will be set false if the p3d file has not been
  // signed by an approved signature.  TODO.

  if (_hidden && _got_wparams) {
    _wparams.set_window_type(P3D_WT_hidden);
  }

  TiXmlElement *xrequires = xpackage->FirstChildElement("requires");
  while (xrequires != NULL) {
    const char *name = xrequires->Attribute("name");
    const char *host_url = xrequires->Attribute("host");
    if (name != NULL && host_url != NULL) {
      const char *version = xrequires->Attribute("version");
      if (version == NULL) {
        version = "";
      }
      P3DHost *host = inst_mgr->get_host(host_url);
      P3DPackage *package = host->get_package(name, version);
      add_package(package);
    }

    xrequires = xrequires->NextSiblingElement("requires");
  }
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
  if (_browser_script_object != NULL) {
    xcommand->LinkEndChild(_session->p3dobj_to_xml(_browser_script_object));
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
      request->_request_type = P3D_RT_stop;

    } else {
      nout << "Ignoring request of type " << rtype << "\n";
    }
  }

  if (request != NULL) {
    request->_instance = this;
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
  nout << "Got notify: " << message << "\n";
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
      _panda_script_object->set_pyobj(result);
      P3D_OBJECT_DECREF(result);
    }

    _panda_script_object->set_string_property("status", "running");

  } else if (message == "onwindowopen") {
    // The process told us that it just succesfully opened its
    // window.  Tear down the splash window.
    _instance_window_opened = true;
    if (_splash_window != NULL) {
      delete _splash_window;
      _splash_window = NULL;
    }

    for (int i = 0; i < (int)IT_num_image_types; ++i) {
      _image_files[i].cleanup();
    }

    _panda_script_object->set_string_property("status", "open");

#ifdef __APPLE__
    // Start a timer to update the frame repeatedly.  This seems to be
    // steadier than waiting for nullEvent.
    CFRunLoopTimerContext timer_context;
    memset(&timer_context, 0, sizeof(timer_context));
    timer_context.info = this;
    _frame_timer = CFRunLoopTimerCreate
      (NULL, 0, 1.0 / 60.0, 0, 0, timer_callback, &timer_context);
    CFRunLoopRef run_loop = CFRunLoopGetCurrent();
    CFRunLoopAddTimer(run_loop, _frame_timer, kCFRunLoopCommonModes);
#endif  // __APPLE__
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
      P3D_OBJECT_SET_PROPERTY(object, property_name.c_str(), value);
    
    TiXmlElement *xvalue = new TiXmlElement("value");
    xvalue->SetAttribute("type", "bool");
    xvalue->SetAttribute("value", (int)result);
    xcommand->LinkEndChild(xvalue);

  } else if (operation == "del_property") {
    bool result = P3D_OBJECT_SET_PROPERTY(object, property_name.c_str(), NULL);
    
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
//     Function: P3DInstance::make_splash_window
//       Access: Private
//  Description: Creates the splash window to be displayed at startup,
//               if it's time.
////////////////////////////////////////////////////////////////////
void P3DInstance::
make_splash_window() {
  if (_splash_window != NULL || _instance_window_opened) {
    // Already got one, or we're already showing the real instance.
    return;
  }
  if (!_got_wparams) {
    // Don't know where to put it yet.
    return;
  }
  if (_wparams.get_window_type() == P3D_WT_hidden) {
    // We're hidden, and so is the splash window.
    return;
  }
  if (_wparams.get_window_type() != P3D_WT_embedded && 
      !_stuff_to_download && _auto_start) {
    // If it's a toplevel or fullscreen window, then we don't want a
    // splash window until we have stuff to download.
    return;
  }

  _splash_window = new SplashWindowType(this);
  _splash_window->set_wparams(_wparams);
  _splash_window->set_install_label(_install_label);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  // Direct the "download" image to the background slot on the splash
  // window for now, while we perform the download.
  set_background_image(IT_download);

  // Go get the required images.
  bool any_standard_images = false;
  for (int i = 0; i < (int)IT_none; ++i) {
    string token_keyword = string(_image_type_names[i]) + "_img";
    if (!_fparams.has_token(token_keyword)) {
      // No specific image for this type is specified; get the default
      // image.  We do this via the P3DPackage interface, so we can
      // use the cached version on disk if it's good.
      _image_files[i]._use_standard_image = true;
      any_standard_images = true;
      
    } else {
      // We have an explicit image specified for this slot, so just
      // download it directly.  This one won't be cached locally
      // (though the browser might be free to cache it).
      _image_files[i]._use_standard_image = false;
      string image_url = _fparams.lookup_token(token_keyword);
      if (image_url.empty()) {
        // No splash image.  Never mind.
        return;
      }
      
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

  if (any_standard_images) {
    // If any of the images requires an image from the standard image
    // package, go get that package.
    if (_image_package == NULL) {
      P3DHost *host = inst_mgr->get_host(PANDA_PACKAGE_HOST_URL);
      _image_package = host->get_package("images", "");
      _image_package->add_instance(this);
    }
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
  if (image_type != _current_background_image) {
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
  if (image_type != _current_button_image) {
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
      } else {
        _splash_window->set_image_filename(string(), P3DSplashWindow::IP_button_ready);
        _splash_window->set_image_filename(string(), P3DSplashWindow::IP_button_rollover);
        _splash_window->set_image_filename(string(), P3DSplashWindow::IP_button_click);
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
  if (package == _image_package) {
    // A special case: the image package gets immediately downloaded,
    // without waiting for anything else.
    package->activate_download();
    return;
  }

  if (get_packages_info_ready()) {
    // All packages are ready to go.  Let's start some download
    // action.
    _downloading_packages.clear();
    _total_download_size = 0;
    Packages::const_iterator pi;
    for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
      P3DPackage *package = (*pi);
      if (package->get_info_ready() && !package->get_ready()) {
        _downloading_packages.push_back(package);
        _total_download_size += package->get_download_size();
      }
    }
    _download_package_index = 0;
    _total_downloaded = 0;

    nout << "Beginning download of " << _downloading_packages.size()
         << " packages, total " << _total_download_size
         << " bytes required.\n";

    if (_downloading_packages.size() > 0) {
      _stuff_to_download = true;

      // Maybe it's time to open a splash window now.
      make_splash_window();
    }

    if (_splash_window != NULL) {
      _splash_window->set_install_progress(0.0);
    }
    _panda_script_object->set_string_property("status", "downloading");
    _panda_script_object->set_int_property("numDownloadingPackages", _downloading_packages.size());
    _panda_script_object->set_int_property("totalDownloadSize", _total_download_size);
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
      // Too bad.  TODO: fail.
      return;
    }

    if (!package->get_ready()) {
      // This package is ready to download.  Begin.
      string name = package->get_package_display_name();
      if (name.empty()) {
        name = package->get_package_name();
      }
      _panda_script_object->set_string_property("downloadPackageName", package->get_package_name());
      _panda_script_object->set_string_property("downloadPackageDisplayName", name);
      _panda_script_object->set_int_property("downloadPackageNumber", _download_package_index + 1);
      _panda_script_object->set_int_property("downloadPackageSize", package->get_download_size());
      set_install_label("Installing " + name);

      nout << "Downloading " << package->get_package_name()
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
    if (!_panda_script_object->get_bool_property("downloadComplete")) {
      _panda_script_object->set_bool_property("downloadComplete", true);
      _panda_script_object->set_string_property("status", "starting");
      send_notify("ondownloadcomplete");
    }

    // Take down the download progress bar.
    if (_splash_window != NULL) {
      _splash_window->set_install_progress(0.0);
      _splash_window->set_install_label("");
    }

    if (_got_wparams && _got_fparams) {
      ready_to_start();
    }
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
  send_notify("onready");
  if (_auto_start) {
    set_background_image(IT_launch);
    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    inst_mgr->start_instance(this);

  } else if (_splash_window != NULL) {
    // We're fully downloaded, and waiting for the user to click play.
    set_background_image(IT_ready);
    set_button_image(IT_play_ready);
    _splash_window->set_button_active(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::report_instance_progress
//       Access: Private
//  Description: Notified as the instance file is downloaded.
////////////////////////////////////////////////////////////////////
void P3DInstance::
report_instance_progress(double progress) {
  if (!_show_dl_instance_progress) {
    // If we haven't yet set the download label, set it after a full
    // second has elapsed.  We don't want to set it too soon, because
    // we're not really sure how long it will take to download (the
    // instance file might be already in the browser cache).
#ifdef _WIN32
    int now = GetTickCount();
    double elapsed = (double)(now - _start_dl_instance_tick) * 0.001;
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    double elapsed = (double)(now.tv_sec - _start_dl_instance_timeval.tv_sec) +
      (double)(now.tv_usec - _start_dl_instance_timeval.tv_usec) / 1000000.0;
#endif

    // Put up the progress bar after 2 seconds have elapsed, if we've
    // still got some distance to go; or after 5 seconds have elapsed
    // regardless.
    if ((elapsed > 2.0 && progress < 0.7) ||
        (elapsed > 5.0)) {
      _show_dl_instance_progress = true;
    }
  }

  if (_splash_window != NULL && _show_dl_instance_progress) {
    _splash_window->set_install_progress(progress);
  }
  _panda_script_object->set_float_property("instanceDownloadProgress", progress);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::report_package_progress
//       Access: Private
//  Description: Notified as the packages required by the instance
//               file are downloaded.
////////////////////////////////////////////////////////////////////
void P3DInstance::
report_package_progress(P3DPackage *package, double progress) {
  if (_download_package_index >= (int)_downloading_packages.size() ||
      package != _downloading_packages[_download_package_index]) {
    // Quietly ignore a download progress report from an unexpected
    // package.
    return;
  }

  // Scale the progress into the range appropriate to this package.
  progress = (progress * package->get_download_size() + _total_downloaded) / _total_download_size;
  progress = min(progress, 1.0);

  if (_splash_window != NULL) {
    _splash_window->set_install_progress(progress);
  }
  _panda_script_object->set_float_property("downloadProgress", progress);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::report_package_done
//       Access: Private
//  Description: Notified when a required package is fully downloaded,
//               or failed.
////////////////////////////////////////////////////////////////////
void P3DInstance::
report_package_done(P3DPackage *package, bool success) {
  nout << "Done downloading " << package->get_package_name()
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

  if (success) {
    report_package_progress(package, 1.0);
    start_next_download();
  } else {
    // TODO: fail.
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
  P3D_request *request = new P3D_request;
  request->_request_type = P3D_RT_notify;
  request->_request._notify._message = strdup(message.c_str());
  add_baked_request(request);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::paint_window
//       Access: Private
//  Description: Actually paints the rendered image to the browser
//               window.  This is only implemented (and needed) for
//               OSX, where the child process isn't allowed to do it
//               directly.
////////////////////////////////////////////////////////////////////
void P3DInstance::
paint_window() {
#ifdef __APPLE__
  if (_swbuffer == NULL || !_instance_window_opened) {
    return;
  }

  QDErr err;

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
      // On a little-endian machine, we only have to reverse the order of the rows.
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

  } else {
    // No frame ready.  Just re-paint the frame we had saved last
    // time.
  }

  /*
  // This is an attempt to paint the frame using the less-deprecated
  // Quartz interfaces.  Sure does seem like a lot of layers to go
  // through just to paint a bitmap.
  CFDataRef data =
    CFDataCreateWithBytesNoCopy(NULL, (const UInt8 *)_reversed_buffer, 
                                y_size * rowsize, kCFAllocatorNull);

  CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
  CGColorSpaceRef color_space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

  CGImageRef image =
    CGImageCreate(x_size, y_size, 8, 32, rowsize, color_space,
                  kCGImageAlphaFirst | kCGBitmapByteOrder32Little, provider,
                  NULL, false, kCGRenderingIntentDefault);

  CGrafPtr port = _wparams.get_parent_window()._port;
  CGContextRef context;
  err = QDBeginCGContext(port, &context);
  if (err != noErr) {
    nout << "Error: QDBeginCGContext\n";
    return;
  }

  //  CGContextTranslateCTM(context, 0.0, win_height);
  //  CGContextScaleCTM(context, 1.0, -1.0);

  // We have to rely on the clipping rectangle having been set up
  // correctly in order to get the proper location to draw the image.
  // This isn't completely right, because if the image is slightly
  // offscreen, the top left of the clipping rectangle will no longer
  // correspond to the top left of the original image.
  CGRect rect = CGContextGetClipBoundingBox(context);
  nout << "rect: " << rect.origin.x << " " << rect.origin.y
       << " " << rect.size.width << " " << rect.size.height << "\n";
  rect.size.width = x_size;
  rect.size.height = y_size;

  CGContextDrawImage(context, rect, image);
  
  //CGContextSynchronize(context);
  CGContextFlush(context);
  QDEndCGContext(port, &context);

  CGImageRelease(image);
  CGColorSpaceRelease(color_space);
  CGDataProviderRelease(provider);

  CFRelease(data);
  */

  // Painting the frame using the deprecated QuickDraw interfaces.
  Rect src_rect = {0, 0, y_size, x_size};
  Rect ddrc_rect = {0, 0, y_size, x_size};

  GWorldPtr pGWorld;
  err = NewGWorldFromPtr(&pGWorld, k32BGRAPixelFormat, &src_rect, 0, 0, 0, 
                         _reversed_buffer, rowsize);
  if (err != noErr) {
    nout << " error in NewGWorldFromPtr, called from paint_window()\n";
    return;
  }

  GrafPtr out_port = _wparams.get_parent_window()._port;
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
#endif  // __APPLE__  
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_modifier_flags
//       Access: Private
//  Description: OSX only: adds the appropriate bits to the Event flag
//               bitmask to correspond to the modifier buttons held in
//               the MacOS-style EventRecord::modifiers mask.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_modifier_flags(unsigned int &swb_flags, int modifiers) {
#ifdef __APPLE__
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

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::timer_callback
//       Access: Private
//  Description: OSX only: this callback is associated with a
//               CFRunLoopTimer, to be called periodically for
//               updating the frame.
////////////////////////////////////////////////////////////////////
void P3DInstance::
timer_callback(CFRunLoopTimerRef timer, void *info) {
  P3DInstance *self = (P3DInstance *)info;
  self->request_refresh();
  //self->paint_window();
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
  _inst->report_instance_progress(get_download_progress());
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
    _inst->set_p3d_filename(get_filename());
  }
}
