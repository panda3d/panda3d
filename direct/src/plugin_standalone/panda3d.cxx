// Filename: panda3d.cxx
// Created by:  drose (03Jun09)
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

#include "panda3d.h"
#include "httpClient.h"
#include "load_plugin.h"
#include "find_root_dir.h"
#include "p3d_plugin_config.h"

// We can include this header file to get the DTOOL_PLATFORM
// definition, even though we don't link with dtool.
#include "dtool_platform.h"

#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif

#ifndef HAVE_GETOPT
  #include "gnu_getopt.h"
#else
  #ifdef PHAVE_GETOPT_H
    #include <getopt.h>
  #endif
#endif


////////////////////////////////////////////////////////////////////
//     Function: Panda3D::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Panda3D::
Panda3D() {
  _root_dir = find_root_dir();
  _reporting_download = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::run
//       Access: Public
//  Description: Starts the program going.  Returns 0 on success,
//               nonzero on failure.
////////////////////////////////////////////////////////////////////
int Panda3D::
run(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;

  // We prefix a "+" sign to tell gnu getopt not to parse options
  // following the first not-option parameter.  (These will be passed
  // into the sub-process.)
  const char *optstr = "+mu:p:ft:s:o:l:h";

  bool allow_multiple = false;
  string download_url = PANDA_PACKAGE_HOST_URL;
  string this_platform = DTOOL_PLATFORM;
  bool verify_contents = false;

  P3D_window_type window_type = P3D_WT_toplevel;
  int win_x = 0, win_y = 0;
  int win_width = 640, win_height = 480;

  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'm':
      allow_multiple = true;
      break;

    case 'u':
      download_url = optarg;
      break;

    case 'p':
      this_platform = optarg;
      break;

    case 'f':
      verify_contents = true;
      break;

    case 't':
      if (strcmp(optarg, "toplevel") == 0) {
        window_type = P3D_WT_toplevel;
      } else if (strcmp(optarg, "embedded") == 0) {
        window_type = P3D_WT_embedded;
      } else if (strcmp(optarg, "fullscreen") == 0) {
        window_type = P3D_WT_fullscreen;
      } else if (strcmp(optarg, "hidden") == 0) {
        window_type = P3D_WT_hidden;
      } else {
        cerr << "Invalid value for -t: " << optarg << "\n";
        return 1;
      }
      break;

    case 's':
      if (!parse_int_pair(optarg, win_width, win_height)) {
        cerr << "Invalid value for -s: " << optarg << "\n";
        return 1;
      }
      break;

    case 'o':
      if (!parse_int_pair(optarg, win_x, win_y)) {
        cerr << "Invalid value for -o: " << optarg << "\n";
        return 1;
      }
      break;

    case 'l':
      _log_dirname = Filename::from_os_specific(optarg).to_os_specific();
      _log_basename = "panda3d";
      break;

    case 'h':
    case '?':
    case '+':
    default:
      usage();
      return 1;
    }
    flag = getopt(argc, argv, optstr);
  }

  argc -= (optind-1);
  argv += (optind-1);

  if (argc < 2) {
    usage();
    return 1;
  }

  // Make sure it ends with a slash.
  if (!download_url.empty() && download_url[download_url.length() - 1] != '/') {
    download_url += '/';
  }

  if (!get_plugin(download_url, this_platform, verify_contents)) {
    cerr << "Unable to load Panda3D plugin.\n";
    return 1;
  }

  int num_instance_filenames, num_instance_args;
  char **instance_filenames, **instance_args;

  if (allow_multiple) {
    // With -m, the remaining arguments are all instance filenames.
    num_instance_filenames = argc - 1;
    instance_filenames = argv + 1;
    num_instance_args = 0;
    instance_args = argv + argc;

  } else {
    // Without -m, there is one instance filename, and everything else
    // gets delivered to that instance.
    num_instance_filenames = 1;
    instance_filenames = argv + 1;
    num_instance_args = argc - 2;
    instance_args = argv + 2;
  }

  P3D_window_handle parent_window;
  if (window_type == P3D_WT_embedded) {
    // The user asked for an embedded window.  Create a toplevel
    // window to be its parent, of the requested size.
    if (win_width == 0 && win_height == 0) {
      win_width = 640;
      win_height = 480;
    }

    make_parent_window(parent_window, win_width, win_height);
    
    // Center the child window(s) within the parent window.
#ifdef _WIN32
    RECT rect;
    GetClientRect(parent_window._hwnd, &rect);

    win_x = (int)(rect.right * 0.1);
    win_y = (int)(rect.bottom * 0.1);
    win_width = (int)(rect.right * 0.8);
    win_height = (int)(rect.bottom * 0.8);
#endif

    // Subdivide the window into num_x_spans * num_y_spans sub-windows.
    int num_y_spans = int(sqrt((double)num_instance_filenames));
    int num_x_spans = (num_instance_filenames + num_y_spans - 1) / num_y_spans;
    
    int inst_width = win_width / num_x_spans;
    int inst_height = win_height / num_y_spans;

    for (int yi = 0; yi < num_y_spans; ++yi) {
      for (int xi = 0; xi < num_x_spans; ++xi) {
        int i = yi * num_x_spans + xi;
        if (i >= num_instance_filenames) {
          continue;
        }

        // Create instance i at window slot (xi, yi).
        int inst_x = win_x + xi * inst_width;
        int inst_y = win_y + yi * inst_height;

        P3D_instance *inst = create_instance
          (instance_filenames[i], P3D_WT_embedded, 
           inst_x, inst_y, inst_width, inst_height, parent_window,
           instance_args, num_instance_args);
        _instances.insert(inst);
      }
    }

  } else {
    // Not an embedded window.  Create each window with the same parameters.
    for (int i = 0; i < num_instance_filenames; ++i) {
      P3D_instance *inst = create_instance
        (instance_filenames[i], window_type, 
         win_x, win_y, win_width, win_height, parent_window,
         instance_args, num_instance_args);
      _instances.insert(inst);
    }
  }

#ifdef _WIN32
  if (window_type == P3D_WT_embedded) {
    // Wait for new messages from Windows, and new requests from the
    // plugin.
    MSG msg;
    int retval;
    retval = GetMessage(&msg, NULL, 0, 0);
    while (retval != 0 && !_instances.empty()) {
      if (retval == -1) {
        cerr << "Error processing message queue.\n";
        exit(1);
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);

      // Check for new requests from the Panda3D plugin.
      P3D_instance *inst = P3D_check_request(false);
      while (inst != (P3D_instance *)NULL) {
        P3D_request *request = P3D_instance_get_request(inst);
        if (request != (P3D_request *)NULL) {
          handle_request(request);
        }
        inst = P3D_check_request(false);
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
    while (!_instances.empty()) {
      P3D_instance *inst = P3D_check_request(false);
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
  EventTimerInterval interval = 200 * kEventDurationMillisecond;
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
  while (!_instances.empty()) {
    P3D_instance *inst = P3D_check_request(false);
    if (inst != (P3D_instance *)NULL) {
      P3D_request *request = P3D_instance_get_request(inst);
      if (request != (P3D_request *)NULL) {
        handle_request(request);
      }
    }
    run_getters();
  }

#endif  // _WIN32, __APPLE__

  // All instances have finished; we can exit.
  unload_plugin();
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::get_plugin
//       Access: Private
//  Description: Downloads the contents.xml file from the named URL
//               and attempts to use it to load the core API.  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool Panda3D::
get_plugin(const string &download_url, const string &this_platform, 
           bool verify_contents) {
  // First, look for the existing contents.xml file.
  Filename contents_filename = Filename(Filename::from_os_specific(_root_dir), "contents.xml");
  if (!verify_contents && read_contents_file(contents_filename, download_url, this_platform, verify_contents)) {
    // Got the file, and it's good.
    return true;
  }

  // Couldn't read it, so go get it.
  string url = download_url;
  url += "contents.xml";
  
  HTTPClient *http = HTTPClient::get_global_ptr();
  PT(HTTPChannel) channel = http->get_document(url);

  // First, download it to a temporary file.
  Filename tempfile = Filename::temporary("", "p3d_");
  if (!channel->download_to_file(tempfile)) {
    cerr << "Unable to download " << url << "\n";
    tempfile.unlink();

    // Couldn't download, but fall through and try to read the
    // contents.xml file anyway.  Maybe it's good enough.
  } else {
    // Successfully downloaded; move the temporary file into place.
    contents_filename.make_dir();
    contents_filename.unlink();
    tempfile.rename_to(contents_filename);
  }

  // Since we had to download some of it, might as well ask the core
  // API to check all of it.
  verify_contents = true;

  return read_contents_file(contents_filename, download_url, this_platform, verify_contents);
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::read_contents_file
//       Access: Private
//  Description: Attempts to open and read the contents.xml file on
//               disk, and uses that data to load the plugin, if
//               possible.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool Panda3D::
read_contents_file(Filename contents_filename, const string &download_url, 
                   const string &this_platform, bool verify_contents) {
  ifstream in;
  contents_filename.set_text();
  if (!contents_filename.open_read(in)) {
    return false;
  }

  TiXmlDocument doc;
  in >> doc;

  TiXmlElement *xcontents = doc.FirstChildElement("contents");
  if (xcontents != NULL) {
    TiXmlElement *xpackage = xcontents->FirstChildElement("package");
    while (xpackage != NULL) {
      const char *name = xpackage->Attribute("name");
      if (name != NULL && strcmp(name, "coreapi") == 0) {
        const char *xplatform = xpackage->Attribute("platform");
        if (xplatform != NULL && strcmp(xplatform, this_platform.c_str()) == 0) {
          return get_core_api(contents_filename, download_url, 
                              this_platform, verify_contents, xpackage);
        }
      }
      
      xpackage = xpackage->NextSiblingElement("package");
    }
  }

  // Couldn't find the coreapi plugin description.
  cerr << "No coreapi plugin defined in contents file for "
       << this_platform << "\n";
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: Panda3D::get_core_api
//       Access: Private
//  Description: Checks the core API DLL file against the
//               specification in the contents file, and downloads it
//               if necessary.
////////////////////////////////////////////////////////////////////
bool Panda3D::
get_core_api(const Filename &contents_filename, const string &download_url,
             const string &this_platform, bool verify_contents,
             TiXmlElement *xpackage) {
  _core_api_dll.load_xml(xpackage);

  if (!_core_api_dll.quick_verify(_root_dir)) {
    // The DLL file needs to be downloaded.  Go get it.
    string url = download_url;
    url += _core_api_dll.get_filename();
    
    Filename pathname = Filename::from_os_specific(_core_api_dll.get_pathname(_root_dir));
    HTTPClient *http = HTTPClient::get_global_ptr();
    PT(HTTPChannel) channel = http->get_document(url);
    pathname.make_dir();
    if (!channel->download_to_file(pathname)) {
      cerr << "Unable to download " << url << "\n";
      return false;
    }

    if (!_core_api_dll.full_verify(_root_dir)) {
      cerr << "Mismatched download for " << url << "\n";
      return false;
    }

    // Since we had to download some of it, might as well ask the core
    // API to check all of it.
    verify_contents = true;
  }

  // Now we've got the DLL.  Load it.
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

  if (!load_plugin(pathname, contents_filename.to_os_specific(),
                   download_url, verify_contents, this_platform, _log_dirname,
                   _log_basename)) {
    cerr << "Unable to launch core API in " << pathname << "\n" << flush;
    return false;
  }

  // Successfully loaded.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::run_getters
//       Access: Private
//  Description: Polls all of the active URL requests.
////////////////////////////////////////////////////////////////////
void Panda3D::
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
//     Function: Panda3D::handle_request
//       Access: Private
//  Description: Handles a single request received via the plugin API
//               from a p3d instance.
////////////////////////////////////////////////////////////////////
void Panda3D::
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
//     Function: Panda3D::make_parent_window
//       Access: Private
//  Description: Creates a toplevel window to contain the embedded
//               instances.  Windows implementation.
////////////////////////////////////////////////////////////////////
void Panda3D::
make_parent_window(P3D_window_handle &parent_window, 
                   int win_width, int win_height) {
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
                 CW_USEDEFAULT, CW_USEDEFAULT, win_width, win_height,
                 NULL, NULL, application, 0);
  if (!toplevel_window) {
    cerr << "Could not create toplevel window!\n";
    exit(1);
  }

  ShowWindow(toplevel_window, SW_SHOWNORMAL);

  parent_window._hwnd = toplevel_window;
}

#else

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::make_parent_window
//       Access: Private
//  Description: Creates a toplevel window to contain the embedded
//               instances.
////////////////////////////////////////////////////////////////////
void Panda3D::
make_parent_window(P3D_window_handle &parent_window, 
                   int win_width, int win_height) {
  // TODO.
  assert(false);
}

#endif

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::create_instance
//       Access: Private
//  Description: Uses the plugin API to create a new P3D instance to
//               play a particular .p3d file.
////////////////////////////////////////////////////////////////////
P3D_instance *Panda3D::
create_instance(const string &p3d, P3D_window_type window_type,
                int win_x, int win_y, int win_width, int win_height,
                P3D_window_handle parent_window, char **args, int num_args) {
  // If the supplied parameter name is a real file, pass it in on the
  // parameter list.  Otherwise, assume it's a URL and let the plugin
  // download it.
  Filename p3d_filename = Filename::from_os_specific(p3d);
  string os_p3d_filename = p3d;
  bool is_local = false;
  if (p3d_filename.exists()) {
    p3d_filename.make_absolute();
    os_p3d_filename = p3d_filename.to_os_specific();
    is_local = true;
  } 

  // Build up the token list.
  pvector<P3D_token> tokens;
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
    token._value = "1";
    tokens.push_back(token);
  }

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
    // We call start() first, to give the core API a chance to notice
    // the "hidden" attrib before we set the window parameters.
    P3D_instance_start(inst, is_local, os_p3d_filename.c_str());
    P3D_instance_setup_window
      (inst, window_type, win_x, win_y, win_width, win_height, parent_window);
  }

  return inst;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::delete_instance
//       Access: Private
//  Description: Deletes the indicated instance and removes it from
//               the internal structures.
////////////////////////////////////////////////////////////////////
void Panda3D::
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
//     Function: Panda3D::usage
//       Access: Private
//  Description: Reports the available command-line options.
////////////////////////////////////////////////////////////////////
void Panda3D::
usage() {
  cerr
    << "\nUsage:\n"
    << "   panda3d [opts] file.p3d [args]\n\n"
    << "   panda3d -m [opts] file_a.p3d file_b.p3d [file_c.p3d ...]\n\n"
  
    << "This program is used to execute a Panda3D application bundle stored\n"
    << "in a .p3d file.  In the first form, without the -m option, it\n"
    << "executes one application; remaining arguments following the\n"
    << "application name are passed into the application.  In the second\n"
    << "form, with the -m option, it can execute multiple applications\n"
    << "simultaneously, though in this form arguments cannot be passed into\n"
    << "the applications.\n\n"

    << "Options:\n\n"

    << "  -m\n"
    << "    Indicates that multiple application filenames will be passed on\n"
    << "    the command line.  All applications will be run at the same\n"
    << "    time, but additional arguments may not be passed to any of the\n"
    << "    applictions.\n\n"

    << "  -t [toplevel|embedded|fullscreen|hidden]\n"
    << "    Specify the type of graphic window to create.  If you specify\n"
    << "    \"embedded\", a new window is created to be the parent.\n\n"

    << "  -s width,height\n"
    << "    Specify the size of the graphic window.\n\n"

    << "  -o x,y\n"
    << "    Specify the position (origin) of the graphic window on the\n"
    << "    screen, or on the parent window.\n\n"

    << "  -l log_dirname\n"
    << "    Specify the full path to the directory in which log files are\n"
    << "    to be written.  If this is not specified, the default is to send\n"
    << "    the application output to the console.\n\n"

    << "  -f\n"
    << "    Force an initial contact of the Panda3D download server, to check\n"
    << "    if a new version is available.  Normally, this is done only\n"
    << "    if contents.xml cannot be read.\n\n"

    << "  -u url\n"
    << "    Specify the URL of the Panda3D download server.  The default is\n"
    << "    \"" << PANDA_PACKAGE_HOST_URL << "\" .\n\n"

    << "  -p platform\n"
    << "    Specify the platform to masquerade as.  The default is \""
    << DTOOL_PLATFORM << "\" .\n\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::parse_int_pair
//       Access: Private
//  Description: Parses a string into an x,y pair of integers.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool Panda3D::
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
//     Function: Panda3D::report_downloading_package
//       Access: Private
//  Description: Tells the user we have to download a package.
////////////////////////////////////////////////////////////////////
void Panda3D::
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
//     Function: Panda3D::report_download_complete
//       Access: Private
//  Description: Tells the user we're done downloading packages
////////////////////////////////////////////////////////////////////
void Panda3D::
report_download_complete(P3D_instance *instance) {
  if (_reporting_download) {
    cerr << "Install complete.\n";
  }
}

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: Panda3D::st_timer_callback
//       Access: Private, Static
//  Description: Installed as a timer on the event loop, so we can
//               process local events, in the Apple implementation.
////////////////////////////////////////////////////////////////////
pascal void Panda3D::
st_timer_callback(EventLoopTimerRef timer, void *user_data) {
  ((Panda3D *)user_data)->timer_callback(timer);
}
#endif  // __APPLE__

#ifdef __APPLE__
////////////////////////////////////////////////////////////////////
//     Function: Panda3D::timer_callback
//       Access: Private
//  Description: Installed as a timer on the event loop, so we can
//               process local events, in the Apple implementation.
////////////////////////////////////////////////////////////////////
void Panda3D::
timer_callback(EventLoopTimerRef timer) {
  // Check for new requests from the Panda3D plugin.
  P3D_instance *inst = P3D_check_request(false);
  while (inst != (P3D_instance *)NULL) {
    P3D_request *request = P3D_instance_get_request(inst);
    if (request != (P3D_request *)NULL) {
      handle_request(request);
    }
    inst = P3D_check_request(false);
  }
  
  // Check the download tasks.
  run_getters();

  // If we're out of instances, exit the application.
  if (_instances.empty()) {
    QuitApplicationEventLoop();
  }
}
#endif  // __APPLE__

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::URLGetter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Panda3D::URLGetter::
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
//     Function: Panda3D::URLGetter::run
//       Access: Public
//  Description: Polls the URLGetter for new results.  Returns true if
//               the URL request is still in progress and run() should
//               be called again later, or false if the URL request
//               has been completed and run() should not be called
//               again.
////////////////////////////////////////////////////////////////////
bool Panda3D::URLGetter::
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


int
main(int argc, char *argv[]) {
  Panda3D program;
  return program.run(argc, argv);
}
