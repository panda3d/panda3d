// Filename: load_plugin.h
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

#ifndef LOAD_PLUGIN_H
#define LOAD_PLUGIN_H

#include "p3d_plugin.h"

#include <string>
using namespace std;

extern P3D_initialize_func *P3D_initialize_ptr;
extern P3D_finalize_func *P3D_finalize_ptr;
extern P3D_set_plugin_version_func *P3D_set_plugin_version_ptr;
extern P3D_set_super_mirror_func *P3D_set_super_mirror_ptr;
extern P3D_new_instance_func *P3D_new_instance_ptr;
extern P3D_instance_start_func *P3D_instance_start_ptr;
extern P3D_instance_start_stream_func *P3D_instance_start_stream_ptr;
extern P3D_instance_finish_func *P3D_instance_finish_ptr;
extern P3D_instance_setup_window_func *P3D_instance_setup_window_ptr;

extern P3D_object_get_type_func *P3D_object_get_type_ptr;
extern P3D_object_get_bool_func *P3D_object_get_bool_ptr;
extern P3D_object_get_int_func *P3D_object_get_int_ptr;
extern P3D_object_get_float_func *P3D_object_get_float_ptr;
extern P3D_object_get_string_func *P3D_object_get_string_ptr;
extern P3D_object_get_repr_func *P3D_object_get_repr_ptr;
extern P3D_object_get_property_func *P3D_object_get_property_ptr;
extern P3D_object_set_property_func *P3D_object_set_property_ptr;
extern P3D_object_has_method_func *P3D_object_has_method_ptr;
extern P3D_object_call_func *P3D_object_call_ptr;
extern P3D_object_eval_func *P3D_object_eval_ptr;
extern P3D_object_incref_func *P3D_object_incref_ptr;
extern P3D_object_decref_func *P3D_object_decref_ptr;

extern P3D_make_class_definition_func *P3D_make_class_definition_ptr;
extern P3D_new_undefined_object_func *P3D_new_undefined_object_ptr;
extern P3D_new_none_object_func *P3D_new_none_object_ptr;
extern P3D_new_bool_object_func *P3D_new_bool_object_ptr;
extern P3D_new_int_object_func *P3D_new_int_object_ptr;
extern P3D_new_float_object_func *P3D_new_float_object_ptr;
extern P3D_new_string_object_func *P3D_new_string_object_ptr;
extern P3D_instance_get_panda_script_object_func *P3D_instance_get_panda_script_object_ptr;
extern P3D_instance_set_browser_script_object_func *P3D_instance_set_browser_script_object_ptr;

extern P3D_instance_get_request_func *P3D_instance_get_request_ptr;
extern P3D_check_request_func *P3D_check_request_ptr;
extern P3D_request_finish_func *P3D_request_finish_ptr;
extern P3D_instance_feed_url_stream_func *P3D_instance_feed_url_stream_ptr;
extern P3D_instance_handle_event_func *P3D_instance_handle_event_ptr;

string get_plugin_basename();
bool 
load_plugin(const string &p3d_plugin_filename, 
            const string &contents_filename, const string &host_url,
            P3D_verify_contents verify_contents, const string &platform,
            const string &log_directory, const string &log_basename,
            bool trusted_environment, bool console_environment,
            const string &root_dir, const string &host_dir, ostream &logfile);
bool
init_plugin(const string &contents_filename, const string &host_url, 
            P3D_verify_contents verify_contents, const string &platform,
            const string &log_directory, const string &log_basename,
            bool trusted_environment, bool console_environment,
            const string &root_dir, const string &host_dir, ostream &logfile);

void unload_plugin(ostream &logfile);
bool is_plugin_loaded();

#endif
