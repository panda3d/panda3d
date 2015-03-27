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
#include "mkdir_complete.h"
#include "parse_color.h"
#include "nppanda3d_common.h"

// We can include this header file to get the DTOOL_PLATFORM
// definition, even though we don't link with dtool.
#include "dtool_platform.h"
#include "pandaVersion.h"

#include <fstream>
#include <algorithm>
#include <string.h>  // strcmp()
#include <time.h>

#ifndef _WIN32
#include <sys/select.h>
#include <sys/time.h>
#endif  // _WIN32

#ifdef HAVE_X11
#include <sys/types.h>
#include <sys/wait.h>
#endif  // HAVE_X11


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
PPInstance(NPMIMEType pluginType, NPP instance, uint16_t mode, 
           int16_t argc, char *argn[], char *argv[], NPSavedData *saved,
           P3D_window_handle_type window_handle_type,
           P3D_event_type event_type) {
  _p3d_inst = NULL;

  _npp_instance = instance;
  _npp_mode = mode;
  _window_handle_type = window_handle_type;
  _event_type = event_type;
  _script_object = NULL;
  _contents_expiration = 0;
  _failed = false;
  _started = false;

  // Copy the tokens and save them within this object.
  _tokens.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    if (argn[i] != NULL) {
      const char *v = argv[i];
      if (v == NULL) {
        // Firefox might give us a NULL argv[i] in some cases.
        v = "";
      }

      // Make the token lowercase, since HTML is case-insensitive but
      // we're not.
      string keyword;
      for (const char *p = argn[i]; *p; ++p) {
        keyword += tolower(*p);
      }

      P3D_token token;
      token._keyword = strdup(keyword.c_str());
      token._value = strdup(v);
      _tokens.push_back(token);
    }
  }

  _root_dir = global_root_dir;

  _got_instance_url = false;
  _p3d_instance_id = 0;

  // fgcolor and bgcolor are useful to know here (in case we have to
  // draw a twirling icon).

  // The default bgcolor is white.
  _bgcolor_r = _bgcolor_g = _bgcolor_b = 0xff;
  if (has_token("bgcolor")) {
    int r, g, b;
    if (parse_color(r, g, b, lookup_token("bgcolor"))) {
      _bgcolor_r = r;
      _bgcolor_g = g;
      _bgcolor_b = b;
    }
  }

  // The default fgcolor is either black or white, according to the
  // brightness of the bgcolor.
  if (_bgcolor_r + _bgcolor_g + _bgcolor_b > 0x80 + 0x80 + 0x80) {
    _fgcolor_r = _fgcolor_g = _fgcolor_b = 0x00;
  } else {
    _fgcolor_r = _fgcolor_g = _fgcolor_b = 0xff;
  }
  if (has_token("fgcolor")) {
    int r, g, b;
    if (parse_color(r, g, b, lookup_token("fgcolor"))) {
      _fgcolor_r = r;
      _fgcolor_g = g;
      _fgcolor_b = b;
    }
  }

  _use_xembed = false;
  _got_window = false;
  _python_window_open = false;
#ifdef HAVE_X11
  _twirl_subprocess_pid = -1;
#ifdef HAVE_GTK
  _plug = NULL;
#endif  // HAVE_GTK
#endif  // HAVE_X11

#ifdef _WIN32
  _hwnd = NULL;
  _bg_brush = NULL;
#endif // _WIN32

#ifndef _WIN32
  // Save the startup time to improve precision of gettimeofday().  We
  // also use this to measure elapsed time from the window parameters
  // having been received.
  struct timeval tv;
  gettimeofday(&tv, (struct timezone *)NULL);
  _init_sec = tv.tv_sec;
  _init_usec = tv.tv_usec;
#endif  // !_WIN32

#ifdef __APPLE__
  // Get the run loop in the browser thread.  (CFRunLoopGetMain() is
  // only 10.5 or higher.  Plus, the browser thread is not necessarily
  // the "main" thread.)
  _run_loop_main = CFRunLoopGetCurrent();
  CFRetain(_run_loop_main);
  _request_timer = NULL;
  INIT_LOCK(_timer_lock);

  // Also set up a timer to twirl the icon until the instance loads.
  _twirl_timer = NULL;
  CFRunLoopTimerContext timer_context;
  memset(&timer_context, 0, sizeof(timer_context));
  timer_context.info = this;
  _twirl_timer = CFRunLoopTimerCreate
    (NULL, 0, 0.1, 0, 0, st_twirl_timer_callback, &timer_context);
  CFRunLoopAddTimer(_run_loop_main, _twirl_timer, kCFRunLoopCommonModes);
#endif  // __APPLE__

#ifdef MACOSX_HAS_EVENT_MODELS
  _got_twirl_images = false;
#endif  // MACOSX_HAS_EVENT_MODELS
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPInstance::
~PPInstance() {
  cleanup_window();

#ifdef __APPLE__
  if (_twirl_timer != NULL) {
    CFRunLoopTimerInvalidate(_twirl_timer);
    CFRelease(_twirl_timer);
    _twirl_timer = NULL;
  }
  if (_request_timer != NULL) {
    CFRunLoopTimerInvalidate(_request_timer);
    CFRelease(_request_timer);
    _request_timer = NULL;
  }
  _run_loop_main = CFRunLoopGetCurrent();
  CFRelease(_run_loop_main);
  DESTROY_LOCK(_timer_lock);
#endif  // __APPLE__

#ifdef MACOSX_HAS_EVENT_MODELS
  osx_release_twirl_images();
#endif  // MACOSX_HAS_EVENT_MODELS

  if (_p3d_inst != NULL) {
    P3D_instance_finish_ptr(_p3d_inst);
    _p3d_inst = NULL;
  }

  assert(_streams.empty());
  assert(_file_datas.empty());

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
  // On Windows and Linux, we must insist on having this call.  OSX
  // doesn't necessarily require it (which is lucky, since it appears
  // that Safari doesn't necessarily provide it!)
#ifndef __APPLE__
  if (!has_plugin_thread_async_call) {
    nout << "Browser version insufficient: we require at least NPAPI version 0.19.\n";
    set_failed();
  }
#else
  // While Safari 5 on Mac claims to provide this function, it doesn't
  // appear to work. (!)  So we pretend we never have it on Mac.
  // Fortunately, this hack does us no harm because the _request_timer
  // hack works fine on OSX.
  has_plugin_thread_async_call = false;
#endif  // __APPLE__

  string url = PANDA_PACKAGE_HOST_URL;
  if (!url.empty() && url[url.length() - 1] != '/') {
    url += '/';
  }
  _download_url_prefix = url;
  _standard_url_prefix = url;
  nout << "Plugin is built with " << PANDA_PACKAGE_HOST_URL << "\n";

  if (!is_plugin_loaded() && !_failed) {
    // We need to read the contents.xml file.  First, check to see if
    // the version on disk is already current enough.
    bool success = false;

    string contents_filename = _root_dir + "/contents.xml";
    if (read_contents_file(contents_filename, false)) {
      if (time(NULL) < _contents_expiration) {
        // Got the file, and it's good.
        get_core_api();
        success = true;
      }
    }

    if (!success) {
      // Go download the latest contents.xml file.
      _download_url_prefix = _standard_url_prefix;
      _mirrors.clear();
      ostringstream strm;
      strm << _download_url_prefix << "contents.xml";
      
      // Append a uniquifying query string to the URL to force the
      // download to go all the way through any caches.  We use the time
      // in seconds; that's unique enough.
      strm << "?" << time(NULL);
      url = strm.str();
      
      PPDownloadRequest *req = new PPDownloadRequest(PPDownloadRequest::RT_contents_file);
      start_download(url, req);
    }
  }

  handle_request_loop();
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

  if (!_got_window) {
#ifdef _WIN32
    _orig_window_proc = NULL;
    if (window->type == NPWindowTypeWindow) {
      // Save the window handle.
      _hwnd = (HWND)window->window;

      // Now that we've got a window handle, we can go get the
      // twirling icon images.
      win_get_twirl_bitmaps();

      _bg_brush = CreateSolidBrush(RGB(_bgcolor_r, _bgcolor_g, _bgcolor_b));

      // Subclass the window to make it call our own window_proc
      // instead of whatever window_proc it has already.  This is
      // mainly just a dopey trick to allow us to poll events in the
      // main thread, but we also rely on this to paint the twirling
      // icon into the browser window.
      SetWindowLongPtr(_hwnd, GWLP_USERDATA, (LONG_PTR)this);
      _orig_window_proc = SetWindowLongPtr(_hwnd, GWLP_WNDPROC, (LONG_PTR)st_window_proc);

      // Also set a timer to go off every once in a while, to update
      // the twirling icon, and also to catch events in case something
      // slips through.
      _init_time = GetTickCount();
      SetTimer(_hwnd, 1, 100, NULL);
    }
#endif  // _WIN32
#ifdef MACOSX_HAS_EVENT_MODELS
    osx_get_twirl_images();
#endif  // MACOSX_HAS_EVENT_MODELS
  }

#if defined(HAVE_GTK) && defined(HAVE_X11)
  if (_use_xembed) {
    if (!_got_window || _window.window != window->window) {
      // The window has changed.  Destroy the old GtkPlug.
      if (_plug != NULL) {
        gtk_widget_destroy(_plug);
        _plug = NULL;
      }

      // Create a new GtkPlug to bind to the XEmbed socket.
      _plug = gtk_plug_new((GdkNativeWindow) reinterpret_cast<XID>(window->window));
      gtk_widget_show(_plug);
      
      nout << "original XID is " << window->window << ", created X11 window "
           << GDK_DRAWABLE_XID(_plug->window) << "\n";
    }
  }
#endif  // HAVE_GTK && HAVE_X11

  _window = *window;
  _got_window = true;

#ifdef HAVE_X11
  if (!_failed && !_started) {
    x11_start_twirl_subprocess();
  }
#endif  // HAVE_X11

  if (!_failed) {
    if (_p3d_inst == NULL) {
      create_instance();
    } else {
      send_window();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::new_stream
//       Access: Public
//  Description: Receives notification of a new stream object, e.g. a
//               url request.
////////////////////////////////////////////////////////////////////
NPError PPInstance::
new_stream(NPMIMEType type, NPStream *stream, bool seekable, uint16_t *stype) {
  assert(find(_streams.begin(), _streams.end(), stream) == _streams.end());
  if (_failed) {
    return NPERR_GENERIC_ERROR;
  }

  if (stream->notifyData == NULL) {
    // This is an unsolicited stream.  Assume the first unsolicited
    // stream we receive is the instance data; any other unsolicited
    // stream is an error.

    if (!_got_instance_url && stream->url != NULL) {
      _got_instance_url = true;
      _instance_url = stream->url;
      stream->notifyData = new PPDownloadRequest(PPDownloadRequest::RT_instance_data);

      if (_p3d_inst != NULL) {
        // If we already have an instance by the time we get this
        // stream, start sending the data to the instance (instead of
        // having to mess around with a temporary file).
        _p3d_instance_id = P3D_instance_start_stream_ptr(_p3d_inst, _instance_url.c_str());
        nout << "p3d instance to stream " << _p3d_instance_id << "\n";
      }
      
      *stype = NP_NORMAL;
      _streams.push_back(stream);
      return NPERR_NO_ERROR;
    }

    // Don't finish downloading the unsolicited stream.
    return NPERR_GENERIC_ERROR;
  }

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);
  switch (req->_rtype) {
  case PPDownloadRequest::RT_contents_file:
    // This is the initial contents.xml file.  We used to download
    // this via NP_ASFILEONLY, but that option doesn't work on Windows
    // within a Unicode user directory.  So we use NP_NORMAL instead.
    *stype = NP_NORMAL;
    _streams.push_back(stream);
    return NPERR_NO_ERROR;

  case PPDownloadRequest::RT_core_dll:
    // This is the core API DLL (or dylib or whatever).  Again, we
    // have to use NP_NORMAL.
    *stype = NP_NORMAL;
    _streams.push_back(stream);
    return NPERR_NO_ERROR;

  case PPDownloadRequest::RT_user:
    // This is a request from the plugin.  We'll receive this as a
    // stream.
    *stype = NP_NORMAL;
    _streams.push_back(stream);
    return NPERR_NO_ERROR;

  default:
    // Don't know what this is.
    nout << "Unexpected request " << (int)req->_rtype << "\n";
  }

  return NPERR_GENERIC_ERROR;
}


////////////////////////////////////////////////////////////////////
//     Function: PPInstance::stop_outstanding_streams
//       Access: Public
//  Description: Stops any download streams that are currently active
//               on the instance.  It is necessary to call this
//               explicitly before destroying the instance, at least
//               for Safari.
////////////////////////////////////////////////////////////////////
void PPInstance::
stop_outstanding_streams() {
  Streams::iterator si;
  Streams streams;
  streams.swap(_streams);
  for (si = streams.begin(); si != streams.end(); ++si) {
    NPStream *stream = (*si);
    nout << "Stopping stream " << (void *)stream << "\n";
    browser->destroystream(_npp_instance, stream, NPRES_USER_BREAK);
    destroy_stream(stream, NPRES_USER_BREAK);
  }

  assert(_streams.empty());

  // Also stop any currently pending _file_datas; these are
  // locally-implemented streams.
  FileDatas::iterator fi;
  for (fi = _file_datas.begin(); fi != _file_datas.end(); ++fi) {
    delete (*fi);
  }
  _file_datas.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::write_ready
//       Access: Public
//  Description: Called by the browser to ask how much data is ready
//               to be received for the indicated stream.
////////////////////////////////////////////////////////////////////
int32_t PPInstance::
write_ready(NPStream *stream) {
  // We're supposed to return the maximum amount of data the plugin is
  // prepared to handle.  Gee, I don't know.  As much as you can give
  // me, I guess.
  return 0x7fffffff;
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

  if (_failed) {
    // We're done; stop this.
    browser->destroystream(_npp_instance, stream, NPRES_USER_BREAK);
    return 0;
  }

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);
  switch (req->_rtype) {
  case PPDownloadRequest::RT_user:
    P3D_instance_feed_url_stream_ptr(_p3d_inst, req->_user_id,
                                     P3D_RC_in_progress, 0,
                                     stream->end, buffer, len);
    return len;

  case PPDownloadRequest::RT_instance_data:
    // There's a special case for the RT_instance_data stream.  This
    // is the first, unsolicited stream that indicates the p3d
    // instance data.  We have to send this stream into the instance,
    // but we can only do this once the instance itself has been
    // created.

    // We used to get away with returning 0 in write_ready until the
    // instance was ready, but that turns out to fail under Safari
    // Snow Leopard, which it seems will hold up every other download
    // until the p3d file has been retrieved.  Sigh.  So we must start
    // accepting the data even before the instance has been created,
    // or we'll never get our contents.xml or any other important bits
    // of data.

    // Nowadays we solve this problem by writing the data to a
    // temporary file until the instance is ready for it.
    if (_p3d_inst == NULL) {
      // The instance isn't ready, so stuff it in a temporary file.
      if (!_p3d_temp_file.feed(stream->end, buffer, len)) {
        set_failed();
      }
      return len;

    } else {
      // The instance has been created.  Redirect the stream into the
      // instance.
      assert(!_p3d_temp_file._opened);
      req->_rtype = PPDownloadRequest::RT_user;
      req->_user_id = _p3d_instance_id;
      P3D_instance_feed_url_stream_ptr(_p3d_inst, req->_user_id,
                                       P3D_RC_in_progress, 0,
                                       stream->end, buffer, len);
      return len;
    }
    break;

  case PPDownloadRequest::RT_contents_file:
    if (!_contents_temp_file.feed(stream->end, buffer, len)) {
      set_failed();
    }
    return len;

  case PPDownloadRequest::RT_core_dll:
    if (!_core_dll_temp_file.feed(stream->end, buffer, len)) {
      set_failed();
    }
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
  Streams::iterator si = find(_streams.begin(), _streams.end(), stream);
  if (si != _streams.end()) {
    _streams.erase(si);
  }

  if (stream->notifyData == NULL) {
    nout << "Unexpected destroy_stream on " << stream->url << "\n";
    return NPERR_NO_ERROR;
  }

  PPDownloadRequest *req = (PPDownloadRequest *)(stream->notifyData);

  P3D_result_code result_code = P3D_RC_done;
  if (reason != NPRES_DONE) {
    if (reason == NPRES_USER_BREAK) {
      result_code = P3D_RC_shutdown;
    } else {
      result_code = P3D_RC_generic_error;
    }
  }

  switch (req->_rtype) {
  case PPDownloadRequest::RT_user:
    if (!req->_notified_done) {
      P3D_instance_feed_url_stream_ptr(_p3d_inst, req->_user_id,
                                       result_code, 0, stream->end, NULL, 0);
      req->_notified_done = true;
    }
    break;

  case PPDownloadRequest::RT_instance_data:
    if (!req->_notified_done) {
      if (_p3d_inst == NULL) {
        // The instance still isn't ready; just mark the data done.
        // We'll send the entire file to the instance when it is ready.
        _p3d_temp_file.finish();
        if (result_code != P3D_RC_done) {
          set_failed();
        }
        
      } else {
        // The instance has (only just) been created.  Tell it we've
        // sent it all the data it will get.
        P3D_instance_feed_url_stream_ptr(_p3d_inst, _p3d_instance_id,
                                         result_code, 0, stream->end, NULL, 0);
      }
      req->_notified_done = true;
    }
    break;

  case PPDownloadRequest::RT_contents_file:
    if (!req->_notified_done) {
      _contents_temp_file.finish();
      if (result_code != P3D_RC_done) {
        set_failed();
      }
      req->_notified_done = true;
    }
    break;

  case PPDownloadRequest::RT_core_dll:
    if (!req->_notified_done) {
      _core_dll_temp_file.finish();
      if (result_code != P3D_RC_done) {
        set_failed();
      }
      req->_notified_done = true;
    }
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
  if (_failed) {
    // We're done; ignore this.
    delete req;
    return;
  }

  switch (req->_rtype) {
  case PPDownloadRequest::RT_user:
    if (!req->_notified_done) {
      // We shouldn't have gotten here without notifying the stream
      // unless the stream never got started (and hence we never
      // called destroy_stream().
      nout << "Failure starting stream\n";
      assert(reason != NPRES_DONE);

      P3D_instance_feed_url_stream_ptr(_p3d_inst, req->_user_id,
                                       P3D_RC_generic_error, 0, 0, NULL, 0);
      req->_notified_done = true;
    }
    break;

  case PPDownloadRequest::RT_contents_file:
    if (reason == NPRES_DONE) {
      downloaded_contents_file(_contents_temp_file._filename);
    } else {
      nout << "Failure downloading " << url << "\n";

      if (reason == NPRES_USER_BREAK) {
        nout << "Failure due to user break\n";
      } else {
        // Couldn't download a fresh contents.xml for some reason.  If
        // there's an outstanding contents.xml file on disk, try to
        // load that one as a fallback.
        string contents_filename = _root_dir + "/contents.xml";
        if (read_contents_file(contents_filename, false)) {
          get_core_api();
        } else {
          nout << "Unable to read contents file " << contents_filename << "\n";
          set_failed();
        }
      }
    }
    break;
    
  case PPDownloadRequest::RT_core_dll:
    if (reason == NPRES_DONE) {
      downloaded_plugin(_core_dll_temp_file._filename);

    } else {
      nout << "Failure downloading " << url << "\n";

      if (reason == NPRES_USER_BREAK) {
        nout << "Failure due to user break\n";
      } else {
        // Couldn't download from this mirror.  Try the next one.
        if (!_core_urls.empty()) {
          string url = _core_urls.back();
          _core_urls.pop_back();
          
          PPDownloadRequest *req2 = new PPDownloadRequest(PPDownloadRequest::RT_core_dll);
          start_download(url, req2);
        }
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
//  Description: Handles a request from the Core API or the
//               application, forwarding it to the browser as
//               appropriate.  Returns true if we should continue the
//               request loop, or false to return (temporarily) to
//               JavaScript.
////////////////////////////////////////////////////////////////////
bool PPInstance::
handle_request(P3D_request *request) {
  if (_p3d_inst == NULL || _failed) {
    return false;
  }
  assert(request->_instance == _p3d_inst);
  bool handled = false;
  bool continue_loop = true;

  switch (request->_request_type) {
  case P3D_RT_stop:
    if (_p3d_inst != NULL) {
      P3D_instance_finish_ptr(_p3d_inst);
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

  case P3D_RT_callback:
    // In the case of a callback, yield control temporarily to JavaScript.
    continue_loop = false;
    break;

  default:
    // Some request types are not handled.
    nout << "Unhandled request: " << request->_request_type << "\n";
    break;
  };

  P3D_request_finish_ptr(request, handled);
  return continue_loop;
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
  /*
  if (!has_plugin_thread_async_call) {
    // If we can't ask Mozilla to call us back using
    // NPN_PluginThreadAsyncCall(), then we'll do it explicitly now,
    // since we know we're in the main thread here.
    handle_request_loop();
  }
  */
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

  P3D_event_data edata;
  memset(&edata, 0, sizeof(edata));
  edata._event_type = _event_type;
  EventAuxData aux_data;
  if (_event_type == P3D_ET_osx_event_record) {
#ifdef __APPLE__
    edata._event._osx_event_record._event = (EventRecord *)event;
#endif  // __APPLE__

#ifdef MACOSX_HAS_EVENT_MODELS
  } else if (_event_type == P3D_ET_osx_cocoa) {
    // Copy the NPCocoaEvent structure componentwise into a
    // P3DCocoaEvent structure.
    NPCocoaEvent *np_event = (NPCocoaEvent *)event;
    P3DCocoaEvent *p3d_event = &edata._event._osx_cocoa._event;
    copy_cocoa_event(p3d_event, np_event, aux_data);
    if (_got_window) {
      handle_cocoa_event(p3d_event);
    }
#endif  // MACOSX_HAS_EVENT_MODELS
  }

  if (_p3d_inst != NULL) {
    if (P3D_instance_handle_event_ptr(_p3d_inst, &edata)) {
      retval = true;
    }
  }

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
    main = P3D_instance_get_panda_script_object_ptr(_p3d_inst);
  }
  nout << "get_panda_script_object, main = " << main << "\n";

  _script_object = PPToplevelObject::make_new(this);
  _script_object->set_main(main);

  browser->retainobject(_script_object);
  return _script_object;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::set_xembed
//       Access: Public
//  Description: Sets the use_xembed flag, telling the instance what
//               kind of window object to expect from NPAPI.  If this
//               is true, the window object is an XID following the
//               XEmbed specification; if false, it is a normal window
//               handle.
////////////////////////////////////////////////////////////////////
void PPInstance::
set_xembed(bool use_xembed) {
  _use_xembed = use_xembed;
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
    return P3D_new_undefined_object_ptr();
  } else if (NPVARIANT_IS_NULL(*variant)) {
    return P3D_new_none_object_ptr();
  } else if (NPVARIANT_IS_BOOLEAN(*variant)) {
    return P3D_new_bool_object_ptr(NPVARIANT_TO_BOOLEAN(*variant));
  } else if (NPVARIANT_IS_INT32(*variant)) {
    return P3D_new_int_object_ptr(NPVARIANT_TO_INT32(*variant));
  } else if (NPVARIANT_IS_DOUBLE(*variant)) {
    return P3D_new_float_object_ptr(NPVARIANT_TO_DOUBLE(*variant));
  } else if (NPVARIANT_IS_STRING(*variant)) {
    NPString str = NPVARIANT_TO_STRING(*variant);
    const UC_NPString &uc_str = *(UC_NPString *)(&str);
    return P3D_new_string_object_ptr(uc_str.UTF8Characters, uc_str.UTF8Length);
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
  return P3D_new_none_object_ptr();
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
    const UC_NPString &uc_str = *(UC_NPString *)(&str);
    out << "string " << string(uc_str.UTF8Characters, uc_str.UTF8Length);
  } else if (NPVARIANT_IS_OBJECT(result)) {
    NPObject *child = NPVARIANT_TO_OBJECT(result);
    out << "object " << child;
  }
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

  if (has_plugin_thread_async_call) {
#ifdef HAS_PLUGIN_THREAD_ASYNC_CALL
    // Since we are running at least Gecko 1.9, and we have this very
    // useful function, let's use it to ask the browser to call us back
    // in the main thread.
    assert((void *)browser->pluginthreadasynccall != (void *)NULL);
    browser->pluginthreadasynccall(inst->_npp_instance, browser_sync_callback, NULL);
#endif  // HAS_PLUGIN_THREAD_ASYNC_CALL

  } else {
    // If we're using an older version of Gecko, we have to do this
    // some other, OS-dependent way.

#ifdef _WIN32
    // Use a Windows message to forward this event to the main thread.
    
    // Get the window handle for the window associated with this
    // instance.
    const NPWindow *win = inst->get_window();
    if (win != NULL && win->type == NPWindowTypeWindow) {
      PostMessage((HWND)(win->window), WM_USER, 0, 0);
    }
#endif  // _WIN32

#ifdef __APPLE__
    // Use an OSX timer to forward this event to the main thread.

    // Only set a new timer if we don't have one already started.
    ACQUIRE_LOCK(inst->_timer_lock);
    if (inst->_request_timer == NULL) {
      CFRunLoopTimerContext timer_context;
      memset(&timer_context, 0, sizeof(timer_context));
      timer_context.info = inst;
      inst->_request_timer = CFRunLoopTimerCreate
        (NULL, 0, 0, 0, 0, timer_callback, &timer_context);
      CFRunLoopAddTimer(inst->_run_loop_main, inst->_request_timer, kCFRunLoopCommonModes);
    }
    RELEASE_LOCK(inst->_timer_lock);
#endif  // __APPLE__

    // Doesn't appear to be a reliable way to simulate this in Linux.
  }
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
//     Function: PPInstance::downloaded_contents_file
//       Access: Private
//  Description: The contents.xml file has been successfully downloaded;
//               copy it into place.
////////////////////////////////////////////////////////////////////
void PPInstance::
downloaded_contents_file(const string &filename) {
  // Now we have the contents.xml file.  Read this to get the
  // filename and md5 hash of our core API DLL.
  if (read_contents_file(filename, true)) {
    // Successfully downloaded and read, and it has been written
    // into its normal place.
    get_core_api();
    
  } else {
    // Error reading the contents.xml file, or in loading the core
    // API that it references.
    nout << "Unable to read contents file " << filename << "\n";
    
    // If there's an outstanding contents.xml file on disk, try to
    // load that one as a fallback.
    string contents_filename = _root_dir + "/contents.xml";
    if (read_contents_file(contents_filename, false)) {
      get_core_api();
    } else {
      nout << "Unable to read contents file " << contents_filename << "\n";
      set_failed();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::read_contents_file
//       Access: Private
//  Description: Attempts to open and read the contents.xml file on
//               disk.  Copies the file to its standard location
//               on success.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool PPInstance::
read_contents_file(const string &contents_filename, bool fresh_download) {
  _download_url_prefix = _standard_url_prefix;
  _mirrors.clear();

  TiXmlDocument doc(contents_filename.c_str());
  if (!doc.LoadFile()) {
    return false;
  }

  bool found_core_package = false;

  TiXmlElement *xcontents = doc.FirstChildElement("contents");
  if (xcontents != NULL) {
    int max_age = P3D_CONTENTS_DEFAULT_MAX_AGE;
    xcontents->Attribute("max_age", &max_age);

    // Get the latest possible expiration time, based on the max_age
    // indication.  Any expiration time later than this is in error.
    time_t now = time(NULL);
    _contents_expiration = now + (time_t)max_age;

    if (fresh_download) {
      // Update the XML with the new download information.
      TiXmlElement *xorig = xcontents->FirstChildElement("orig");
      while (xorig != NULL) {
        xcontents->RemoveChild(xorig);
        xorig = xcontents->FirstChildElement("orig");
      }

      xorig = new TiXmlElement("orig");
      xcontents->LinkEndChild(xorig);
      
      xorig->SetAttribute("expiration", (int)_contents_expiration);

    } else {
      // Read the expiration time from the XML.
      int expiration = 0;
      TiXmlElement *xorig = xcontents->FirstChildElement("orig");
      if (xorig != NULL) {
        xorig->Attribute("expiration", &expiration);
      }
      
      _contents_expiration = min(_contents_expiration, (time_t)expiration);
    }

    nout << "read contents.xml, max_age = " << max_age
         << ", expires in " << max(_contents_expiration, now) - now
         << " s\n";

    // Look for the <host> entry; it might point us at a different
    // download URL, and it might mention some mirrors.
    find_host(xcontents);

    // Now look for the core API package.
    _coreapi_set_ver = "";
    TiXmlElement *xpackage = xcontents->FirstChildElement("package");
    while (xpackage != NULL) {
      const char *name = xpackage->Attribute("name");
      if (name != NULL && strcmp(name, "coreapi") == 0) {
        const char *platform = xpackage->Attribute("platform");
        if (platform != NULL && strcmp(platform, DTOOL_PLATFORM) == 0) {
          _coreapi_dll.load_xml(xpackage);
          const char *set_ver = xpackage->Attribute("set_ver");
          if (set_ver != NULL) {
            _coreapi_set_ver = set_ver;
          }
          found_core_package = true;
          break;
        }
      }
        
      xpackage = xpackage->NextSiblingElement("package");
    }
  }

  if (!found_core_package) {
    // Couldn't find the coreapi package description.
    nout << "No coreapi package defined in contents file for "
         << DTOOL_PLATFORM << "\n";
    return false;
  }

  // Check the coreapi_set_ver token.  If it is given, it specifies a
  // minimum Core API version number we expect to find.  If we didn't
  // find that number, perhaps our contents.xml is out of date.
  string coreapi_set_ver = lookup_token("coreapi_set_ver");
  if (!coreapi_set_ver.empty()) {
    nout << "Instance asked for Core API set_ver " << coreapi_set_ver
         << ", we found " << _coreapi_set_ver << "\n";
    // But don't bother if we just freshly downloaded it.
    if (!fresh_download) {
      if (compare_seq(coreapi_set_ver, _coreapi_set_ver) > 0) {
        // The requested set_ver value is higher than the one we have on
        // file; our contents.xml file must be out of date after all.
        nout << "expiring contents.xml\n";
        _contents_expiration = 0;
      }
    }
  }

  // Success.  Now save the file in its proper place.
  string standard_filename = _root_dir + "/contents.xml";

  mkfile_complete(standard_filename, nout);
  if (!doc.SaveFile(standard_filename.c_str())) {
    nout << "Couldn't rewrite " << standard_filename << "\n";
    return false;
  }
  
  return true;
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
  // Since we're no longer using NP_ASFILEONLY, none of these URL
  // requests will normally come through this codepath (they'll go
  // through url_notify() above, instead), unless we short-circuited
  // the browser by "downloading" a file:// url.
  switch (req->_rtype) {
  case PPDownloadRequest::RT_contents_file:
    // The contents.xml file that gets things going.
    downloaded_contents_file(filename);
    break;

  case PPDownloadRequest::RT_core_dll:
    // This is the core API DLL (or dylib or whatever).  Now that
    // we've downloaded it, we can load it.
    downloaded_plugin(filename);
    break;

  case PPDownloadRequest::RT_user:
    // Here's the user-requested file.  It needs to be streamed to the
    // user, so we'll open the file and feed it to the user.
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
//     Function: PPInstance::send_p3d_temp_file_data
//       Access: Private
//  Description: Once the instance has been created, sends it all of
//               the data we have saved up for it while we were
//               waiting.
////////////////////////////////////////////////////////////////////
void PPInstance::
send_p3d_temp_file_data() {
  assert(_p3d_temp_file._opened);

  nout << "Sending " << _p3d_temp_file._current_size
       << " preliminary bytes of " << _p3d_temp_file._total_size
       << " total p3d data\n";
        
  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  _p3d_temp_file.close();

  ifstream in(_p3d_temp_file._filename.c_str(), ios::binary);
  in.read(buffer, buffer_size);
  size_t total = 0;
  size_t count = in.gcount();
  while (count != 0) {
    P3D_instance_feed_url_stream_ptr(_p3d_inst, _p3d_instance_id,
                                     P3D_RC_in_progress, 0,
                                     _p3d_temp_file._total_size, buffer, count);
    total += count;

    in.read(buffer, buffer_size);
    count = in.gcount();
  }
  nout << "sent " << count << " bytes.\n";

  in.close();

  if (_p3d_temp_file._finished) {
    // If we'd already finished the stream earlier, tell the plugin.
    P3D_instance_feed_url_stream_ptr(_p3d_inst, _p3d_instance_id,
                                     P3D_RC_done, 0, _p3d_temp_file._total_size,
                                     NULL, 0);
  }
  _p3d_temp_file.cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::get_core_api
//       Access: Private
//  Description: Checks the core API DLL file against the
//               specification in the contents file, and downloads it
//               if necessary.
////////////////////////////////////////////////////////////////////
void PPInstance::
get_core_api() {
  if (_coreapi_dll.quick_verify(_root_dir)) {
    // The DLL file is good.  Just load it.
    do_load_plugin();

  } else {
    // The DLL file needs to be downloaded.  Build up our list of
    // URL's to attempt to download it from, in reverse order.
    string url;

    // Our last act of desperation: hit the original host, with a
    // query uniquifier, to break through any caches.
    ostringstream strm;
    strm << _download_url_prefix << _coreapi_dll.get_filename()
         << "?" << time(NULL);
    url = strm.str();
    _core_urls.push_back(url);

    // Before we try that, we'll hit the original host, without a
    // uniquifier.
    url = _download_url_prefix;
    url += _coreapi_dll.get_filename();
    _core_urls.push_back(url);

    // And before we try that, we'll try two mirrors, at random.
    vector<string> mirrors;
    choose_random_mirrors(mirrors, 2);
    for (vector<string>::iterator si = mirrors.begin();
         si != mirrors.end(); 
         ++si) {
      url = (*si) + _coreapi_dll.get_filename();
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
  if (!_coreapi_dll.quick_verify_pathname(filename)) {
    nout << "After download, " << _coreapi_dll.get_filename() << " is no good.\n";
    nout << "Expected:\n";
    _coreapi_dll.write(nout);
    const FileSpec *actual = _coreapi_dll.force_get_actual_file(filename);
    if (actual != NULL) {
      nout << "Found:\n";
      actual->write(nout);
    }

    // That DLL came out wrong.  Try the next URL.
    if (!_core_urls.empty()) {
      string url = _core_urls.back();
      _core_urls.pop_back();

      _core_dll_temp_file.cleanup();
      PPDownloadRequest *req = new PPDownloadRequest(PPDownloadRequest::RT_core_dll);
      start_download(url, req);
      return;
    }

    set_failed();
    return;
  }

  // Copy the file onto the target.
  string pathname = _coreapi_dll.get_pathname(_root_dir);
  if (!copy_file(filename, pathname)) {
    nout << "Couldn't copy " << pathname << "\n";
    set_failed();
    return;
  }

  if (!_coreapi_dll.quick_verify(_root_dir)) {
    nout << "After copying, " << pathname << " is no good.\n";
    set_failed();
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
  string pathname = _coreapi_dll.get_pathname(_root_dir);

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
  string contents_filename = _root_dir + "/contents.xml";
  if (!load_plugin(pathname, contents_filename, PANDA_PACKAGE_HOST_URL,
                   P3D_VC_normal, "", "", "", false, false, 
                   _root_dir, "", nout)) {
    nout << "Unable to launch core API in " << pathname << "\n";
    set_failed();
    return;
  }

  // Format the coreapi_timestamp as a string, for passing as a
  // parameter.
  ostringstream stream;
  stream << _coreapi_dll.get_timestamp();
  string coreapi_timestamp = stream.str();

#ifdef PANDA_OFFICIAL_VERSION
  static const bool official = true;
#else
  static const bool official = false;
#endif
  P3D_set_plugin_version_ptr(P3D_PLUGIN_MAJOR_VERSION, P3D_PLUGIN_MINOR_VERSION,
                             P3D_PLUGIN_SEQUENCE_VERSION, official,
                             PANDA_DISTRIBUTOR,
                             PANDA_PACKAGE_HOST_URL, coreapi_timestamp.c_str(),
                             _coreapi_set_ver.c_str());

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
  if (_started) {
    // Already created.
    return;
  }

  if (!is_plugin_loaded()) {
    // Plugin is not loaded yet.
    return;
  }

#ifdef __APPLE__
  // We no longer need to twirl the icon.  Stop the timer.
  if (_twirl_timer != NULL) {
    CFRunLoopTimerInvalidate(_twirl_timer);
    CFRelease(_twirl_timer);
    _twirl_timer = NULL;
  }
#endif  // __APPLE__

#ifdef HAVE_X11
  x11_stop_twirl_subprocess();
#endif  // HAVE_X11

  // In the Windows case, we let the timer keep running, because it
  // also checks for wayward messages.

  P3D_token *tokens = NULL;
  if (!_tokens.empty()) {
    tokens = &_tokens[0];
  }
  _started = true;
  _p3d_inst = P3D_new_instance_ptr(request_ready, tokens, _tokens.size(), 
                               0, NULL, this);
  if (_p3d_inst == NULL) {
    set_failed();
    return;
  }

  // Now get the browser's toplevel DOM object (called the "window"
  // object in JavaScript), to pass to the plugin.
  NPObject *window_object = NULL;
  if (browser->getvalue(_npp_instance, NPNVWindowNPObject,
                        &window_object) == NPERR_NO_ERROR) {
    PPBrowserObject *pobj = new PPBrowserObject(this, window_object);
    P3D_instance_set_browser_script_object_ptr(_p3d_inst, pobj);
    browser->releaseobject(window_object);
  } else {
    nout << "Couldn't get window_object\n";
  }
  
  if (_script_object != NULL) {
    // Now that we have a true instance, initialize our
    // script_object with the proper P3D_object pointer.
    P3D_object *main = P3D_instance_get_panda_script_object_ptr(_p3d_inst);
    nout << "new instance, setting main = " << main << "\n";
    _script_object->set_main(main);
  }

  if (_got_instance_url) {
    // Create the user_id for streaming the p3d data into the instance.
    _p3d_instance_id = P3D_instance_start_stream_ptr(_p3d_inst, _instance_url.c_str());
    nout << "p3d instance to stream " << _p3d_instance_id << "\n";

    // If we have already started to receive any instance data, send it
    // to the plugin now.
    if (_p3d_temp_file._opened) {
      send_p3d_temp_file_data();
    }
  }
  
  if (_got_window) {
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
  memset(&parent_window, 0, sizeof(parent_window));
  parent_window._window_handle_type = P3D_WHT_none;

  if (_window.type == NPWindowTypeWindow) {
    // We have a "windowed" plugin.  Parent our window to the one we
    // were given.  In this case, we should also reset the offset to
    // (0, 0), since the window we were given is already placed in the
    // right spot.
#ifdef _WIN32
    assert(!_use_xembed);
    parent_window._window_handle_type = P3D_WHT_win_hwnd;
    parent_window._handle._win_hwnd._hwnd = (HWND)(_window.window);
    x = 0;
    y = 0;

#elif defined(__APPLE__)
    assert(!_use_xembed);
    parent_window._window_handle_type = _window_handle_type;
    if (_window_handle_type == P3D_WHT_osx_port) {
#if !__LP64__
      NP_Port *port = (NP_Port *)_window.window;
      parent_window._handle._osx_port._port = port->port;
#endif
    } else if (_window_handle_type == P3D_WHT_osx_cgcontext) {
      NP_CGContext *context = (NP_CGContext *)_window.window;
      if (context != NULL) {
        parent_window._handle._osx_cgcontext._context = context->context;
        parent_window._handle._osx_cgcontext._window = (WindowRef)context->window;
      }
    }

#elif defined(HAVE_X11)
    if (_use_xembed) {
      // If we're using the XEmbed model, we've actually received an
      // XID for a GtkSocket.
#ifdef HAVE_GTK
      // If we're using XEmbed, pass the X11 Window pointer of our
      // plug down to Panda.  (Hmm, it would be nice to pass the XID
      // object and use this system in general within Panda, but
      // that's for the future, I think.)
      assert(_plug != NULL);
      parent_window._window_handle_type = P3D_WHT_x11_window;
      parent_window._handle._x11_window._xwindow = GDK_DRAWABLE_XID(_plug->window);
#endif  // HAVE_GTK
    } else {
      // If we're not using XEmbed, this is just a standard X11 Window
      // pointer.
      parent_window._window_handle_type = P3D_WHT_x11_window;
      parent_window._handle._x11_window._xwindow = (X11_Window)(_window.window);
    }
    x = 0;
    y = 0;
#endif

  } else {
    // We have a "windowless" plugin.  Parent our window directly to
    // the browser window.
#ifdef _WIN32
    HWND hwnd;
    if (browser->getvalue(_npp_instance, NPNVnetscapeWindow,
                          &hwnd) == NPERR_NO_ERROR) {
      parent_window._window_handle_type = P3D_WHT_win_hwnd;
      parent_window._handle._win_hwnd._hwnd = hwnd;
    }

#elif defined(__APPLE__)
    parent_window._window_handle_type = _window_handle_type;
    if (_window_handle_type == P3D_WHT_osx_port) {
#if !__LP64__
      NP_Port *port = (NP_Port *)_window.window;
      parent_window._handle._osx_port._port = port->port;
#endif
    } else if (_window_handle_type == P3D_WHT_osx_cgcontext) {
      NP_CGContext *context = (NP_CGContext *)_window.window;
      if (context != NULL) {
        parent_window._handle._osx_cgcontext._context = context->context;
        parent_window._handle._osx_cgcontext._window = (WindowRef)context->window;
      }
    }

#elif defined(HAVE_X11)
    unsigned long win;
    if (browser->getvalue(_npp_instance, NPNVnetscapeWindow,
                          &win) == NPERR_NO_ERROR) {
      parent_window._window_handle_type = P3D_WHT_x11_window;
      parent_window._handle._x11_window._xwindow = win;
    }
#endif
  }

#ifdef HAVE_X11
  // In the case of X11, grab the display as well.
  parent_window._handle._x11_window._xdisplay = 0;
  void *disp;
  if (browser->getvalue(_npp_instance, NPNVxDisplay,
                        &disp) == NPERR_NO_ERROR) {
    parent_window._handle._x11_window._xdisplay = disp;
  }
#endif

  P3D_window_type window_type = P3D_WT_embedded;
  if (_window.window == NULL && _event_type != P3D_ET_osx_cocoa) {
    // No parent window: it must be a hidden window.
    window_type = P3D_WT_hidden;
  } else if (_window.width == 0 || _window.height == 0) {
    // No size: hidden.
    window_type = P3D_WT_hidden;
  }    

  P3D_instance_setup_window_ptr
    (_p3d_inst, window_type,
     x, y, _window.width, _window.height,
     &parent_window);
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
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, _orig_window_proc);
    InvalidateRect(hwnd, NULL, true);

    if (_bg_brush != NULL) {
      DeleteObject(_bg_brush);
      _bg_brush = NULL;
    }
    for (int step = 0; step < twirl_num_steps + 1; ++step) {
      if (_twirl_bitmaps[step] != NULL) {
        DeleteObject(_twirl_bitmaps[step]);
        _twirl_bitmaps[step] = NULL;
      }
    }
#endif  // _WIN32

#ifdef HAVE_X11
    x11_stop_twirl_subprocess();

#ifdef HAVE_GTK
    if (_plug != NULL) {
      gtk_widget_destroy(_plug);
      _plug = NULL;
    }
#endif  // HAVE_GTK
#endif  // HAVE_X11

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
//     Function: PPInstance::lookup_token
//       Access: Private
//  Description: Returns the value associated with the first
//               appearance of the named token, or empty string if the
//               token does not appear.
////////////////////////////////////////////////////////////////////
string PPInstance::
lookup_token(const string &keyword) const {
  Tokens::const_iterator ti;
  for (ti = _tokens.begin(); ti != _tokens.end(); ++ti) {
    if (keyword == (*ti)._keyword) {
      return (*ti)._value;
    }
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::has_token
//       Access: Private
//  Description: Returns true if the named token appears in the list,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PPInstance::
has_token(const string &keyword) const {
  Tokens::const_iterator ti;
  for (ti = _tokens.begin(); ti != _tokens.end(); ++ti) {
    if ((*ti)._keyword == keyword) {
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: PPInstance::compare_seq
//       Access: Private, Static
//  Description: Compares the two dotted-integer sequence values
//               numerically.  Returns -1 if seq_a sorts first, 1 if
//               seq_b sorts first, 0 if they are equivalent.
////////////////////////////////////////////////////////////////////
int PPInstance::
compare_seq(const string &seq_a, const string &seq_b) {
  const char *num_a = seq_a.c_str();
  const char *num_b = seq_b.c_str();
  int comp = compare_seq_int(num_a, num_b);
  while (comp == 0) {
    if (*num_a != '.') {
      if (*num_b != '.') {
        // Both strings ran out together.
        return 0;
      }
      // a ran out first.
      return -1;
    } else if (*num_b != '.') {
      // b ran out first.
      return 1;
    }

    // Increment past the dot.
    ++num_a;
    ++num_b;
    comp = compare_seq(num_a, num_b);
  }

  return comp;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::compare_seq_int
//       Access: Private, Static
//  Description: Numerically compares the formatted integer value at
//               num_a with num_b.  Increments both num_a and num_b to
//               the next character following the valid integer.
////////////////////////////////////////////////////////////////////
int PPInstance::
compare_seq_int(const char *&num_a, const char *&num_b) {
  long int a;
  char *next_a;
  long int b;
  char *next_b;

  a = strtol((char *)num_a, &next_a, 10);
  b = strtol((char *)num_b, &next_b, 10);

  num_a = next_a;
  num_b = next_b;

  if (a < b) {
    return -1;
  } else if (b < a) {
    return 1;
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::set_failed
//       Access: Private
//  Description: Called when something has gone wrong that prevents
//               the plugin instance from running.  Specifically, this
//               means it failed to load the core API.
////////////////////////////////////////////////////////////////////
void PPInstance::
set_failed() {
  if (!_failed) {
    _failed = true;

    nout << "Plugin failed.\n";
    stop_outstanding_streams();

    // Look for the "onpluginfail" token.
    string expression = lookup_token("onpluginfail");

    if (!expression.empty()) {
      // Now attempt to evaluate the expression.
      NPObject *window_object = NULL;
      if (browser->getvalue(_npp_instance, NPNVWindowNPObject,
                            &window_object) == NPERR_NO_ERROR) {
        NPString npexpr = { expression.c_str(), (uint32_t)expression.length() };
        NPVariant result;
        if (browser->evaluate(_npp_instance, window_object,
                              &npexpr, &result)) {
          nout << "Eval " << expression << "\n";
          browser->releasevariantvalue(&result);
        } else {
          nout << "Unable to eval " << expression << "\n";
        }

        browser->releaseobject(window_object);
      }
    }

    if (_p3d_inst != NULL) {
      P3D_instance_finish_ptr(_p3d_inst);
      _p3d_inst = NULL;
    }
    cleanup_window();
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

  P3D_instance *p3d_inst = P3D_check_request_ptr(0.0);
  while (p3d_inst != (P3D_instance *)NULL) {
    P3D_request *request = P3D_instance_get_request_ptr(p3d_inst);
    if (request != (P3D_request *)NULL) {
      PPInstance *inst = (PPInstance *)(p3d_inst->_user_data);
      assert(inst != NULL);
      if (!inst->handle_request(request)) {
        // If handling the request is meant to yield control
        // temporarily to JavaScript (e.g. P3D_RT_callback), then do
        // so now.
        return;
      }

      if (!is_plugin_loaded()) {
        // Oops, we may have unloaded the plugin as an indirect effect
        // of handling the request.  If so, get out of here.
        return;
      }
    }
    p3d_inst = P3D_check_request_ptr(0.0);
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
//     Function: PPInstance::st_window_proc
//       Access: Private, Static
//  Description: We bind this function to the parent window we were
//               given in set_window, so we can spin the request_loop
//               when needed.  This is only in the Windows case; other
//               platforms rely on explicit windows events.
////////////////////////////////////////////////////////////////////
LONG PPInstance::
st_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  LONG_PTR self = GetWindowLongPtr(hwnd, GWLP_USERDATA);
  if (self == NULL) {
    // We haven't assigned the pointer yet (!?)
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }

  return ((PPInstance *)self)->window_proc(hwnd, msg, wparam, lparam);
}
#endif  // _WIN32

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::window_proc
//       Access: Private
//  Description: The non-static window_proc() function.
//
//               We bind this function to the parent window we were
//               given in set_window, so we can spin the request_loop
//               when needed.  This is only in the Windows case; other
//               platforms rely on explicit windows events.
////////////////////////////////////////////////////////////////////
LONG PPInstance::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (!has_plugin_thread_async_call) {
    // Since we're here in the main thread, call handle_request_loop()
    // to see if there are any new requests to be serviced by the main
    // thread.
    handle_request_loop();
  }

  switch (msg) {
  case WM_ERASEBKGND:
    // Eat the WM_ERASEBKGND message, so the browser's intervening
    // window won't overdraw on top of our own window.
    return true;
    
  case WM_PAINT:
    if (!_started) {
      // If we haven't yet loaded the instance, we can paint a
      // twirling icon in the window.
      PAINTSTRUCT ps;
      HDC dc = BeginPaint(hwnd, &ps);
      win_paint_twirl(hwnd, dc);
      EndPaint(hwnd, &ps);
    }
    return true;

  case WM_TIMER:
    if (!_started) {
      InvalidateRect(_hwnd, NULL, FALSE);
    }
    break;

  case WM_USER:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}
#endif  // _WIN32


#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::win_get_twirl_bitmaps
//       Access: Private
//  Description: Fills _twirl_bitmaps with an array of bitmaps for
//               drawing the twirling icon while we're waiting for the
//               instance to load.
////////////////////////////////////////////////////////////////////
void PPInstance::
win_get_twirl_bitmaps() {
  BITMAPINFOHEADER bmih;
  bmih.biSize = sizeof(bmih);
  bmih.biWidth = twirl_width;
  bmih.biHeight = -twirl_height;
  bmih.biPlanes = 1;
  bmih.biBitCount = 32;
  bmih.biCompression = BI_RGB;
  bmih.biSizeImage = 0;
  bmih.biXPelsPerMeter = 0;
  bmih.biYPelsPerMeter = 0;
  bmih.biClrUsed = 0;
  bmih.biClrImportant = 0;

  BITMAPINFO bmi;
  memcpy(&bmi, &bmih, sizeof(bmih));

  HDC dc = GetDC(_hwnd);

  static const size_t twirl_size = twirl_width * twirl_height;
  unsigned char twirl_data[twirl_size * 3];
  unsigned char new_data[twirl_size * 4];

  for (int step = 0; step < twirl_num_steps + 1; ++step) {
    get_twirl_data(twirl_data, twirl_size, step,
                   _fgcolor_r, _fgcolor_g, _fgcolor_b, 
                   _bgcolor_r, _bgcolor_g, _bgcolor_b);

    // Expand out the RGB channels into RGBA.
    for (int yi = 0; yi < twirl_height; ++yi) {
      const unsigned char *sp = twirl_data + yi * twirl_width * 3;
      unsigned char *dp = new_data + yi * twirl_width * 4;
      for (int xi = 0; xi < twirl_width; ++xi) {
        // RGB <= BGR.
        dp[0] = sp[2];
        dp[1] = sp[1];
        dp[2] = sp[0];
        dp[3] = (unsigned char)0xff;
        sp += 3;
        dp += 4;
      }
    }

    // Now load the image.
    _twirl_bitmaps[step] = CreateDIBitmap(dc, &bmih, CBM_INIT, (const char *)new_data, &bmi, 0);
  }

  ReleaseDC(_hwnd, dc);
}
#endif  // _WIN32

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::win_paint_twirl
//       Access: Private
//  Description: Paints the twirling icon into the browser window
//               before the instance has started.
////////////////////////////////////////////////////////////////////
void PPInstance::
win_paint_twirl(HWND hwnd, HDC dc) {
  RECT rect;
  GetClientRect(_hwnd, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  // Double-buffer with an offscreen bitmap first.
  HDC bdc = CreateCompatibleDC(dc);
  HBITMAP buffer = CreateCompatibleBitmap(dc, width, height);
  SelectObject(bdc, buffer);

  // Start by painting the background color.
  FillRect(bdc, &rect, _bg_brush);

  if (!_started) {
    DWORD now = GetTickCount();
    // Don't draw the twirling icon until at least half a second has
    // passed, so we don't distract people by drawing it
    // unnecessarily.
    if (_failed || (now - _init_time) >= 500) {
      // Which frame are we drawing?
      int step = (now / 100) % twirl_num_steps;
      if (_failed) {
        step = twirl_num_steps;
      }
      
      HBITMAP twirl = _twirl_bitmaps[step];
      
      int left = rect.left + (width - twirl_width) / 2;
      int top = rect.top + (height - twirl_height) / 2;
      
      HDC mem_dc = CreateCompatibleDC(bdc);
      SelectObject(mem_dc, twirl);
      
      BitBlt(bdc, left, top, twirl_width, twirl_height,
             mem_dc, 0, 0, SRCCOPY);
      
      SelectObject(mem_dc, NULL);
      DeleteDC(mem_dc);
    }
  }

  // Now blit the buffer to the window.
  BitBlt(dc, 0, 0, width, height, bdc, 0, 0, SRCCOPY);
}
#endif  // _WIN32

#ifdef MACOSX_HAS_EVENT_MODELS
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::copy_cocoa_event
//       Access: Private, Static
//  Description: Copies the NPCocoaEvent structure componentwise into
//               a P3DCocoaEvent structure, for passing into the core
//               API.
//
//               The aux_data object is used to manage temporary
//               storage on the strings created for the event.
////////////////////////////////////////////////////////////////////
void PPInstance::
copy_cocoa_event(P3DCocoaEvent *p3d_event, NPCocoaEvent *np_event,
                 EventAuxData &aux_data) {
  p3d_event->version = np_event->version;

  switch (np_event->type) {
  case NPCocoaEventDrawRect:
    p3d_event->type = P3DCocoaEventDrawRect;
    break;
  case NPCocoaEventMouseDown:
    p3d_event->type = P3DCocoaEventMouseDown;
    break;
  case NPCocoaEventMouseUp:
    p3d_event->type = P3DCocoaEventMouseUp;
    break;
  case NPCocoaEventMouseMoved:
    p3d_event->type = P3DCocoaEventMouseMoved;
    break;
  case NPCocoaEventMouseEntered:
    p3d_event->type = P3DCocoaEventMouseEntered;
    break;
  case NPCocoaEventMouseExited:
    p3d_event->type = P3DCocoaEventMouseExited;
    break;
  case NPCocoaEventMouseDragged:
    p3d_event->type = P3DCocoaEventMouseDragged;
    break;
  case NPCocoaEventKeyDown:
    p3d_event->type = P3DCocoaEventKeyDown;
    break;
  case NPCocoaEventKeyUp:
    p3d_event->type = P3DCocoaEventKeyUp;
    break;
  case NPCocoaEventFlagsChanged:
    p3d_event->type = P3DCocoaEventFlagsChanged;
    break;
  case NPCocoaEventFocusChanged:
    p3d_event->type = P3DCocoaEventFocusChanged;
    break;
  case NPCocoaEventWindowFocusChanged:
    p3d_event->type = P3DCocoaEventWindowFocusChanged;
    break;
  case NPCocoaEventScrollWheel:
    p3d_event->type = P3DCocoaEventScrollWheel;
    break;
  case NPCocoaEventTextInput:
    p3d_event->type = P3DCocoaEventTextInput;
    break;
  }

  switch (np_event->type) {
  case NPCocoaEventDrawRect:
    p3d_event->data.draw.context = np_event->data.draw.context;
    p3d_event->data.draw.x = np_event->data.draw.x;
    p3d_event->data.draw.y = np_event->data.draw.y;
    p3d_event->data.draw.width = np_event->data.draw.width;
    p3d_event->data.draw.height = np_event->data.draw.height;
    break;

  case NPCocoaEventMouseDown:
  case NPCocoaEventMouseUp:
  case NPCocoaEventMouseMoved:
  case NPCocoaEventMouseEntered:
  case NPCocoaEventMouseExited:
  case NPCocoaEventMouseDragged:
  case NPCocoaEventScrollWheel:
    p3d_event->data.mouse.modifierFlags = np_event->data.mouse.modifierFlags;
    p3d_event->data.mouse.pluginX = np_event->data.mouse.pluginX;
    p3d_event->data.mouse.pluginY = np_event->data.mouse.pluginY;
    p3d_event->data.mouse.buttonNumber = np_event->data.mouse.buttonNumber;
    p3d_event->data.mouse.clickCount = np_event->data.mouse.clickCount;
    p3d_event->data.mouse.deltaX = np_event->data.mouse.deltaX;
    p3d_event->data.mouse.deltaY = np_event->data.mouse.deltaY;
    p3d_event->data.mouse.deltaZ = np_event->data.mouse.deltaZ;
    break;

  case NPCocoaEventKeyDown:
  case NPCocoaEventKeyUp:
  case NPCocoaEventFlagsChanged:
    p3d_event->data.key.modifierFlags = np_event->data.key.modifierFlags;
    p3d_event->data.key.characters = 
      make_ansi_string(aux_data._characters, np_event->data.key.characters);
    p3d_event->data.key.charactersIgnoringModifiers = 
      make_ansi_string(aux_data._characters_im, np_event->data.key.charactersIgnoringModifiers);
    p3d_event->data.key.isARepeat = np_event->data.key.isARepeat;
    p3d_event->data.key.keyCode = np_event->data.key.keyCode;
    break;

  case NPCocoaEventFocusChanged:
  case NPCocoaEventWindowFocusChanged:
    p3d_event->data.focus.hasFocus = np_event->data.focus.hasFocus;
    break;

  case NPCocoaEventTextInput:
    p3d_event->data.text.text = 
      make_ansi_string(aux_data._text, np_event->data.text.text);
    break;
  }
}
#endif  // MACOSX_HAS_EVENT_MODELS

#ifdef MACOSX_HAS_EVENT_MODELS
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::make_ansi_string
//       Access: Private, Static
//  Description: OSX only: Fills result with the unicode characters in
//               the NPNSString.  Also returns result.c_str().
////////////////////////////////////////////////////////////////////
const wchar_t *PPInstance::
make_ansi_string(wstring &result, NPNSString *ns_string) {
  result.clear();
  
  if (ns_string != NULL) {
    // An NPNSString is really just an NSString, which is itself just a
    // CFString.
    CFStringRef cfstr = (CFStringRef)ns_string;
    CFIndex length = CFStringGetLength(cfstr);
    
    for (CFIndex i = 0; i < length; ++i) {
      result += (wchar_t)CFStringGetCharacterAtIndex(cfstr, i);
    }
  }

  return result.c_str();
}
#endif  // MACOSX_HAS_EVENT_MODELS

#ifdef MACOSX_HAS_EVENT_MODELS
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::handle_cocoa_event
//       Access: Private
//  Description: Locally processes a Cocoa event for the window before
//               sending it down to the Core API.  This is used for
//               drawing a twirling icon in the window while the Core
//               API is downloading.
////////////////////////////////////////////////////////////////////
void PPInstance::
handle_cocoa_event(const P3DCocoaEvent *p3d_event) {
  switch (p3d_event->type) {
  case P3DCocoaEventDrawRect:
    if (!_started) {
      CGContextRef context = p3d_event->data.draw.context;
      paint_twirl_osx_cgcontext(context);
    }
    break;
    
  default:
    break;
  }
}
#endif  // MACOSX_HAS_EVENT_MODELS

#ifdef MACOSX_HAS_EVENT_MODELS
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::osx_get_twirl_images
//       Access: Private
//  Description: Fills _twirl_images with an array of images for
//               drawing the twirling icon while we're waiting for the
//               instance to load.
////////////////////////////////////////////////////////////////////
void PPInstance::
osx_get_twirl_images() {
  if (_got_twirl_images) {
    return;
  }
  _got_twirl_images = true;

  static const size_t twirl_size = twirl_width * twirl_height;
  unsigned char twirl_data[twirl_size * 3];

  for (int step = 0; step < twirl_num_steps + 1; ++step) {
    get_twirl_data(twirl_data, twirl_size, step,
                   _fgcolor_r, _fgcolor_g, _fgcolor_b, 
                   _bgcolor_r, _bgcolor_g, _bgcolor_b);

    unsigned char *new_data = new unsigned char[twirl_size * 4];

    // Expand the RGB channels into RGBA.  Flip upside-down too.
    for (int yi = 0; yi < twirl_height; ++yi) {
      const unsigned char *sp = twirl_data + (twirl_height - 1 - yi) * twirl_width * 3;
      unsigned char *dp = new_data + yi * twirl_width * 4;
      for (int xi = 0; xi < twirl_width; ++xi) {
        // RGB <= BGR.
        dp[0] = sp[2];
        dp[1] = sp[1];
        dp[2] = sp[0];
        dp[3] = (unsigned char)0xff;
        sp += 3;
        dp += 4;
      }
    }

    OsxImageData &image = _twirl_images[step];
    image._raw_data = new_data;

    image._data =
      CFDataCreateWithBytesNoCopy(NULL, (const UInt8 *)image._raw_data, 
                                  twirl_size * 4, kCFAllocatorNull);
    image._provider = CGDataProviderCreateWithCFData(image._data);
    image._color_space = CGColorSpaceCreateDeviceRGB();
    
    image._image =
      CGImageCreate(twirl_width, twirl_height, 8, 32, 
                    twirl_width * 4, image._color_space,
                    kCGImageAlphaFirst | kCGBitmapByteOrder32Little, 
                    image._provider, NULL, false, kCGRenderingIntentDefault);
  }
}
#endif  // MACOSX_HAS_EVENT_MODELS

#ifdef MACOSX_HAS_EVENT_MODELS
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::osx_release_twirl_images
//       Access: Private
//  Description: Frees the twirl_images array.
////////////////////////////////////////////////////////////////////
void PPInstance::
osx_release_twirl_images() {
  if (!_got_twirl_images) {
    return;
  }
  _got_twirl_images = false;

  for (int step = 0; step < twirl_num_steps + 1; ++step) {
    OsxImageData &image = _twirl_images[step];

    if (image._image != NULL) {
      CGImageRelease(image._image);
      image._image = NULL;
    }
    if (image._color_space != NULL) {
      CGColorSpaceRelease(image._color_space);
      image._color_space = NULL;
    }
    if (image._provider != NULL) {
      CGDataProviderRelease(image._provider);
      image._provider = NULL;
    }
    if (image._data != NULL) {
      CFRelease(image._data);
      image._data = NULL;
    }
    if (image._raw_data != NULL) {
      delete[] image._raw_data;
      image._raw_data = NULL;
    }
  }
}
#endif  // MACOSX_HAS_EVENT_MODELS

#ifdef MACOSX_HAS_EVENT_MODELS
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::paint_twirl_osx_cgcontext
//       Access: Private
//  Description: Actually paints the twirling icon in the OSX window,
//               using Core Graphics.  (We don't bother painting it in
//               the older Cocoa interface.)
////////////////////////////////////////////////////////////////////
void PPInstance::
paint_twirl_osx_cgcontext(CGContextRef context) {
  // Clear the whole region to the bgcolor before beginning.
  float bg_components[] = { _bgcolor_r / 255.0f, _bgcolor_g / 255.0f, _bgcolor_b / 255.0f, 1 };
  CGColorSpaceRef rgb_space = CGColorSpaceCreateDeviceRGB();
  CGColorRef bg = CGColorCreate(rgb_space, bg_components);

  CGRect region = { { 0, 0 }, { _window.width, _window.height } };
  CGContextBeginPath(context);
  CGContextSetFillColorWithColor(context, bg);
  CGContextAddRect(context, region);
  CGContextFillPath(context);

  CGColorRelease(bg);
  CGColorSpaceRelease(rgb_space);

  if (_failed) {
    // Draw the failed icon if something went wrong.
    osx_paint_image(context, _twirl_images[twirl_num_steps]);

  } else {
    struct timeval tv;
    gettimeofday(&tv, (struct timezone *)NULL);
    double now = (double)(tv.tv_sec - _init_sec) + (double)(tv.tv_usec - _init_usec) / 1000000.0;
    
    // Don't draw the twirling icon until at least half a second has
    // passed, so we don't distract people by drawing it
    // unnecessarily.
    if (now >= 0.5) {
      int step = ((int)(now * 10.0)) % twirl_num_steps;
      osx_paint_image(context, _twirl_images[step]);
    }
  }
}
#endif  // MACOSX_HAS_EVENT_MODELS

#ifdef MACOSX_HAS_EVENT_MODELS
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::osx_paint_image
//       Access: Private
//  Description: Draws the indicated image, centered within the
//               window.  Returns true on success, false if the image
//               is not defined.
////////////////////////////////////////////////////////////////////
bool PPInstance::
osx_paint_image(CGContextRef context, const OsxImageData &image) {
  if (image._image == NULL) {
    return false;
  }
    
  // Determine the relative size of image and window.
  int win_cx = _window.width / 2;
  int win_cy = _window.height / 2;

  CGRect rect = { { 0, 0 }, { 0, 0 } };
    
  // The bitmap fits within the window; center it.
      
  // This is the top-left corner of the bitmap in window coordinates.
  int p_x = win_cx - twirl_width / 2;
  int p_y = win_cy - twirl_height / 2;
  
  rect.origin.x += p_x;
  rect.origin.y += p_y;
  rect.size.width = twirl_width;
  rect.size.height = twirl_height;

  CGContextDrawImage(context, rect, image._image);
  
  return true;
}
#endif  // MACOSX_HAS_EVENT_MODELS

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::timer_callback
//       Access: Private, Static
//  Description: OSX only: this callback is associated with a
//               CFRunLoopTimer; it's used to forward request messages
//               to the main thread.
////////////////////////////////////////////////////////////////////
void PPInstance::
timer_callback(CFRunLoopTimerRef timer, void *info) {
  PPInstance *self = (PPInstance *)info;
  ACQUIRE_LOCK(self->_timer_lock);
  if (self->_request_timer != NULL) {
    CFRunLoopTimerInvalidate(self->_request_timer);
    CFRelease(self->_request_timer);
    self->_request_timer = NULL;
  }
  RELEASE_LOCK(self->_timer_lock);

  self->handle_request_loop();
}
#endif  // __APPLE__

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::st_twirl_timer_callback
//       Access: Private, Static
//  Description: OSX only: this callback is used to twirl the icon
//               before the instance loads.
////////////////////////////////////////////////////////////////////
void PPInstance::
st_twirl_timer_callback(CFRunLoopTimerRef timer, void *info) {
  PPInstance *self = (PPInstance *)info;
  self->twirl_timer_callback();
}
#endif  // __APPLE__

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::twirl_timer_callback
//       Access: Private
//  Description: OSX only: this callback is used to twirl the icon
//               before the instance loads.
////////////////////////////////////////////////////////////////////
void PPInstance::
twirl_timer_callback() {
  if (_got_window) {
    NPRect rect = { 0, 0, (unsigned short)_window.height, (unsigned short)_window.width };
    browser->invalidaterect(_npp_instance, &rect);
  }
}
#endif  // __APPLE__

#ifdef HAVE_X11
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::x11_start_twirl_subprocess
//       Access: Public
//  Description: Spawns a separate process to twirl the loading icon
//               in the X11 browser window.
////////////////////////////////////////////////////////////////////
void PPInstance::
x11_start_twirl_subprocess() {
  if (_twirl_subprocess_pid != -1) {
    // Already started.
    return;
  }

  // Fork and exec.
  pid_t child = fork();
  if (child < 0) {
    perror("fork");
    return;
  }

  if (child == 0) {
    // Here we are in the child process.
    x11_twirl_subprocess_run();
    _exit(99);
  }

  // In the parent process.
  _twirl_subprocess_pid = child;
  nout << "Started twirl subprocess, pid " << _twirl_subprocess_pid
       << "\n";
}
#endif  // HAVE_X11

#ifdef HAVE_X11
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::x11_stop_twirl_subprocess
//       Access: Public
//  Description: Kills the twirl process that was started earlier.
////////////////////////////////////////////////////////////////////
void PPInstance::
x11_stop_twirl_subprocess() {
  if (_twirl_subprocess_pid == -1) {
    // Already stopped.
    return;
  }

  kill(_twirl_subprocess_pid, SIGKILL);

  int status;
  pid_t result = waitpid(_twirl_subprocess_pid, &status, 0);

  nout << "Twirl window process has successfully stopped.\n";
  if (WIFEXITED(status)) {
    nout << "  exited normally, status = "
         << WEXITSTATUS(status) << "\n";
  } else if (WIFSIGNALED(status)) {
    nout << "  signalled by " << WTERMSIG(status) << ", core = " 
         << WCOREDUMP(status) << "\n";
  } else if (WIFSTOPPED(status)) {
    nout << "  stopped by " << WSTOPSIG(status) << "\n";
  }
  _twirl_subprocess_pid = -1;
}
#endif  // HAVE_X11

#ifdef HAVE_X11
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::x11_twirl_subprocess_run
//       Access: Public
//  Description: The code that is run within a subprocess.  This code
//               is responsible for twirling the loading icon
//               endlessly.
////////////////////////////////////////////////////////////////////
void PPInstance::
x11_twirl_subprocess_run() {
  // Since everything within this function happens within a subprocess
  // that will just exit, we can be a little sloppy with our resource
  // allocation.  It is all done directly within this function, and we
  // don't need to worry about freeing stuff.

  // First, sleep for 0.5 seconds, so we don't start twirling right
  // away (to avoid distracting the user unnecessarily).

  struct timespec req;
  req.tv_sec = 0;
  req.tv_nsec = 500000000;  // 500 ms
  nanosleep(&req, NULL);

  // We haven't been killed yet, so the plugin is still loading.
  // Start twirling.

  // First, embed a window.
  X11_Display *display = XOpenDisplay(NULL);
  assert(display != NULL);
  int screen = DefaultScreen(display);

  int depth = DefaultDepth(display, screen);
  Visual *dvisual = DefaultVisual(display, screen);

  long event_mask = ExposureMask;

  // Allocate the foreground and background colors.
  Colormap colormap = DefaultColormap(display, screen);

  XColor fg;
  fg.red = _fgcolor_r * 0x101;
  fg.green = _fgcolor_g * 0x101;
  fg.blue = _fgcolor_b * 0x101;
  fg.flags = DoRed | DoGreen | DoBlue;
  unsigned long fg_pixel = -1;
  if (XAllocColor(display, colormap, &fg)) {
    fg_pixel = fg.pixel;
  }

  XColor bg;
  bg.red = _bgcolor_r * 0x101;
  bg.green = _bgcolor_g * 0x101;
  bg.blue = _bgcolor_b * 0x101;
  bg.flags = DoRed | DoGreen | DoBlue;
  unsigned long bg_pixel = -1;
  if (XAllocColor(display, colormap, &bg)) {
    bg_pixel = bg.pixel;
  }

  // Initialize window attributes
  XSetWindowAttributes wa;
  wa.background_pixel = XWhitePixel(display, screen);
  if (bg_pixel != -1) {
    wa.background_pixel = bg_pixel;
  }
  wa.border_pixel = 0;
  wa.event_mask = event_mask;

  unsigned long attrib_mask = CWBackPixel | CWBorderPixel | CWEventMask;

  X11_Window parent = 0;
  if (_use_xembed) {
#ifdef HAVE_GTK
    assert(_plug != NULL);
    parent = GDK_DRAWABLE_XID(_plug->window);
#endif  // HAVE_GTK
  } else {
    parent = (X11_Window)(_window.window);
  }

  X11_Window window = XCreateWindow
    (display, parent, 0, 0, _window.width, _window.height,
     0, depth, InputOutput, dvisual, attrib_mask, &wa);
  XMapWindow(display, window);

  // Create a graphics context.
  XGCValues gcval;
  gcval.function = GXcopy;
  gcval.plane_mask = AllPlanes;
  gcval.foreground = BlackPixel(display, screen);
  if (fg_pixel != -1) {
    gcval.foreground = fg_pixel;
  }
  gcval.background = WhitePixel(display, screen);
  if (bg_pixel != -1) {
    gcval.background = bg_pixel;
  }
  GC graphics_context = XCreateGC(display, window, 
    GCFunction | GCPlaneMask | GCForeground | GCBackground, &gcval); 

  // Load up the twirling images.
  XImage *images[twirl_num_steps];
  double r_ratio = dvisual->red_mask / 255.0;
  double g_ratio = dvisual->green_mask / 255.0;
  double b_ratio = dvisual->blue_mask / 255.0;

  static const size_t twirl_size = twirl_width * twirl_height;
  unsigned char twirl_data[twirl_size * 3];

  for (int step = 0; step < twirl_num_steps; ++step) {
    get_twirl_data(twirl_data, twirl_size, step,
                   _fgcolor_r, _fgcolor_g, _fgcolor_b, 
                   _bgcolor_r, _bgcolor_g, _bgcolor_b);
    uint32_t *new_data = new uint32_t[twirl_size];
    int j = 0;
    for (int i = 0; i < twirl_size * 3; i += 3) {
      unsigned int r, g, b;
      r = (unsigned int)(twirl_data[i+0] * r_ratio);
      g = (unsigned int)(twirl_data[i+1] * g_ratio);
      b = (unsigned int)(twirl_data[i+2] * b_ratio);
      new_data[j++] = ((r & dvisual->red_mask) |
                       (g & dvisual->green_mask) |
                       (b & dvisual->blue_mask));
    }

    // Now load the image.
    images[step] = XCreateImage(display, CopyFromParent, DefaultDepth(display, screen), 
                                ZPixmap, 0, (char *)new_data, twirl_width, twirl_height, 32, 0);
  }

  // Now twirl indefinitely, until our parent process kills us.
  bool needs_redraw = true;
  int last_step = -1;
  while (true) {
    // First, scan for X events.
    XEvent event;
    while (XCheckWindowEvent(display, window, ~0, &event)) {
      switch (event.type) {
      case Expose:
      case GraphicsExpose:
        needs_redraw = true;
        break;
      }

      // We should probably track the resize event, but this window
      // will be short-lived (and probably won't have an opportunity
      // to resize anyway) so we don't bother.
    }

    // What step are we on now?
    struct timeval tv;
    gettimeofday(&tv, (struct timezone *)NULL);
    double now = (double)(tv.tv_sec - _init_sec) + (double)(tv.tv_usec - _init_usec) / 1000000.0;
    int step = ((int)(now * 10.0)) % twirl_num_steps;
    if (step != last_step) {
      needs_redraw = true;
    }

    if (needs_redraw) {
      XClearWindow(display, window);
      int xo = (_window.width - twirl_width) / 2;
      int yo = (_window.height - twirl_height) / 2;
      XPutImage(display, window, graphics_context, images[step], 0, 0, 
                xo, yo, twirl_width, twirl_height);

      XFlush(display);
      needs_redraw = false;
      last_step = step;
    }

    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 100000000;  // 100 ms
    nanosleep(&req, NULL);
  }
}
#endif  // HAVE_X11

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
    bool download_ok = P3D_instance_feed_url_stream_ptr
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

  P3D_instance_feed_url_stream_ptr
    (_p3d_inst, _user_id, result, 0, _total_count, NULL, 0);

  // All done.
  _thread_done = true;
}


////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamTempFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPInstance::StreamTempFile::
StreamTempFile() {
  _opened = false;
  _finished = false;
  _current_size = 0;
  _total_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamTempFile::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPInstance::StreamTempFile::
~StreamTempFile() {
  cleanup();
}
    
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamTempFile::open
//       Access: Public
//  Description: Creates the temp file and prepares to write to it.
//               It is not normally necessary to call this explicitly;
//               it will be called automatically on the first call to
//               feed().
////////////////////////////////////////////////////////////////////
void PPInstance::StreamTempFile::
open() {
  assert(!_opened);
  _opened = true;
  _finished = false;
  _current_size = 0;
  _total_size = 0;

  char *name = tempnam(NULL, "p3d_");
  _filename = name;
  free(name);

  _stream.clear();
  _stream.open(_filename.c_str(), ios::binary);
  if (!_stream) {
    nout << "Unable to open temp file " << _filename << "\n";
  } else {
    nout << "Opening " << _filename << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamTempFile::feed
//       Access: Public
//  Description: Receives new data from the URL and writes it to the
//               temp file.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool PPInstance::StreamTempFile::
feed(size_t total_expected_data, const void *this_data,
     size_t this_data_size) {
  if (_finished) {
    nout << "feed(" << total_expected_data << ", " << (void *)this_data
         << ", " << this_data_size << ") to " << _filename
         << ", but already finished at " << _current_size << "\n";
    return false;
  }

  if (!_opened) {
    open();
  }

  _stream.write((const char *)this_data, this_data_size);
  _current_size += this_data_size;
  _total_size = total_expected_data;

  if (!_stream) {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamTempFile::finish
//       Access: Public
//  Description: Marks the end of the data received from the URL.  The
//               file is closed but not yet deleted; it remains on
//               disk and may be read at leisure.
////////////////////////////////////////////////////////////////////
void PPInstance::StreamTempFile::
finish() {
  if (!_finished) {
    _finished = true;
    _total_size = _current_size;
  }

  _stream.close();
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamTempFile::close
//       Access: Public
//  Description: Closes the stream for more data.  The file is not yet
//               deleted; it remains on disk and may be read at
//               leisure.
////////////////////////////////////////////////////////////////////
void PPInstance::StreamTempFile::
close() {
  _stream.close();
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::StreamTempFile::cleanup
//       Access: Public
//  Description: Closes all open processes and removes the temp file
//               from disk.
////////////////////////////////////////////////////////////////////
void PPInstance::StreamTempFile::
cleanup() {
  finish();

  if (!_filename.empty()) {
    nout << "Deleting " << _filename << "\n";
    unlink(_filename.c_str());
    _filename.clear();
  }

  _opened = false;
  _finished = false;
}
