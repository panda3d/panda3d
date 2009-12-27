// Filename: panda3dBase.cxx
// Created by:  pro-rsoft (07Dec09)
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

#include "panda3dBase.h"
#include "httpClient.h"
#include "find_root_dir.h"
#include "p3d_plugin_config.h"
#include "executionEnvironment.h"

// We can include this header file to get the DTOOL_PLATFORM
// definition, even though we don't link with dtool.
#include "dtool_platform.h"
#include "pandaVersion.h"

#include <ctype.h>
#include <sstream>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

// The amount of time in seconds to wait for new messages.
static const double wait_cycle = 0.2;

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Panda3DBase::
Panda3DBase(bool console_environment) {
  _console_environment = console_environment;

  _root_dir = find_root_dir();
  _reporting_download = false;
  _enable_security = false;

  _window_type = P3D_WT_toplevel;

  _win_x = -1;
  _win_y = -1;
  _win_width = 640;
  _win_height = 480;

  _exit_with_last_instance = true;
  _host_url = PANDA_PACKAGE_HOST_URL;
  _this_platform = DTOOL_PLATFORM;
  _verify_contents = false;

  // Seed the lame random number generator in rand(); we use it to
  // select a mirror for downloading.
  srand((unsigned int)time(NULL));
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::run_embedded
//       Access: Public
//  Description: Runs with the data embedded in the current
//               executable, at the specified offset.
////////////////////////////////////////////////////////////////////
int Panda3DBase::
run_embedded(int read_offset, int argc, char *argv[]) {
  // Make sure the splash window will be put in the center of the screen
  _win_x = -2;
  _win_y = -2;
  
  // Read out some parameters from the binary
  pifstream read;
  Filename f = ExecutionEnvironment::get_binary_name();
  f.make_absolute();
  f.set_binary();
  if (!f.open_read(read)) {
	  cerr << "Failed to read from stream. Maybe the binary is corrupt?\n";
	  return 1;
  }
  read.seekg(read_offset);
  char curchr = 1;
  string curstr;
  bool havenull = false;
  P3D_token token;
  token._keyword = NULL;
  token._value = NULL;
  string keyword;
  string value;
  string root_dir;
  while (true) {
    curchr = read.get();
    if (curchr == 0) {
      // Two null bytes in a row means we've reached the end of the data.
      if (havenull) {
        break;
      }
      
      // This means we haven't seen an '=' character yet.
      if (keyword == "") {
        if (curstr != "") {
          cerr << "Ignoring token '" << curstr << "' without value\n";
        }
      } else {
        value.assign(curstr);
        P3D_token token;
        token._keyword = keyword.c_str();
        token._value = value.c_str();
        _tokens.push_back(token);
        
        // Read out the tokens that may interest us
        if (keyword == "width") {
          _win_width = atoi(value.c_str());
        } else if (keyword == "height") {
          _win_height = atoi(value.c_str());
        } else if (keyword == "root_dir") {
          root_dir = value;
        }
      }
      curstr = "";
      havenull = true;
    } else if (curchr == '=') {
      keyword.assign(curstr);
      curstr = "";
      havenull = false;
    } else {
      curstr += curchr;
      havenull = false;
    }
  }
  // Update the offset to the current read pointer.
  // This is where the multifile really starts.
  read_offset = read.tellg();
  read.close();
  
  // Initialize the plugin
  if (!P3D_initialize(P3D_API_VERSION, NULL,
                      _host_url.c_str(), _verify_contents, _this_platform.c_str(),
                      _log_dirname.c_str(), _log_basename.c_str(),
                      !_enable_security, _console_environment,
                      root_dir.c_str())) {
    // Oops, failure to initialize.
    cerr << "Failed to initialize plugin (wrong API version?)\n";
    return 1;
  }
  
  // Create a plugin instance and run the program
  P3D_instance *inst = create_instance
    (f, true, _win_x, _win_y, _win_width, _win_height,
     argv, argc, read_offset);
  _instances.insert(inst);
  
  run_main_loop();
  P3D_finalize();
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::run_main_loop
//       Access: Public
//  Description: Gets lost in the application main loop, waiting for
//               system events and notifications from the open
//               instance(s).
////////////////////////////////////////////////////////////////////
void Panda3DBase::
run_main_loop() {
#ifdef _WIN32
  if (_window_type == P3D_WT_embedded) {
    // Wait for new messages from Windows, and new requests from the
    // plugin.
    MSG msg;
    int retval;
    retval = GetMessage(&msg, NULL, 0, 0);
    while (retval != 0 && !time_to_exit()) {
      if (retval == -1) {
        cerr << "Error processing message queue.\n";
        return;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);

      // Check for new requests from the Panda3D plugin.
      P3D_instance *inst = P3D_check_request(wait_cycle);
      while (inst != (P3D_instance *)NULL) {
        P3D_request *request = P3D_instance_get_request(inst);
        if (request != (P3D_request *)NULL) {
          handle_request(request);
        }
        inst = P3D_check_request(wait_cycle);
      }

      while (!_url_getters.empty() && 
             !PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
        // If there are no Windows messages, check the download tasks.
        run_getters();
      }
      retval = GetMessage(&msg, NULL, 0, 0);
    }
    
    // WM_QUIT has been received.  Terminate all instances, and fall
    // through.
    while (!_instances.empty()) {
      P3D_instance *inst = *(_instances.begin());
      delete_instance(inst);
    }

  } else {
    // Not an embedded window, so we don't have our own window to
    // generate Windows events.  Instead, just wait for requests.
    while (!time_to_exit()) {
      P3D_instance *inst = P3D_check_request(wait_cycle);
      if (inst != (P3D_instance *)NULL) {
        P3D_request *request = P3D_instance_get_request(inst);
        if (request != (P3D_request *)NULL) {
          handle_request(request);
        }
      }
      run_getters();
    }
  }

#elif defined(__APPLE__)
  // OSX really prefers to own the main loop, so we install a timer to
  // call out to our instances and getters, rather than polling within
  // the event loop as we do in the Windows case, above.
  EventLoopRef main_loop = GetMainEventLoop();
  EventLoopTimerUPP timer_upp = NewEventLoopTimerUPP(st_timer_callback);
  EventLoopTimerRef timer;
  EventTimerInterval interval = wait_cycle * kEventDurationSecond;
  InstallEventLoopTimer(main_loop, interval, interval,
                        timer_upp, this, &timer);
  RunApplicationEventLoop();
  RemoveEventLoopTimer(timer);
  
  // Terminate all instances, and fall through.
  while (!_instances.empty()) {
    P3D_instance *inst = *(_instances.begin());
    delete_instance(inst);
  }
    
#else  // _WIN32, __APPLE__

  // Now wait while we process pending requests.
  while (!time_to_exit()) {
    P3D_instance *inst = P3D_check_request(wait_cycle);
    if (inst != (P3D_instance *)NULL) {
      P3D_request *request = P3D_instance_get_request(inst);
      if (request != (P3D_request *)NULL) {
        handle_request(request);
      }
    }
    run_getters();
  }

#endif  // _WIN32, __APPLE__
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::run_getters
//       Access: Protected
//  Description: Polls all of the active URL requests.
////////////////////////////////////////////////////////////////////
void Panda3DBase::
run_getters() {
  URLGetters::iterator gi;
  gi = _url_getters.begin();
  while (gi != _url_getters.end()) {
    URLGetter *getter = (*gi);
    if (getter->run()) {
      // This URLGetter is still working.  Leave it.
      ++gi;
    } else {
      // This URLGetter is done.  Remove it and delete it.
      URLGetters::iterator dgi = gi;
      ++gi;
      _url_getters.erase(dgi);
      delete getter;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::handle_request
//       Access: Protected
//  Description: Handles a single request received via the plugin API
//               from a p3d instance.
////////////////////////////////////////////////////////////////////
void Panda3DBase::
handle_request(P3D_request *request) {
  bool handled = false;
  switch (request->_request_type) {
  case P3D_RT_stop:
    delete_instance(request->_instance);
#ifdef _WIN32
    // Post a silly message to spin the event loop.
    PostMessage(NULL, WM_USER, 0, 0);
#endif
    handled = true;
    break;

  case P3D_RT_get_url:
    {
      int unique_id = request->_request._get_url._unique_id;
      const string &url = request->_request._get_url._url;
      URLGetter *getter = new URLGetter
        (request->_instance, unique_id, URLSpec(url), "");
      _url_getters.insert(getter);
      handled = true;
    }
    break;

  case P3D_RT_notify:
    {
      if (strcmp(request->_request._notify._message, "ondownloadnext") == 0) {
        // Tell the user we're downloading a package.
        report_downloading_package(request->_instance);
      } else if (strcmp(request->_request._notify._message, "ondownloadcomplete") == 0) {
        // Tell the user we're done downloading.
        report_download_complete(request->_instance);
      }
    }
    break;

  default:
    break;
  };

  P3D_request_finish(request, handled);
}

#ifdef _WIN32
LONG WINAPI
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  };

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::make_parent_window
//       Access: Protected
//  Description: Creates a toplevel window to contain the embedded
//               instances.  Windows implementation.
////////////////////////////////////////////////////////////////////
void Panda3DBase::
make_parent_window() {
  WNDCLASS wc;

  HINSTANCE application = GetModuleHandle(NULL);
  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.lpfnWndProc = window_proc;
  wc.hInstance = application;
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszClassName = "panda3d";

  if (!RegisterClass(&wc)) {
    cerr << "Could not register window class!\n";
    exit(1);
  }

  DWORD window_style = 
    WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |
    WS_SIZEBOX | WS_MAXIMIZEBOX;

  HWND toplevel_window = 
    CreateWindow("panda3d", "Panda3D", window_style,
                 CW_USEDEFAULT, CW_USEDEFAULT, _win_width, _win_height,
                 NULL, NULL, application, 0);
  if (!toplevel_window) {
    cerr << "Could not create toplevel window!\n";
    exit(1);
  }

  ShowWindow(toplevel_window, SW_SHOWNORMAL);

  memset(&_parent_window, 0, sizeof(_parent_window));
  _parent_window._window_handle_type = P3D_WHT_win_hwnd;
  _parent_window._handle._win_hwnd._hwnd = toplevel_window;
}

#else

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::make_parent_window
//       Access: Protected
//  Description: Creates a toplevel window to contain the embedded
//               instances.
////////////////////////////////////////////////////////////////////
void Panda3DBase::
make_parent_window() {
  // TODO.
  assert(false);
}

#endif

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::create_instance
//       Access: Protected
//  Description: Uses the plugin API to create a new P3D instance to
//               play a particular .p3d file.  This instance is also
//               started if start_instance is true (which requires
//               that the named p3d file exists).
////////////////////////////////////////////////////////////////////
P3D_instance *Panda3DBase::
create_instance(const string &p3d, bool start_instance,
                int win_x, int win_y, int win_width, int win_height,
                char **args, int num_args, const int &p3d_offset) {
  // Check to see if the p3d filename we were given is a URL, or a
  // local file.
  Filename p3d_filename = Filename::from_os_specific(p3d);
  string os_p3d_filename = p3d;
  bool is_local = !is_url(p3d);
  if (is_local && start_instance) {
    if (!p3d_filename.exists()) {
      cerr << "No such file: " << p3d_filename << "\n";
      exit(1);
    }

    p3d_filename.make_absolute();
    os_p3d_filename = p3d_filename.to_os_specific();
  } 

  // Build up the token list.
  Tokens tokens = _tokens;
  P3D_token token;

  string log_basename;
  if (!_log_dirname.empty()) {
    // Generate output to a logfile.
    log_basename = p3d_filename.get_basename_wo_extension();
    token._keyword = "log_basename";
    token._value = log_basename.c_str();
    tokens.push_back(token);
  } else {
    // Send output to the console.
    token._keyword = "console_output";
    if (_console_environment) {
      token._value = "1";
    } else {
      token._value = "0";
    }
    tokens.push_back(token);
  }

  token._keyword = "auto_start";
  token._value = "1";
  tokens.push_back(token);

  P3D_token *tokens_p;
  size_t num_tokens = tokens.size();
  if (!tokens.empty()) {
    tokens_p = &tokens[0];
  }

  // Build up the argument list, beginning with the p3d_filename.
  pvector<const char *> argv;
  argv.push_back(os_p3d_filename.c_str());
  for (int i = 0; i < num_args; ++i) {
    argv.push_back(args[i]);
  }

  P3D_instance *inst = P3D_new_instance(NULL, tokens_p, num_tokens,
                                        argv.size(), &argv[0], NULL);

  if (inst != NULL) {
    if (start_instance) {
      // We call start() first, to give the core API a chance to
      // notice the "hidden" attrib before we set the window
      // parameters.
      P3D_instance_start(inst, is_local, os_p3d_filename.c_str(), p3d_offset);
    }

    P3D_instance_setup_window
      (inst, _window_type, win_x, win_y, win_width, win_height, &_parent_window);
  }

  return inst;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::delete_instance
//       Access: Protected
//  Description: Deletes the indicated instance and removes it from
//               the internal structures.
////////////////////////////////////////////////////////////////////
void Panda3DBase::
delete_instance(P3D_instance *inst) {
  P3D_instance_finish(inst);
  _instances.erase(inst);

  // Make sure we also terminate any pending URLGetters associated
  // with this instance.
  URLGetters::iterator gi;
  gi = _url_getters.begin();
  while (gi != _url_getters.end()) {
    URLGetter *getter = (*gi);
    if (getter->get_instance() == inst) {
      URLGetters::iterator dgi = gi;
      ++gi;
      _url_getters.erase(dgi);
      delete getter;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::parse_token
//       Access: Protected
//  Description: Parses a web token of the form token=value, and
//               stores it in _tokens.  Returns true on success, false
//               on failure.
////////////////////////////////////////////////////////////////////
bool Panda3DBase::
parse_token(char *arg) {
  char *equals = strchr(arg, '=');
  if (equals == NULL) {
    return false;
  }

  // Directly munge the C string to truncate it at the equals sign.
  // Classic C tricks.
  *equals = '\0';
  P3D_token token;
  token._keyword = strdup(arg);
  token._value = strdup(equals + 1);
  *equals = '=';

  _tokens.push_back(token);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::parse_int_pair
//       Access: Protected
//  Description: Parses a string into an x,y pair of integers.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool Panda3DBase::
parse_int_pair(char *arg, int &x, int &y) {
  char *endptr;
  x = strtol(arg, &endptr, 10);
  if (*endptr == ',') {
    y = strtol(endptr + 1, &endptr, 10);
    if (*endptr == '\0') {
      return true;
    }
  }

  // Some parse error on the string.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::is_url
//       Access: Protected, Static
//  Description: Returns true if the indicated string appears to be a
//               URL, with a leading http:// or file:// or whatever,
//               or false if it must be a local filename instead.
////////////////////////////////////////////////////////////////////
bool Panda3DBase::
is_url(const string &param) {
  // We define a URL prefix as a sequence of at least two letters,
  // followed by a colon, followed by at least one slash.
  size_t p = 0;
  while (p < param.size() && isalpha(param[p])) {
    ++p;
  }
  if (p < 2) {
    // Not enough letters.
    return false;
  }
  if (p >= param.size() || param[p] != ':') {
    // No colon.
    return false;
  }
  ++p;
  if (p >= param.size() || param[p] != '/') {
    // No slash.
    return false;
  }

  // It matches the rules.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::report_downloading_package
//       Access: Protected
//  Description: Tells the user we have to download a package.
////////////////////////////////////////////////////////////////////
void Panda3DBase::
report_downloading_package(P3D_instance *instance) {
  P3D_object *obj = P3D_instance_get_panda_script_object(instance);
  
  P3D_object *display_name = P3D_object_get_property(obj, "downloadPackageDisplayName");
  if (display_name == NULL) {
    cerr << "Installing package.\n";
    return;
  }

  int name_length = P3D_object_get_string(display_name, NULL, 0);
  char *name = new char[name_length + 1];
  P3D_object_get_string(display_name, name, name_length + 1);

  cerr << "Installing " << name << "\n";

  delete[] name;
  P3D_object_decref(display_name);
  _reporting_download = true;
}
 
////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::report_download_complete
//       Access: Protected
//  Description: Tells the user we're done downloading packages
////////////////////////////////////////////////////////////////////
void Panda3DBase::
report_download_complete(P3D_instance *instance) {
  if (_reporting_download) {
    cerr << "Install complete.\n";
  }
}

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::st_timer_callback
//       Access: Protected, Static
//  Description: Installed as a timer on the event loop, so we can
//               process local events, in the Apple implementation.
////////////////////////////////////////////////////////////////////
pascal void Panda3DBase::
st_timer_callback(EventLoopTimerRef timer, void *user_data) {
  ((Panda3DBase *)user_data)->timer_callback(timer);
}
#endif  // __APPLE__

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::timer_callback
//       Access: Protected
//  Description: Installed as a timer on the event loop, so we can
//               process local events, in the Apple implementation.
////////////////////////////////////////////////////////////////////
void Panda3DBase::
timer_callback(EventLoopTimerRef timer) {
  // Check for new requests from the Panda3D plugin.
  P3D_instance *inst = P3D_check_request(0.0);
  while (inst != (P3D_instance *)NULL) {
    P3D_request *request = P3D_instance_get_request(inst);
    if (request != (P3D_request *)NULL) {
      handle_request(request);
    }
    inst = P3D_check_request(0.0);
  }
  
  // Check the download tasks.
  run_getters();

  // If we're out of instances, exit the application.
  if (time_to_exit()) {
    QuitApplicationEventLoop();
  }
}
#endif  // __APPLE__

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::URLGetter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Panda3DBase::URLGetter::
URLGetter(P3D_instance *instance, int unique_id,
          const URLSpec &url, const string &post_data) :
  _instance(instance),
  _unique_id(unique_id),
  _url(url),
  _post_data(post_data)
{
  HTTPClient *http = HTTPClient::get_global_ptr();

  _channel = http->make_channel(false);
  //  _channel->set_download_throttle(true);
  if (_post_data.empty()) {
    _channel->begin_get_document(_url);
  } else {
    _channel->begin_post_form(_url, _post_data);
  }

  _channel->download_to_ram(&_rf);
  _bytes_sent = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DBase::URLGetter::run
//       Access: Public
//  Description: Polls the URLGetter for new results.  Returns true if
//               the URL request is still in progress and run() should
//               be called again later, or false if the URL request
//               has been completed and run() should not be called
//               again.
////////////////////////////////////////////////////////////////////
bool Panda3DBase::URLGetter::
run() {
  if (_channel->run() || _rf.get_data_size() != 0) {
    if (_rf.get_data_size() != 0) {
      // Got some new data.
      bool download_ok = P3D_instance_feed_url_stream
        (_instance, _unique_id, P3D_RC_in_progress,
         _channel->get_status_code(),
         _channel->get_file_size(),
         (const unsigned char *)_rf.get_data().data(), _rf.get_data_size());
      _bytes_sent += _rf.get_data_size();
      _rf.clear();

      if (!download_ok) {
        // The plugin doesn't care any more.  Interrupt the download.
        cerr << "Download interrupted: " << _url 
             << ", after " << _bytes_sent << " of " << _channel->get_file_size()
             << " bytes.\n";
        return false;
      }
    }

    // Still more to come; call this method again later.
    return true;
  }

  // All done.
  P3D_result_code status = P3D_RC_done;
  if (!_channel->is_valid()) {
    if (_channel->get_status_code() != 0) {
      status = P3D_RC_http_error;
    } else {
      status = P3D_RC_generic_error;
    }
    cerr << "Error getting URL " << _url << "\n";
  }

  P3D_instance_feed_url_stream
    (_instance, _unique_id, status,
     _channel->get_status_code(),
     _bytes_sent, NULL, 0);
  return false;
}
