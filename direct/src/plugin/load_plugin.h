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
extern P3D_new_instance_func *P3D_new_instance;
extern P3D_instance_start_func *P3D_instance_start;
extern P3D_instance_finish_func *P3D_instance_finish;
extern P3D_instance_setup_window_func *P3D_instance_setup_window;

extern P3D_value_finish_func *P3D_value_finish;
extern P3D_value_copy_func *P3D_value_copy;
extern P3D_new_none_value_func *P3D_new_none_value;
extern P3D_new_bool_value_func *P3D_new_bool_value;
extern P3D_value_get_bool_func *P3D_value_get_bool;
extern P3D_new_int_value_func *P3D_new_int_value;
extern P3D_value_get_int_func *P3D_value_get_int;
extern P3D_new_float_value_func *P3D_new_float_value;
extern P3D_value_get_float_func *P3D_value_get_float;
extern P3D_new_string_value_func *P3D_new_string_value;
extern P3D_value_get_string_length_func *P3D_value_get_string_length;
extern P3D_value_extract_string_func *P3D_value_extract_string;
extern P3D_new_list_value_func *P3D_new_list_value;
extern P3D_value_get_list_length_func *P3D_value_get_list_length;
extern P3D_value_get_list_item_func *P3D_value_get_list_item;
extern P3D_instance_get_property_func *P3D_instance_get_property;
extern P3D_instance_get_property_list_func *P3D_instance_get_property_list;
extern P3D_instance_set_property_func *P3D_instance_set_property;
extern P3D_instance_call_func *P3D_instance_call;

extern P3D_instance_get_request_func *P3D_instance_get_request;
extern P3D_check_request_func *P3D_check_request;
extern P3D_request_finish_func *P3D_request_finish;
extern P3D_instance_feed_url_stream_func *P3D_instance_feed_url_stream;
extern P3D_instance_feed_value_func *P3D_instance_feed_value;

string get_plugin_basename();
bool load_plugin(const string &p3d_plugin_filename);
void unload_plugin();
bool is_plugin_loaded();

#endif
