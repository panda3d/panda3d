// Filename: p3dEmbed.cxx
// Created by:  rdb (07Dec09)
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

#define P3D_FUNCTION_PROTOTYPES

#include "p3dEmbed.h"
#include "load_plugin.h"
#include "find_root_dir.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DEmbed::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DEmbed::
P3DEmbed(bool console_environment) : Panda3DBase(console_environment) {
  // Since the Panda3DBase constructor no longer assigns _root_dir, we
  // have to do it here.
  _root_dir = find_root_dir();
  
  // We should leave the arguments intact, just pass them
  // 1:1 as we've received them.
  _prepend_filename_to_args = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DEmbed::run_embedded
//       Access: Public
//  Description: Runs with the data embedded in the current
//               executable, at the specified offset.
////////////////////////////////////////////////////////////////////
int P3DEmbed::
run_embedded(streampos read_offset, int argc, char *argv[]) {
  // Check to see if we've actually got an application embedded.  If
  // we do, read_offset will have been modified to contain a different
  // value than the one we compiled in, above.  We test against
  // read_offset + 1, because any appearances of this exact number
  // within the binary will be replaced (including this one).

  // We also have to store this computation in a member variable, to
  // work around a compiler optimization that might otherwise remove
  // the + 1 from the test.
  _read_offset_check = read_offset + (streampos)1;
  if (_read_offset_check == (streampos)0xFF3D3D01) {
    cerr << "This program is not intended to be run directly.\nIt is used by pdeploy to construct an embedded Panda3D application.\n";
    return 1;
  }

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
  int curchr = read.get();
  if (curchr == EOF) {
    cerr << "Couldn't seek to " << read_offset << "\n";
    return 1;
  }

  string curstr;
  bool havenull = false;
  P3D_token token;
  token._keyword = NULL;
  token._value = NULL;
  string keyword;
  string value;
  string root_dir;
  string host_dir;
  while (true) {
    if (curchr == EOF) {
      cerr << "Truncated stream\n";
      return(1);

    } else if (curchr == 0) {
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
        token._keyword = strdup(keyword.c_str());
        token._value = strdup(value.c_str());
        _tokens.push_back(token);

        // Read out the tokens that may interest us
        if (keyword == "width") {
          _win_width = atoi(value.c_str());
          _got_win_size = true;
        } else if (keyword == "height") {
          _win_height = atoi(value.c_str());
          _got_win_size = true;
        } else if (keyword == "log_basename") {
          _log_basename = value;
        } else if (keyword == "root_dir") {
          root_dir = value;
        } else if (keyword == "host_dir") {
          host_dir = value;
        } else if (keyword == "verify_contents") {
          if (value == "never") {
            _verify_contents = P3D_VC_never;
          } else if (value == "force") {
            _verify_contents = P3D_VC_force;
          } else if (value == "normal") {
            _verify_contents = P3D_VC_normal;
          } else if (value == "none") {
            _verify_contents = P3D_VC_none;
          } else {
            _verify_contents = (P3D_verify_contents)atoi(value.c_str());
          }
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
    curchr = read.get();
  }

  // Update the offset to the current read pointer.
  // This is where the multifile really starts.
  read_offset = read.tellg();
  read.close();

  // Make the root directory absolute
  if (!root_dir.empty()) {
    Filename root_dir_f(root_dir);
    root_dir_f.make_absolute(f.get_dirname());
    _root_dir = root_dir_f.to_os_specific();
  }
  
  // Make the host directory absolute
  if (!host_dir.empty()) {
    Filename host_dir_f(host_dir);
    host_dir_f.make_absolute(f.get_dirname());
    _host_dir = host_dir_f.to_os_specific();
  }

  // Initialize the core API by directly assigning all of the function
  // pointers.
  P3D_initialize_ptr = &P3D_initialize;
  P3D_finalize_ptr = &P3D_finalize;
  P3D_set_plugin_version_ptr = &P3D_set_plugin_version;
  P3D_set_super_mirror_ptr = &P3D_set_super_mirror;
  P3D_new_instance_ptr = &P3D_new_instance;
  P3D_instance_start_ptr = &P3D_instance_start;
  P3D_instance_start_stream_ptr = &P3D_instance_start_stream;
  P3D_instance_finish_ptr = &P3D_instance_finish;
  P3D_instance_setup_window_ptr = &P3D_instance_setup_window;

  P3D_object_get_type_ptr = &P3D_object_get_type;
  P3D_object_get_bool_ptr = &P3D_object_get_bool;
  P3D_object_get_int_ptr = &P3D_object_get_int;
  P3D_object_get_float_ptr = &P3D_object_get_float;
  P3D_object_get_string_ptr = &P3D_object_get_string;
  P3D_object_get_repr_ptr = &P3D_object_get_repr;
  P3D_object_get_property_ptr = &P3D_object_get_property;
  P3D_object_set_property_ptr = &P3D_object_set_property;
  P3D_object_has_method_ptr = &P3D_object_has_method;
  P3D_object_call_ptr = &P3D_object_call;
  P3D_object_eval_ptr = &P3D_object_eval;
  P3D_object_incref_ptr = &P3D_object_incref;
  P3D_object_decref_ptr = &P3D_object_decref;

  P3D_make_class_definition_ptr = &P3D_make_class_definition;
  P3D_new_undefined_object_ptr = &P3D_new_undefined_object;
  P3D_new_none_object_ptr = &P3D_new_none_object;
  P3D_new_bool_object_ptr = &P3D_new_bool_object;
  P3D_new_int_object_ptr = &P3D_new_int_object;
  P3D_new_float_object_ptr = &P3D_new_float_object;
  P3D_new_string_object_ptr = &P3D_new_string_object;
  P3D_instance_get_panda_script_object_ptr = &P3D_instance_get_panda_script_object;
  P3D_instance_set_browser_script_object_ptr = &P3D_instance_set_browser_script_object;

  P3D_instance_get_request_ptr = &P3D_instance_get_request;
  P3D_check_request_ptr = &P3D_check_request;
  P3D_request_finish_ptr = &P3D_request_finish;
  P3D_instance_feed_url_stream_ptr = &P3D_instance_feed_url_stream;
  P3D_instance_handle_event_ptr = &P3D_instance_handle_event;

  // Calling the executable with --prep just prepares the directory
  // structure, this is usually invoked in the installer.
  if (argc == 2 && strcmp(argv[1], "--prep") == 0) {
    cerr << "Invoking the prepare step is deprecated, please rebuild the application using a more recent version of pdeploy\n";
    _window_type = P3D_WT_hidden;
    _log_basename = "prep";
    P3D_token token;
    token._keyword = "stop_on_ready";
    token._value = "1";
    _tokens.push_back(token);
    token._keyword = "hidden";
    token._value = "1";
    _tokens.push_back(token);
  }

  // Now call init_plugin() to verify that we got all of the required
  // function pointers.  This will also call P3D_initialize().
  if (!init_plugin("", _host_url, _verify_contents, _this_platform, 
                   _log_dirname, _log_basename, true, _console_environment,
                   _root_dir, _host_dir, cerr)) {
    cerr << "Unable to launch core API\n";
    return 1;
  }
  
  // Create a plugin instance and run the program
  P3D_instance *inst = create_instance(f, true, argv, argc, read_offset);
  _instances.insert(inst);
  
  run_main_loop();


  unload_plugin(cerr);
  return 0;
}
