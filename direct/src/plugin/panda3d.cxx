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

#include "p3d_plugin.h"

#include <iostream>
#include <string>

#ifdef _WIN32
#include "wingetopt.h"
#include <windows.h>
#else
#include <getopt.h>
#endif

using namespace std;

static const string default_plugin_filename = "libp3d_plugin";

P3D_initialize_func *P3D_initialize;
P3D_free_string_func *P3D_free_string;
P3D_create_instance_func *P3D_create_instance;
P3D_instance_finish_func *P3D_instance_finish;
P3D_instance_has_property_func *P3D_instance_has_property;
P3D_instance_get_property_func *P3D_instance_get_property;
P3D_instance_set_property_func *P3D_instance_set_property;
P3D_instance_get_request_func *P3D_instance_get_request;
P3D_check_request_func *P3D_check_request;
P3D_request_finish_func *P3D_request_finish;
P3D_instance_feed_url_stream_func *P3D_instance_feed_url_stream;

bool
load_plugin(const string &config_xml_filename,
            const string &p3d_plugin_filename) {
  string filename = p3d_plugin_filename;
  if (filename.empty()) {
    // Look for the plugin along the path.
    filename = default_plugin_filename;
#ifdef _WIN32
    filename += ".dll";
#else
    filename += ".so";
#endif
  }

#ifdef _WIN32
  HMODULE module = LoadLibrary(filename.c_str());
  if (module == NULL) {
    // Couldn't load the DLL.
    return false;
  }

  // Get the full path to the DLL in case it was found along the path.
  static const buffer_size = 4096;
  static char buffer[buffer_size];
  if (GetModuleFileName(module, buffer, buffer_size) != 0) {
    if (GetLastError() != 0) {
      filename = buffer;
    }
  }
  cerr << filename << "\n";

  // Now get all of the function pointers.
  P3D_initialize = (P3D_initialize_func *)GetProcAddress(module, "P3D_initialize");  
  P3D_free_string = (P3D_free_string_func *)GetProcAddress(module, "P3D_free_string");  
  P3D_create_instance = (P3D_create_instance_func *)GetProcAddress(module, "P3D_create_instance");  
  P3D_instance_finish = (P3D_instance_finish_func *)GetProcAddress(module, "P3D_instance_finish");  
  P3D_instance_has_property = (P3D_instance_has_property_func *)GetProcAddress(module, "P3D_instance_has_property");  
  P3D_instance_get_property = (P3D_instance_get_property_func *)GetProcAddress(module, "P3D_instance_get_property");  
  P3D_instance_set_property = (P3D_instance_set_property_func *)GetProcAddress(module, "P3D_instance_set_property");  
  P3D_instance_get_request = (P3D_instance_get_request_func *)GetProcAddress(module, "P3D_instance_get_request");  
  P3D_check_request = (P3D_check_request_func *)GetProcAddress(module, "P3D_check_request");  
  P3D_request_finish = (P3D_request_finish_func *)GetProcAddress(module, "P3D_request_finish");  
  P3D_instance_feed_url_stream = (P3D_instance_feed_url_stream_func *)GetProcAddress(module, "P3D_instance_feed_url_stream");  
#endif  // _WIN32

  // Ensure that all of the function pointers have been found.
  if (P3D_initialize == NULL ||
      P3D_free_string == NULL ||
      P3D_create_instance == NULL ||
      P3D_instance_finish == NULL ||
      P3D_instance_has_property == NULL ||
      P3D_instance_get_property == NULL ||
      P3D_instance_set_property == NULL ||
      P3D_instance_get_request == NULL ||
      P3D_check_request == NULL ||
      P3D_request_finish == NULL ||
      P3D_instance_feed_url_stream == NULL) {
    return false;
  }

  // Successfully loaded.
  if (!P3D_initialize(NULL, filename.c_str())) {
    // Oops, failure to initialize.
    return false;
  }

  return true;
}

void
handle_request(P3D_request *request) {
  bool handled = false;

  switch (request->_request_type) {
  case P3D_RT_stop:
    P3D_instance_finish(request->_instance);
    handled = true;
    break;

  default:
    // Some request types are not handled.
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

void
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
#endif // _WIN32

void
usage() {
  cerr
    << "\nUsage:\n"
    << "   panda3d [opts] file.p3d [file_b.p3d file_c.p3d ...]\n\n"
  
    << "This program is used to execute a Panda3D application bundle stored\n"
    << "in a .p3d file.  Normally you only run one p3d bundle at a time,\n"
    << "but it is possible to run multiple bundles simultaneously.\n\n"

    << "Options:\n\n"

    << "  -c config.xml\n"
    << "    Specify the name of the config.xml file that informs the Panda\n"
    << "    plugin where to download patches and such.  This is normally\n"
    << "    not necessary to specify, since it is already stored within\n"
    << "    the Panda plugin itself.\n\n"

    << "  -p p3d_plugin.dll\n"
    << "    Specify the full path to the particular Panda plugin DLL to\n"
    << "    run.  Normally, this will be found by searching in the usual\n"
    << "    places.\n\n"

    << "  -t [toplevel|embedded|fullscreen|hidden]\n"
    << "    Specify the type of graphic window to create.  If you specify "
    << "    \"embedded\", a new window is created to be the parent.\n\n"

    << "  -s width,height\n"
    << "    Specify the size of the graphic window.\n\n"

    << "  -o x,y\n"
    << "    Specify the position (origin) of the graphic window on the\n"
    << "    screen, or on the parent window.\n\n";
}

bool
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

int
main(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;
  const char *optstr = "c:p:t:s:o:h";

  string config_xml_filename;
  string p3d_plugin_filename;
  P3D_window_type window_type = P3D_WT_toplevel;
  int win_x = 0, win_y = 0;
  int win_width = 0, win_height = 0;

  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'c':
      config_xml_filename = optarg;
      break;

    case 'p':
      p3d_plugin_filename = optarg;
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

    case 'h':
    case '?':
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

  if (!load_plugin(config_xml_filename, p3d_plugin_filename)) {
    cerr << "Unable to load Panda3D plugin.\n";
    return 1;
  }

  P3D_window_handle parent_window;
  if (window_type == P3D_WT_embedded) {
    // The user asked for an embedded window.  Create a toplevel
    // window to be its parent, of the requested size.
    make_parent_window(parent_window, win_width, win_height);

    // And center the graphics window within that parent window.
    win_x = (int)(win_width * 0.1);
    win_y = (int)(win_height * 0.1);
    win_width = (int)(win_width * 0.8);
    win_height = (int)(win_height * 0.8);
  }

  // For now, only one instance at a time is supported.  Ignore the
  // remaining command-line parameters.
  P3D_create_instance
    (NULL, argv[1], 
     window_type, win_x, win_y, win_width, win_height, parent_window,
     NULL, 0);

#ifdef _WIN32
  // Wait for new messages from Windows, and new requests from the
  // plugin.
  MSG msg;
  int retval;
  retval = GetMessage(&msg, NULL, 0, 0);
  while (retval != 0) {
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
    retval = GetMessage(&msg, NULL, 0, 0);
  }

#else

  // Now wait while we process pending requests.
  P3D_instance *inst = P3D_check_request(true);
  while (inst != (P3D_instance *)NULL) {
    P3D_request *request = P3D_instance_get_request(inst);
    if (request != (P3D_request *)NULL) {
      handle_request(request);
    }
    inst = P3D_check_request(true);
  }
#endif

  // All instances have finished; we can exit.

  return 0;
}
