// Filename: xFileAnimationSet.h
// Created by:  drose (02Oct04)
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

#ifndef XFILEANIMATIONSET_H
#define XFILEANIMATIONSET_H

#include "pandatoolbase.h"
#include "pmap.h"
#include "epvector.h"
#include "luse.h"
#include "namable.h"

class XFileToEggConverter;
class EggGroup;
class EggTable;
class EggXfmSAnim;

////////////////////////////////////////////////////////////////////
//       Class : XFileAnimationSet
// Description : This represents a tree of EggTables, corresponding to
//               Animation entries in the X file.  There is one
//               EggTable for each joint in the character's joint
//               set, and the whole tree is structured as a
//               mirror of the joint set.
////////////////////////////////////////////////////////////////////
class XFileAnimationSet : public Namable {
public:
  XFileAnimationSet();
  ~XFileAnimationSet();

  bool create_hierarchy(XFileToEggConverter *converter);
  EggXfmSAnim *get_table(const string &joint_name) const;

  enum FrameDataFlags {
    FDF_scale    = 0x01,
    FDF_rot      = 0x02,
    FDF_trans    = 0x04,
    FDF_mat      = 0x08,
  };

  class FrameEntry {
  public:
    INLINE FrameEntry();
    INLINE const LMatrix4d &get_mat(int flags) const;

    LVecBase3d _scale;
    LQuaterniond _rot;
    LVector3d _trans;
    LMatrix4d _mat;
  };

  typedef epvector<FrameEntry> FrameEntries;

  class FrameData {
  public:
    INLINE FrameData();
    FrameEntries _entries;
    int _flags;
  };
  
  FrameData &create_frame_data(const string &joint_name);

public:
  double _frame_rate;

private:
  void mirror_table(XFileToEggConverter *converter,
                    EggGroup *model_node, EggTable *anim_node);

  typedef pmap<string, FrameData> JointData;
  JointData _joint_data;

  class TablePair {
  public:
    EggGroup *_joint;
    EggXfmSAnim *_table;
  };

  typedef pmap<string, TablePair> Tables;
  Tables _tables;
};

#include "xFileAnimationSet.I"

#endif

