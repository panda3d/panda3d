// Filename: modelPool.h
// Created by:  drose (25Aug00)
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

#ifndef MODELPOOL_H
#define MODELPOOL_H

#include <pandabase.h>

#include <filename.h>
#include <pt_Node.h>

#include <map>

////////////////////////////////////////////////////////////////////
//       Class : ModelPool
// Description : This is the preferred interface for loading models.
//               It unifies all references to the same filename, so
//               that multiple attempts to load the same model will
//               return the same pointer.  Note that the default
//               behavior is thus to make instances: use with caution.
//               Use the copy_subgraph() method on Node (or use
//               NodePath::copy_to) to make modifiable copies of the
//               node.
//
//               Unlike TexturePool, this class does not automatically
//               resolve the model filenames before loading, so a
//               relative path and an absolute path to the same model
//               will appear to be different filenames.
//
//               This does not presently support asynchronous loading,
//               although it wouldn't be *too* difficult to add.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ModelPool {
PUBLISHED:
  INLINE static bool has_model(const string &filename);
  INLINE static bool verify_model(const string &filename);
  INLINE static PT_Node load_model(const string &filename);

  INLINE static void add_model(const string &filename, Node *model);
  INLINE static void release_model(const string &filename);
  INLINE static void release_all_models();

  INLINE static int garbage_collect();

  INLINE static void list_contents(ostream &out);

private:
  INLINE ModelPool();

  bool ns_has_model(const string &filename);
  PT_Node ns_load_model(const string &filename);
  void ns_add_model(const string &filename, Node *model);
  void ns_release_model(const string &filename);
  void ns_release_all_models();
  int ns_garbage_collect();
  void ns_list_contents(ostream &out);

  static ModelPool *get_ptr();

  static ModelPool *_global_ptr;
  typedef map<string,  PT(Node) > Models;
  Models _models;
};

#include "modelPool.I"

#endif


