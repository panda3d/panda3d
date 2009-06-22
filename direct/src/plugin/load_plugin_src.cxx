// Filename: load_plugin_src.cxx
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


// This code is used in the plugin_standalone directory, and also in
// the plugin_npapi directory.  To facilitate that code re-use with
// minimal structural overhead, it is designed to be simply #included
// into the different source files.

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

static const string default_plugin_filename = "libp3d_plugin";

P3D_initialize_func *P3D_initialize;
P3D_free_string_func *P3D_free_string;
P3D_create_instance_func *P3D_create_instance;
P3D_instance_finish_func *P3D_instance_finish;
P3D_instance_setup_window_func *P3D_instance_setup_window;
P3D_instance_has_property_func *P3D_instance_has_property;
P3D_instance_get_property_func *P3D_instance_get_property;
P3D_instance_set_property_func *P3D_instance_set_property;
P3D_instance_get_request_func *P3D_instance_get_request;
P3D_check_request_func *P3D_check_request;
P3D_request_finish_func *P3D_request_finish;
P3D_instance_feed_url_stream_func *P3D_instance_feed_url_stream;

#ifdef _WIN32
static HMODULE module;
#endif



static void
unload_plugin() {
#ifdef _WIN32
  FreeLibrary(module);
  module = NULL;
#else 
  // TODO: unload_dso
#endif
  
  P3D_initialize = NULL;
  P3D_free_string = NULL;
  P3D_create_instance = NULL;
  P3D_instance_finish = NULL;
  P3D_instance_has_property = NULL;
  P3D_instance_get_property = NULL;
  P3D_instance_set_property = NULL;
  P3D_instance_get_request = NULL;
  P3D_check_request = NULL;
  P3D_request_finish = NULL;
  P3D_instance_feed_url_stream = NULL;
}

static bool
load_plugin(const string &p3d_plugin_filename) {
  string filename = p3d_plugin_filename;
  if (filename.empty()) {
    // Look for the plugin along the path.
    filename = default_plugin_filename + dll_ext;
  }

#ifdef _WIN32
  module = LoadLibrary(filename.c_str());
  if (module == NULL) {
    // Couldn't load the DLL.
    return false;
  }

  // Now get all of the function pointers.
  P3D_initialize = (P3D_initialize_func *)GetProcAddress(module, "P3D_initialize");  
  P3D_free_string = (P3D_free_string_func *)GetProcAddress(module, "P3D_free_string");  
  P3D_create_instance = (P3D_create_instance_func *)GetProcAddress(module, "P3D_create_instance");  
  P3D_instance_finish = (P3D_instance_finish_func *)GetProcAddress(module, "P3D_instance_finish");  
  P3D_instance_setup_window = (P3D_instance_setup_window_func *)GetProcAddress(module, "P3D_instance_setup_window");  
  P3D_instance_has_property = (P3D_instance_has_property_func *)GetProcAddress(module, "P3D_instance_has_property");  
  P3D_instance_get_property = (P3D_instance_get_property_func *)GetProcAddress(module, "P3D_instance_get_property");  
  P3D_instance_set_property = (P3D_instance_set_property_func *)GetProcAddress(module, "P3D_instance_set_property");  
  P3D_instance_get_request = (P3D_instance_get_request_func *)GetProcAddress(module, "P3D_instance_get_request");  
  P3D_check_request = (P3D_check_request_func *)GetProcAddress(module, "P3D_check_request");  
  P3D_request_finish = (P3D_request_finish_func *)GetProcAddress(module, "P3D_request_finish");  
  P3D_instance_feed_url_stream = (P3D_instance_feed_url_stream_func *)GetProcAddress(module, "P3D_instance_feed_url_stream");  

#else  // _WIN32
  // Posix case.
  void *module = dlopen(filename.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (module == NULL) {
    // Couldn't load the .so.
    return false;
  }
  
  // Now get all of the function pointers.
  P3D_initialize = (P3D_initialize_func *)dlsym(module, "P3D_initialize");  
  P3D_free_string = (P3D_free_string_func *)dlsym(module, "P3D_free_string");  
  P3D_create_instance = (P3D_create_instance_func *)dlsym(module, "P3D_create_instance");  
  P3D_instance_finish = (P3D_instance_finish_func *)dlsym(module, "P3D_instance_finish");  
  P3D_instance_setup_window = (P3D_instance_setup_window_func *)dlsym(module, "P3D_instance_setup_window");  
  P3D_instance_has_property = (P3D_instance_has_property_func *)dlsym(module, "P3D_instance_has_property");  
  P3D_instance_get_property = (P3D_instance_get_property_func *)dlsym(module, "P3D_instance_get_property");  
  P3D_instance_set_property = (P3D_instance_set_property_func *)dlsym(module, "P3D_instance_set_property");  
  P3D_instance_get_request = (P3D_instance_get_request_func *)dlsym(module, "P3D_instance_get_request");  
  P3D_check_request = (P3D_check_request_func *)dlsym(module, "P3D_check_request");  
  P3D_request_finish = (P3D_request_finish_func *)dlsym(module, "P3D_request_finish");  
  P3D_instance_feed_url_stream = (P3D_instance_feed_url_stream_func *)dlsym(module, "P3D_instance_feed_url_stream");  

#endif  // _WIN32

  // Ensure that all of the function pointers have been found.
  if (P3D_initialize == NULL ||
      P3D_free_string == NULL ||
      P3D_create_instance == NULL ||
      P3D_instance_finish == NULL ||
      P3D_instance_setup_window == NULL ||
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
#ifdef _WIN32
  string logfilename = "c:/cygwin/home/drose/t0.log";
#else
  string logfilename = "/Users/drose/t0.log";
#endif

  if (!P3D_initialize(P3D_API_VERSION, logfilename.c_str())) {
    // Oops, failure to initialize.
    unload_plugin();
    return false;
  }

  return true;
}
