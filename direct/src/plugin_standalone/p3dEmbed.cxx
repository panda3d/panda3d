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

#ifdef _WIN32
unsigned long p3d_offset = 0xFF3D3D00;
#else
#include <stdint.h>
uint32_t p3d_offset = 0xFF3D3D00;
#endif

////////////////////////////////////////////////////////////////////
//     Function: P3DEmbed::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DEmbed::
P3DEmbed(bool console_environment) : Panda3DBase(console_environment) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DEmbed::run_embedded
//       Access: Public
//  Description: Runs with the data embedded in the current
//               executable, at the specified offset.
////////////////////////////////////////////////////////////////////
int P3DEmbed::
run_embedded(streampos read_offset, int argc, char *argv[]) {
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
        } else if (keyword == "height") {
          _win_height = atoi(value.c_str());
        } else if (keyword == "root_dir") {
          root_dir = value;
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
  Filename root_dir_f(root_dir);
  root_dir_f.make_absolute(f.get_dirname());

  // Initialize the plugin.  Since we are linked with the core API
  // statically, we pass the empty string as the plugin filename, and
  // pull the required symbols out of the local address space.
  if (!load_plugin("", "",
                   _host_url, _verify_contents, _this_platform, _log_dirname,
                   _log_basename, true, _console_environment,
                   root_dir_f.to_os_specific(), cerr)) {
    cerr << "Unable to launch core API\n";
    return 1;
  }
  
  // Create a plugin instance and run the program
  P3D_instance *inst = create_instance
    (f, true, _win_x, _win_y, _win_width, _win_height,
     argv, argc, read_offset);
  _instances.insert(inst);
  
  run_main_loop();

  // Though it's not strictly necessary to call P3D_finalize() here
  // (because unload_plugin() will call it), we have to do it anyway,
  // to force the contents of libp3d_plugin_static.lib to be linked
  // in.  If we don't appear to make any calls to these functions,
  // then the linker may decide to omit all of them.
  P3D_finalize();

  unload_plugin();
  return 0;
}

int
main(int argc, char *argv[]) {
  // Check to see if we've actually got an application embedded.  If
  // we do, p3d_offset will have been modified to contain a different
  // value than the one we compiled in, above.  We test against
  // p3d_offset + 1, because any appearances of this exact number
  // within the binary will be replaced (including this one).
  if (p3d_offset + 1 == 0xFF3D3D01) {
    cerr << "This program is not intended to be run directly.\nIt is used by pdeploy to construct an embedded Panda3D application.\n";
    return 1;
  }

  P3DEmbed program(true);
  return program.run_embedded(p3d_offset, argc, argv);
}

