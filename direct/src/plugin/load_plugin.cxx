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
#include "is_pathsep.h"
#include "wstring_encode.h"

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

P3D_initialize_func *P3D_initialize_ptr;
P3D_finalize_func *P3D_finalize_ptr;
P3D_set_plugin_version_func *P3D_set_plugin_version_ptr;
P3D_set_super_mirror_func *P3D_set_super_mirror_ptr;
P3D_new_instance_func *P3D_new_instance_ptr;
P3D_instance_start_func *P3D_instance_start_ptr;
P3D_instance_start_stream_func *P3D_instance_start_stream_ptr;
P3D_instance_finish_func *P3D_instance_finish_ptr;
P3D_instance_setup_window_func *P3D_instance_setup_window_ptr;

P3D_object_get_type_func *P3D_object_get_type_ptr;
P3D_object_get_bool_func *P3D_object_get_bool_ptr;
P3D_object_get_int_func *P3D_object_get_int_ptr;
P3D_object_get_float_func *P3D_object_get_float_ptr;
P3D_object_get_string_func *P3D_object_get_string_ptr;
P3D_object_get_repr_func *P3D_object_get_repr_ptr;
P3D_object_get_property_func *P3D_object_get_property_ptr;
P3D_object_set_property_func *P3D_object_set_property_ptr;
P3D_object_has_method_func *P3D_object_has_method_ptr;
P3D_object_call_func *P3D_object_call_ptr;
P3D_object_eval_func *P3D_object_eval_ptr;
P3D_object_incref_func *P3D_object_incref_ptr;
P3D_object_decref_func *P3D_object_decref_ptr;

P3D_make_class_definition_func *P3D_make_class_definition_ptr;
P3D_new_undefined_object_func *P3D_new_undefined_object_ptr;
P3D_new_none_object_func *P3D_new_none_object_ptr;
P3D_new_bool_object_func *P3D_new_bool_object_ptr;
P3D_new_int_object_func *P3D_new_int_object_ptr;
P3D_new_float_object_func *P3D_new_float_object_ptr;
P3D_new_string_object_func *P3D_new_string_object_ptr;
P3D_instance_get_panda_script_object_func *P3D_instance_get_panda_script_object_ptr;
P3D_instance_set_browser_script_object_func *P3D_instance_set_browser_script_object_ptr;

P3D_instance_get_request_func *P3D_instance_get_request_ptr;
P3D_check_request_func *P3D_check_request_ptr;
P3D_request_finish_func *P3D_request_finish_ptr;
P3D_instance_feed_url_stream_func *P3D_instance_feed_url_stream_ptr;
P3D_instance_handle_event_func *P3D_instance_handle_event_ptr;

#ifdef _WIN32
static HMODULE module = NULL;
#else
static void *module = NULL;
#endif

static bool plugin_loaded = false;
static bool dso_needs_unload = false;


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
//               If load_plugin() has already been called
//               successfully, this returns true immediately, without
//               parsing any parameters.
//
//               If p3d_plugin_filename is empty, the module is
//               assumed to be already loaded (or statically linked
//               in), and the symbols are located within the current
//               address space.
////////////////////////////////////////////////////////////////////
bool
load_plugin(const string &p3d_plugin_filename, 
            const string &contents_filename, const string &host_url, 
            P3D_verify_contents verify_contents, const string &platform,
            const string &log_directory, const string &log_basename,
            bool trusted_environment, bool console_environment,
            const string &root_dir, const string &host_dir,
            ostream &logfile) {
  if (plugin_loaded) {
    return true;
  }
  string filename = p3d_plugin_filename;

#ifdef _WIN32
  assert(module == NULL);

  if (filename.empty()) {
    // If no filename is supplied, look within our existing address space.
    module = GetModuleHandle(NULL);
    dso_needs_unload = false;

  } else {
    // If a filename is supplied, attempt to load it as a dynamic library.

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
    wstring filename_w;
    if (string_to_wstring(filename_w, filename)) {
      module = LoadLibraryW(filename_w.c_str());
    }
    dso_needs_unload = true;
  }

  if (module == NULL) {
    // Couldn't load the DLL.
    logfile 
      << "Couldn't load " << filename << ", error = " 
      << GetLastError() << "\n";
    return false;
  }

  #define get_func GetProcAddress

#else  // _WIN32
  // Posix case.
  assert(module == NULL);
  if (filename.empty()) {
    module = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL);
  } else {
    module = dlopen(filename.c_str(), RTLD_LAZY | RTLD_LOCAL);
  }
  if (module == NULL) {
    // Couldn't load the .so.
    const char *message = dlerror();
    if (message == (char *)NULL) {
      message = "No error";
    }
    logfile << "Couldn't load " << filename << ": " << message << "\n";

    return false;
  }
  dso_needs_unload = true;

  #define get_func dlsym

#endif  // _WIN32

  // Now get all of the function pointers.
  P3D_initialize_ptr = (P3D_initialize_func *)get_func(module, "P3D_initialize");  
  P3D_finalize_ptr = (P3D_finalize_func *)get_func(module, "P3D_finalize");  
  P3D_set_plugin_version_ptr = (P3D_set_plugin_version_func *)get_func(module, "P3D_set_plugin_version");  
  P3D_set_super_mirror_ptr = (P3D_set_super_mirror_func *)get_func(module, "P3D_set_super_mirror");  
  P3D_new_instance_ptr = (P3D_new_instance_func *)get_func(module, "P3D_new_instance");  
  P3D_instance_start_ptr = (P3D_instance_start_func *)get_func(module, "P3D_instance_start");  
  P3D_instance_start_stream_ptr = (P3D_instance_start_stream_func *)get_func(module, "P3D_instance_start_stream");  
  P3D_instance_finish_ptr = (P3D_instance_finish_func *)get_func(module, "P3D_instance_finish");  
  P3D_instance_setup_window_ptr = (P3D_instance_setup_window_func *)get_func(module, "P3D_instance_setup_window");  

  P3D_object_get_type_ptr = (P3D_object_get_type_func *)get_func(module, "P3D_object_get_type");
  P3D_object_get_bool_ptr = (P3D_object_get_bool_func *)get_func(module, "P3D_object_get_bool");
  P3D_object_get_int_ptr = (P3D_object_get_int_func *)get_func(module, "P3D_object_get_int");
  P3D_object_get_float_ptr = (P3D_object_get_float_func *)get_func(module, "P3D_object_get_float");
  P3D_object_get_string_ptr = (P3D_object_get_string_func *)get_func(module, "P3D_object_get_string");
  P3D_object_get_repr_ptr = (P3D_object_get_repr_func *)get_func(module, "P3D_object_get_repr");
  P3D_object_get_property_ptr = (P3D_object_get_property_func *)get_func(module, "P3D_object_get_property");
  P3D_object_set_property_ptr = (P3D_object_set_property_func *)get_func(module, "P3D_object_set_property");
  P3D_object_has_method_ptr = (P3D_object_has_method_func *)get_func(module, "P3D_object_has_method");
  P3D_object_call_ptr = (P3D_object_call_func *)get_func(module, "P3D_object_call");
  P3D_object_eval_ptr = (P3D_object_eval_func *)get_func(module, "P3D_object_eval");
  P3D_object_incref_ptr = (P3D_object_incref_func *)get_func(module, "P3D_object_incref");
  P3D_object_decref_ptr = (P3D_object_decref_func *)get_func(module, "P3D_object_decref");
  P3D_make_class_definition_ptr = (P3D_make_class_definition_func *)get_func(module, "P3D_make_class_definition");
  P3D_new_undefined_object_ptr = (P3D_new_undefined_object_func *)get_func(module, "P3D_new_undefined_object");
  P3D_new_none_object_ptr = (P3D_new_none_object_func *)get_func(module, "P3D_new_none_object");
  P3D_new_bool_object_ptr = (P3D_new_bool_object_func *)get_func(module, "P3D_new_bool_object");
  P3D_new_int_object_ptr = (P3D_new_int_object_func *)get_func(module, "P3D_new_int_object");
  P3D_new_float_object_ptr = (P3D_new_float_object_func *)get_func(module, "P3D_new_float_object");
  P3D_new_string_object_ptr = (P3D_new_string_object_func *)get_func(module, "P3D_new_string_object");
  P3D_instance_get_panda_script_object_ptr = (P3D_instance_get_panda_script_object_func *)get_func(module, "P3D_instance_get_panda_script_object");
  P3D_instance_set_browser_script_object_ptr = (P3D_instance_set_browser_script_object_func *)get_func(module, "P3D_instance_set_browser_script_object");

  P3D_instance_get_request_ptr = (P3D_instance_get_request_func *)get_func(module, "P3D_instance_get_request");  
  P3D_check_request_ptr = (P3D_check_request_func *)get_func(module, "P3D_check_request");  
  P3D_request_finish_ptr = (P3D_request_finish_func *)get_func(module, "P3D_request_finish");  
  P3D_instance_feed_url_stream_ptr = (P3D_instance_feed_url_stream_func *)get_func(module, "P3D_instance_feed_url_stream");  
  P3D_instance_handle_event_ptr = (P3D_instance_handle_event_func *)get_func(module, "P3D_instance_handle_event");  

  #undef get_func

  // Successfully loaded.
  plugin_loaded = true;

  if (!init_plugin(contents_filename, host_url, 
                   verify_contents, platform,
                   log_directory, log_basename,
                   trusted_environment, console_environment,
                   root_dir, host_dir, logfile)) {
    unload_dso();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: init_plugin
//  Description: Ensures all the required function pointers have been
//               set, and then calls P3D_initialize() on the
//               recently-loaded plugin.  Returns true on success,
//               false on failure.
//
//               It is not necessary to call this after calling
//               load_plugin(); it is called implicitly.
////////////////////////////////////////////////////////////////////
bool
init_plugin(const string &contents_filename, const string &host_url, 
            P3D_verify_contents verify_contents, const string &platform,
            const string &log_directory, const string &log_basename,
            bool trusted_environment, bool console_environment,
            const string &root_dir, const string &host_dir,
            ostream &logfile) {

  // Ensure that all of the function pointers have been found.
  if (P3D_initialize_ptr == NULL ||
      P3D_finalize_ptr == NULL ||
      P3D_set_plugin_version_ptr == NULL ||
      P3D_set_super_mirror_ptr == NULL ||
      P3D_new_instance_ptr == NULL ||
      P3D_instance_start_ptr == NULL ||
      P3D_instance_start_stream_ptr == NULL ||
      P3D_instance_finish_ptr == NULL ||
      P3D_instance_setup_window_ptr == NULL ||

      P3D_object_get_type_ptr == NULL ||
      P3D_object_get_bool_ptr == NULL ||
      P3D_object_get_int_ptr == NULL ||
      P3D_object_get_float_ptr == NULL ||
      P3D_object_get_string_ptr == NULL ||
      P3D_object_get_repr_ptr == NULL ||
      P3D_object_get_property_ptr == NULL ||
      P3D_object_set_property_ptr == NULL ||
      P3D_object_has_method_ptr == NULL ||
      P3D_object_call_ptr == NULL ||
      P3D_object_eval_ptr == NULL ||
      P3D_object_incref_ptr == NULL ||
      P3D_object_decref_ptr == NULL ||

      P3D_make_class_definition_ptr == NULL ||
      P3D_new_undefined_object_ptr == NULL ||
      P3D_new_none_object_ptr == NULL ||
      P3D_new_bool_object_ptr == NULL ||
      P3D_new_int_object_ptr == NULL ||
      P3D_new_float_object_ptr == NULL ||
      P3D_new_string_object_ptr == NULL ||
      P3D_instance_get_panda_script_object_ptr == NULL ||
      P3D_instance_set_browser_script_object_ptr == NULL ||
      
      P3D_instance_get_request_ptr == NULL ||
      P3D_check_request_ptr == NULL ||
      P3D_request_finish_ptr == NULL ||
      P3D_instance_feed_url_stream_ptr == NULL ||
      P3D_instance_handle_event_ptr == NULL) {
    
    logfile
      << "Some function pointers not found:"
      << "\nP3D_initialize_ptr = " << P3D_initialize_ptr
      << "\nP3D_finalize_ptr = " << P3D_finalize_ptr
      << "\nP3D_set_plugin_version_ptr = " << P3D_set_plugin_version_ptr
      << "\nP3D_set_super_mirror_ptr = " << P3D_set_super_mirror_ptr
      << "\nP3D_new_instance_ptr = " << P3D_new_instance_ptr
      << "\nP3D_instance_start_ptr = " << P3D_instance_start_ptr
      << "\nP3D_instance_start_stream_ptr = " << P3D_instance_start_stream_ptr
      << "\nP3D_instance_finish_ptr = " << P3D_instance_finish_ptr
      << "\nP3D_instance_setup_window_ptr = " << P3D_instance_setup_window_ptr

      << "\nP3D_object_get_type_ptr = " << P3D_object_get_type_ptr
      << "\nP3D_object_get_bool_ptr = " << P3D_object_get_bool_ptr
      << "\nP3D_object_get_int_ptr = " << P3D_object_get_int_ptr
      << "\nP3D_object_get_float_ptr = " << P3D_object_get_float_ptr
      << "\nP3D_object_get_string_ptr = " << P3D_object_get_string_ptr
      << "\nP3D_object_get_repr_ptr = " << P3D_object_get_repr_ptr
      << "\nP3D_object_get_property_ptr = " << P3D_object_get_property_ptr
      << "\nP3D_object_set_property_ptr = " << P3D_object_set_property_ptr
      << "\nP3D_object_has_method_ptr = " << P3D_object_has_method_ptr
      << "\nP3D_object_call_ptr = " << P3D_object_call_ptr
      << "\nP3D_object_eval_ptr = " << P3D_object_eval_ptr
      << "\nP3D_object_incref_ptr = " << P3D_object_incref_ptr
      << "\nP3D_object_decref_ptr = " << P3D_object_decref_ptr

      << "\nP3D_make_class_definition_ptr = " << P3D_make_class_definition_ptr
      << "\nP3D_new_undefined_object_ptr = " << P3D_new_undefined_object_ptr
      << "\nP3D_new_none_object_ptr = " << P3D_new_none_object_ptr
      << "\nP3D_new_bool_object_ptr = " << P3D_new_bool_object_ptr
      << "\nP3D_new_int_object_ptr = " << P3D_new_int_object_ptr
      << "\nP3D_new_float_object_ptr = " << P3D_new_float_object_ptr
      << "\nP3D_new_string_object_ptr = " << P3D_new_string_object_ptr
      << "\nP3D_instance_get_panda_script_object_ptr = " << P3D_instance_get_panda_script_object_ptr
      << "\nP3D_instance_set_browser_script_object_ptr = " << P3D_instance_set_browser_script_object_ptr

      << "\nP3D_instance_get_request_ptr = " << P3D_instance_get_request_ptr
      << "\nP3D_check_request_ptr = " << P3D_check_request_ptr
      << "\nP3D_request_finish_ptr = " << P3D_request_finish_ptr
      << "\nP3D_instance_feed_url_stream_ptr = " << P3D_instance_feed_url_stream_ptr
      << "\nP3D_instance_handle_event_ptr = " << P3D_instance_handle_event_ptr
      << "\n";
    return false;
  }

  if (!P3D_initialize_ptr(P3D_API_VERSION, contents_filename.c_str(),
                          host_url.c_str(), verify_contents, platform.c_str(),
                          log_directory.c_str(), log_basename.c_str(),
                          trusted_environment, console_environment, 
                          root_dir.c_str(), host_dir.c_str())) {
    // Oops, failure to initialize.
    logfile
      << "Failed to initialize plugin (passed API version " 
      << P3D_API_VERSION << ")\n";
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
unload_plugin(ostream &logfile) {
  if (!plugin_loaded) {
    return;
  }

  P3D_finalize_ptr();
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
  if (dso_needs_unload) {
    assert(module != NULL);
#ifdef _WIN32
    FreeLibrary(module);
#else 
    dlclose(module);
#endif
    module = NULL;
    dso_needs_unload = false;
  }
  
  P3D_initialize_ptr = NULL;
  P3D_finalize_ptr = NULL;
  P3D_set_plugin_version_ptr = NULL;
  P3D_set_super_mirror_ptr = NULL;
  P3D_new_instance_ptr = NULL;
  P3D_instance_start_ptr = NULL;
  P3D_instance_start_stream_ptr = NULL;
  P3D_instance_finish_ptr = NULL;
  P3D_instance_setup_window_ptr = NULL;

  P3D_object_get_type_ptr = NULL;
  P3D_object_get_bool_ptr = NULL;
  P3D_object_get_int_ptr = NULL;
  P3D_object_get_float_ptr = NULL;
  P3D_object_get_string_ptr = NULL;
  P3D_object_get_repr_ptr = NULL;
  P3D_object_get_property_ptr = NULL;
  P3D_object_set_property_ptr = NULL;
  P3D_object_has_method_ptr = NULL;
  P3D_object_call_ptr = NULL;
  P3D_object_eval_ptr = NULL;
  P3D_object_incref_ptr = NULL;
  P3D_object_decref_ptr = NULL;

  P3D_make_class_definition_ptr = NULL;
  P3D_new_undefined_object_ptr = NULL;
  P3D_new_none_object_ptr = NULL;
  P3D_new_bool_object_ptr = NULL;
  P3D_new_int_object_ptr = NULL;
  P3D_new_float_object_ptr = NULL;
  P3D_new_string_object_ptr = NULL;
  P3D_instance_get_panda_script_object_ptr = NULL;
  P3D_instance_set_browser_script_object_ptr = NULL;

  P3D_instance_get_request_ptr = NULL;
  P3D_check_request_ptr = NULL;
  P3D_request_finish_ptr = NULL;
  P3D_instance_feed_url_stream_ptr = NULL;
  P3D_instance_handle_event_ptr = NULL;

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
