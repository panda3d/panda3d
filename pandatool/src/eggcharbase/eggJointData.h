// Filename: eggJointData.h
// Created by:  drose (23Feb01)
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

#ifndef EGGJOINTDATA_H
#define EGGJOINTDATA_H

#include "pandatoolbase.h"

#include "eggComponentData.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : EggJointData
// Description : This is one node of a hierarchy of EggJointData
//               nodes, each of which represents a single joint of the
//               character hierarchy across all loaded files: the
//               various models, the LOD's of each model, and the
//               various animation channel files.
////////////////////////////////////////////////////////////////////
class EggJointData : public EggComponentData {
public:
  EggJointData(EggCharacterCollection *collection,
               EggCharacterData *char_data);

  INLINE int get_num_children() const;
  INLINE EggJointData *get_child(int n) const;
  EggJointData *find_joint(const string &name);

  int get_num_frames(int model_index) const;
  LMatrix4d get_frame(int model_index, int n) const;
  LMatrix4d get_net_frame(int model_index, int n) const;

  bool do_rebuild();
  void optimize();

  virtual void add_back_pointer(int model_index, EggObject *egg_object);
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  typedef pvector<EggJointData *> Children;
  Children _children;
  EggJointData *_parent;

  friend class EggCharacterCollection;
};

#include "eggJointData.I"

#endif
