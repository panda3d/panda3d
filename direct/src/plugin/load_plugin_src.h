// Filename: load_plugin_src.h
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

extern P3D_initialize_func *P3D_initialize;
extern P3D_free_string_func *P3D_free_string;
extern P3D_create_instance_func *P3D_create_instance;
extern P3D_instance_finish_func *P3D_instance_finish;
extern P3D_instance_has_property_func *P3D_instance_has_property;
extern P3D_instance_get_property_func *P3D_instance_get_property;
extern P3D_instance_set_property_func *P3D_instance_set_property;
extern P3D_instance_get_request_func *P3D_instance_get_request;
extern P3D_check_request_func *P3D_check_request;
extern P3D_request_finish_func *P3D_request_finish;
extern P3D_instance_feed_url_stream_func *P3D_instance_feed_url_stream;
