// Filename: xFileMaker.h
// Created by:  drose (19Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef XFILEMAKER_H
#define XFILEMAKER_H

#include <pandatoolbase.h>

#include <filename.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d.h>
#include <dxfile.h>
#include <rmxfguid.h>
#undef WIN32_LEAN_AND_MEAN

class EggNode;
class EggGroupNode;
class EggGroup;
class EggBin;
class EggData;

////////////////////////////////////////////////////////////////////
//       Class : XFileMaker
// Description : This class converts a Panda scene graph into a .X
//               file and writes it out.
////////////////////////////////////////////////////////////////////
class XFileMaker {
public:
  XFileMaker();
  ~XFileMaker();

  bool open(const Filename &filename);
  void close();

  bool add_tree(EggData &egg_data);

private:
  bool add_node(EggNode *egg_node, LPDIRECTXFILEDATA dx_parent);
  bool add_group(EggGroup *egg_group, LPDIRECTXFILEDATA dx_parent);
  bool add_bin(EggBin *egg_bin, LPDIRECTXFILEDATA dx_parent);
  bool add_polyset(EggBin *egg_bin, LPDIRECTXFILEDATA dx_parent);

  bool recurse_nodes(EggGroupNode *egg_node, LPDIRECTXFILEDATA dx_parent);
  bool create_frame(LPDIRECTXFILEDATA &data, const string &name);
  bool attach_and_release(LPDIRECTXFILEDATA data, LPDIRECTXFILEDATA dx_parent);

  static string make_nice_name(const string &str);

  LPDIRECTXFILE _dx_file;
  LPDIRECTXFILESAVEOBJECT _dx_file_save;

  int _mesh_index;
};

#endif

