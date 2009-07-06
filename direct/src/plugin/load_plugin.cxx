// Filename: load_plugin.cxx
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

#include "load_plugin.h"
#include "p3d_plugin_config.h"

#include "assert.h"

#include <iostream>

#ifndef _WIN32
#include <dlfcn.h>
#endif

#ifdef _WIN32
static const string dll_ext = ".dll";
#elif defined(__APPLE__)
static const string dll_ext = ".dylib";
#else
static const string dll_ext = ".so";
#endif

static const string default_plugin_filename = "p3d_plugin";

P3D_initialize_func *P3D_initialize;
P3D_finalize_func *P3D_finalize;
P3D_new_instance_func *P3D_new_instance;
P3D_instance_start_func *P3D_instance_start;
P3D_instance_finish_func *P3D_instance_finish;
P3D_instance_setup_window_func *P3D_instance_setup_window;

P3D_make_class_definition_func *P3D_make_class_definition;
P3D_new_none_object_func *P3D_new_none_object;
P3D_new_bool_object_func *P3D_new_bool_object;
P3D_new_int_object_func *P3D_new_int_object;
P3D_new_float_object_func *P3D_new_float_object;
P3D_new_string_object_func *P3D_new_string_object;
P3D_instance_get_panda_script_object_func *P3D_instance_get_panda_script_object;
P3D_instance_set_browser_script_object_func *P3D_instance_set_browser_script_object;

P3D_instance_get_request_func *P3D_instance_get_request;
P3D_check_request_func *P3D_check_request;
P3D_request_finish_func *P3D_request_finish;
P3D_instance_feed_url_stream_func *P3D_instance_feed_url_stream;

#ifdef _WIN32
static HMODULE module = NULL;
#else
static void *module = NULL;
#endif

static bool plugin_loaded = false;


////////////////////////////////////////////////////////////////////
//     Function: get_plugin_basename
//  Description: Returns the default plugin filename, without any
//               directory path (but including the extension
//               appropriate to this platform).
////////////////////////////////////////////////////////////////////
string
get_plugin_basename() {
  return default_plugin_filename + dll_ext;
}

////////////////////////////////////////////////////////////////////
//     Function: is_pathsep
//  Description: Returns true if the indicated character is a path
//               separator character (e.g. slash or backslash), false
//               otherwise.
////////////////////////////////////////////////////////////////////
static inline bool
is_pathsep(char ch) {
  if (ch == '/') {
    return true;
  }
#ifdef _WIN32
  if (ch == '\\') {
    return true;
  }
#endif
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: find_extension_dot
//  Description: Returns the position in the string of the dot before
//               the filename extension; that is, the position of the
//               rightmost dot that is right of the rightmost slash
//               (or backslash, on Windows).  Returns string::npos if
//               there is no extension.
////////////////////////////////////////////////////////////////////
static size_t
find_extension_dot(const string &filename) {
  size_t p = filename.length();
  while (p > 0 && !is_pathsep(filename[p - 1])) {
    --p;
    if (filename[p] == '.') {
      return p;
    }
  }

  return string::npos;
}

// Forward reference for function defined below.
static void unload_dso();

////////////////////////////////////////////////////////////////////
//     Function: load_plugin
//  Description: Loads the plugin and assigns all of the function
//               pointers.  Returns true on success, false on failure.
//               If the filename is empty, it is searched along the
//               path.
////////////////////////////////////////////////////////////////////
bool
load_plugin(const string &p3d_plugin_filename) {
  string filename = p3d_plugin_filename;
  if (filename.empty()) {
    // Look for the plugin along the path.
    filename = get_plugin_basename();
  }

  if (plugin_loaded) {
    return true;
  }

#ifdef _WIN32
  assert(module == NULL);
  
  // On Windows, the filename passed to LoadLibrary() must have an
  // extension, or a default ".DLL" will be implicitly added.  If the
  // file actually has no extension, we must add "." to avoid this.

  // Check whether the filename has an extension.
  size_t extension_dot = find_extension_dot(filename);
  if (extension_dot == string::npos) {
    // No extension.
    filename += ".";
  }

  SetErrorMode(0);
  module = LoadLibrary(filename.c_str());
  if (module == NULL) {
    // Couldn't load the DLL.
    return false;
  }

  #define get_func GetProcAddress

#else  // _WIN32
  // Posix case.
  assert(module == NULL);
  module = dlopen(filename.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (module == NULL) {
    // Couldn't load the .so.
    return false;
  }

  #define get_func dlsym

#endif  // _WIN32

  cerr << "Got module = " << module << "\n";

  // Now get all of the function pointers.
  P3D_initialize = (P3D_initialize_func *)get_func(module, "P3D_initialize");  
  P3D_finalize = (P3D_finalize_func *)get_func(module, "P3D_finalize");  
  P3D_new_instance = (P3D_new_instance_func *)get_func(module, "P3D_new_instance");  
  P3D_instance_start = (P3D_instance_start_func *)get_func(module, "P3D_instance_start");  
  P3D_instance_finish = (P3D_instance_finish_func *)get_func(module, "P3D_instance_finish");  
  P3D_instance_setup_window = (P3D_instance_setup_window_func *)get_func(module, "P3D_instance_setup_window");  

  P3D_make_class_definition = (P3D_make_class_definition_func *)get_func(module, "P3D_make_class_definition");
  P3D_new_none_object = (P3D_new_none_object_func *)get_func(module, "P3D_new_none_object");
  P3D_new_bool_object = (P3D_new_bool_object_func *)get_func(module, "P3D_new_bool_object");
  P3D_new_int_object = (P3D_new_int_object_func *)get_func(module, "P3D_new_int_object");
  P3D_new_float_object = (P3D_new_float_object_func *)get_func(module, "P3D_new_float_object");
  P3D_new_string_object = (P3D_new_string_object_func *)get_func(module, "P3D_new_string_object");
  P3D_instance_get_panda_script_object = (P3D_instance_get_panda_script_object_func *)get_func(module, "P3D_instance_get_panda_script_object");
  P3D_instance_set_browser_script_object = (P3D_instance_set_browser_script_object_func *)get_func(module, "P3D_instance_set_browser_script_object");

  P3D_instance_get_request = (P3D_instance_get_request_func *)get_func(module, "P3D_instance_get_request");  
  P3D_check_request = (P3D_check_request_func *)get_func(module, "P3D_check_request");  
  P3D_request_finish = (P3D_request_finish_func *)get_func(module, "P3D_request_finish");  
  P3D_instance_feed_url_stream = (P3D_instance_feed_url_stream_func *)get_func(module, "P3D_instance_feed_url_stream");  

  #undef get_func

  // Ensure that all of the function pointers have been found.
  if (P3D_initialize == NULL ||
      P3D_finalize == NULL ||
      P3D_new_instance == NULL ||
      P3D_instance_start == NULL ||
      P3D_instance_finish == NULL ||
      P3D_instance_setup_window == NULL ||

      P3D_make_class_definition == NULL ||
      P3D_new_none_object == NULL ||
      P3D_new_bool_object == NULL ||
      P3D_new_int_object == NULL ||
      P3D_new_float_object == NULL ||
      P3D_new_string_object == NULL ||
      P3D_instance_get_panda_script_object == NULL ||
      P3D_instance_set_browser_script_object == NULL ||
      
      P3D_instance_get_request == NULL ||
      P3D_check_request == NULL ||
      P3D_request_finish == NULL ||
      P3D_instance_feed_url_stream == NULL) {
    
    cerr
      << "Some function pointers not found:"
      << "\nP3D_initialize = " << P3D_initialize
      << "\nP3D_finalize = " << P3D_finalize
      << "\nP3D_new_instance = " << P3D_new_instance
      << "\nP3D_instance_start = " << P3D_instance_start
      << "\nP3D_instance_finish = " << P3D_instance_finish
      << "\nP3D_instance_setup_window = " << P3D_instance_setup_window
      
      << "\nP3D_make_class_definition = " << P3D_make_class_definition
      << "\nP3D_new_none_object = " << P3D_new_none_object
      << "\nP3D_new_bool_object = " << P3D_new_bool_object
      << "\nP3D_new_int_object = " << P3D_new_int_object
      << "\nP3D_new_float_object = " << P3D_new_float_object
      << "\nP3D_new_string_object = " << P3D_new_string_object
      << "\nP3D_instance_get_panda_script_object = " << P3D_instance_get_panda_script_object
      << "\nP3D_instance_set_browser_script_object = " << P3D_instance_set_browser_script_object
      
      << "\nP3D_instance_get_request = " << P3D_instance_get_request
      << "\nP3D_check_request = " << P3D_check_request
      << "\nP3D_request_finish = " << P3D_request_finish
      << "\nP3D_instance_feed_url_stream = " << P3D_instance_feed_url_stream
      << "\n";
    return false;
  }

  // Successfully loaded.
  plugin_loaded = true;

  string logfilename = P3D_PLUGIN_LOGFILE2;

  if (!P3D_initialize(P3D_API_VERSION, logfilename.c_str())) {
    // Oops, failure to initialize.
    cerr << "Failed to initialize plugin (wrong API version?)\n";
    unload_plugin();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: unload_plugin
//  Description: Calls finalize, then removes the plugin from memory
//               space and clears all of the pointers.
////////////////////////////////////////////////////////////////////
void
unload_plugin() {
  if (!plugin_loaded) {
    return;
  }

  cerr << "unload_plugin called\n";

  P3D_finalize();
  unload_dso();
}

////////////////////////////////////////////////////////////////////
//     Function: unload_dso
//  Description: Removes the plugin from memory space and clears all
//               of the pointers.  This is only intended to be called
//               by load_plugin(), above, in the specific case that
//               the plugin loaded but could not successfully
//               initialize itself.  All user code should call
//               unload_plugin(), above, which first calls
//               P3D_finalize().
////////////////////////////////////////////////////////////////////
static void
unload_dso() {
#ifdef _WIN32
  assert(module != NULL);
  FreeLibrary(module);
  module = NULL;
#else 
  assert(module != NULL);
  dlclose(module);
  module = NULL;
#endif
  
  P3D_initialize = NULL;
  P3D_finalize = NULL;
  P3D_new_instance = NULL;
  P3D_instance_start = NULL;
  P3D_instance_finish = NULL;
  P3D_instance_setup_window = NULL;

  P3D_make_class_definition = NULL;
  P3D_new_none_object = NULL;
  P3D_new_bool_object = NULL;
  P3D_new_int_object = NULL;
  P3D_new_float_object = NULL;
  P3D_new_string_object = NULL;
  P3D_instance_get_panda_script_object = NULL;
  P3D_instance_set_browser_script_object = NULL;

  P3D_instance_get_request = NULL;
  P3D_check_request = NULL;
  P3D_request_finish = NULL;
  P3D_instance_feed_url_stream = NULL;

  plugin_loaded = false;
}

////////////////////////////////////////////////////////////////////
//     Function: is_plugin_loaded
//  Description: Returns true if the plugin has been loaded
//               successfully by a previous call to load_plugin(),
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool
is_plugin_loaded() {
  return plugin_loaded;
}
