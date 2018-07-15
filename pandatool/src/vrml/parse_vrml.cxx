/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parse_vrml.cxx
 * @author drose
 * @date 2004-10-01
 */

/**************************************************
 * VRML 2.0, Draft 2 Parser
 * Copyright (C) 1996 Silicon Graphics, Inc.
 *
 * Author(s)    : Gavin Bell
 *                Daniel Woods (first port)
 **************************************************
 */

#include "pandatoolbase.h"

#include "parse_vrml.h"
#include "vrmlParserDefs.h"
#include "vrmlNodeType.h"
#include "vrmlNode.h"
#include "standard_nodes.h"
#include "zStream.h"
#include "virtualFileSystem.h"

using std::istream;
using std::istringstream;
using std::string;

extern int vrmlyyparse();
extern void vrmlyyResetLineNumber();
extern int vrmlyydebug;
extern int vrmlyy_flex_debug;

extern VrmlScene *parsed_scene;

/**
 * Loads the set of standard VRML node definitions into the parser, if it has
 * not already been loaded.
 */
static bool
get_standard_nodes() {
  static bool got_standard_nodes = false;
  static bool read_ok = true;
  if (got_standard_nodes) {
    return read_ok;
  }

  // The standardNodes.wrl file has been compiled into this binary.  Extract
  // it out.

  string data((const char *)standard_nodes_data, standard_nodes_data_len);

#ifdef HAVE_ZLIB
  // The data is stored compressed; decompress it on-the-fly.
  istringstream inz(data);
  IDecompressStream in(&inz, false);

#else
  // The data is stored uncompressed, so just load it.
  istringstream in(data);
#endif  // HAVE_ZLIB

  vrml_init_parser(in, "standardNodes.wrl");
  if (vrmlyyparse() != 0) {
    read_ok = false;
  }
  vrml_cleanup_parser();

  got_standard_nodes = true;
  return read_ok;
}

/**
 * Reads the named VRML file and returns a corresponding VrmlScene, or NULL if
 * there is a parse error.
 */
VrmlScene *
parse_vrml(Filename filename) {
  filename.set_text();
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *in = vfs->open_read_file(filename, true);
  if (in == nullptr) {
    nout << "Cannot open " << filename << " for reading.\n";
    return nullptr;
  }
  VrmlScene *result = parse_vrml(*in, filename);
  vfs->close_read_file(in);
  return result;
}

/**
 * Reads the indicated input stream and returns a corresponding VrmlScene, or
 * NULL if there is a parse error.
 */
VrmlScene *
parse_vrml(istream &in, const string &filename) {
  if (!get_standard_nodes()) {
    std::cerr << "Internal error--unable to parse VRML.\n";
    return nullptr;
  }

  VrmlScene *scene = nullptr;
  VrmlNodeType::pushNameSpace();

  vrml_init_parser(in, filename);
  if (vrmlyyparse() == 0) {
    scene = parsed_scene;
  }
  vrml_cleanup_parser();

  VrmlNodeType::popNameSpace();

  return scene;
}

#if 0
int
main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "parse_vrml filename.wrl\n";
    exit(1);
  }

  VrmlScene *scene = parse_vrml(argv[1]);
  if (scene == nullptr) {
    exit(1);
  }

  std::cout << *scene << "\n";
  return (0);
}
#endif
