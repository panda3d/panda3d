// Filename: xFileMaker.h
// Created by:  drose (19Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef XFILEMAKER_H
#define XFILEMAKER_H

#include "pandatoolbase.h"

#include "filename.h"
#include "pmap.h"
#include "luse.h"

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
class EggVertexPool;
class Datagram;
class XFileMesh;

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

  bool create_object(LPDIRECTXFILEDATA &obj, REFGUID template_id,
                     const string &name, const Datagram &dg);
  bool create_frame(LPDIRECTXFILEDATA &obj, const string &name);
  bool add_frame_transform(LPDIRECTXFILEDATA obj, const LMatrix4f &mat);

  bool attach_and_release(LPDIRECTXFILEDATA obj, LPDIRECTXFILEDATA dx_parent);

  static string make_nice_name(const string &str);

  XFileMesh *get_mesh(LPDIRECTXFILEDATA dx_parent);
  bool finalize_mesh(LPDIRECTXFILEDATA dx_parent);

  LPDIRECTXFILE _dx_file;
  LPDIRECTXFILESAVEOBJECT _dx_file_save;

  int _mesh_index;

  typedef pmap<LPDIRECTXFILEDATA, XFileMesh *> Meshes;
  Meshes _meshes;
};

#endif

