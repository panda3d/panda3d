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

extern P3D_initialize_func *P3D_initialize;
extern P3D_finalize_func *P3D_finalize;
extern P3D_new_instance_func *P3D_new_instance;
extern P3D_instance_start_func *P3D_instance_start;
extern P3D_instance_finish_func *P3D_instance_finish;
extern P3D_instance_setup_window_func *P3D_instance_setup_window;

extern P3D_make_class_definition_func *P3D_make_class_definition;
extern P3D_new_none_object_func *P3D_new_none_object;
extern P3D_new_bool_object_func *P3D_new_bool_object;
extern P3D_new_int_object_func *P3D_new_int_object;
extern P3D_new_float_object_func *P3D_new_float_object;
extern P3D_new_string_object_func *P3D_new_string_object;
extern P3D_new_list_object_func *P3D_new_list_object;
extern P3D_instance_get_script_object_func *P3D_instance_get_script_object;
extern P3D_instance_set_script_object_func *P3D_instance_set_script_object;

extern P3D_instance_get_request_func *P3D_instance_get_request;
extern P3D_check_request_func *P3D_check_request;
extern P3D_request_finish_func *P3D_request_finish;
extern P3D_instance_feed_url_stream_func *P3D_instance_feed_url_stream;

string get_plugin_basename();
bool load_plugin(const string &p3d_plugin_filename);
void unload_plugin();
bool is_plugin_loaded();

#endif
