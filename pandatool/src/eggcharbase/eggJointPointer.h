// Filename: eggJointPointer.h
// Created by:  drose (26Feb01)
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

#ifndef EGGJOINTPOINTER_H
#define EGGJOINTPOINTER_H

#include "pandatoolbase.h"

#include "eggBackPointer.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : EggJointPointer
// Description : This is a base class for EggJointNodePointer and
//               EggMatrixTablePointer.  It stores a back pointer to
//               either a <Joint> entry or an xform <Table> data, and
//               thus presents an interface that returns 1-n matrices,
//               one for each frame.  (<Joint> entries, for model
//               files, appear the same as one-frame animations.)
////////////////////////////////////////////////////////////////////
class EggJointPointer : public EggBackPointer {
public:
  virtual int get_num_frames() const=0;
  virtual LMatrix4d get_frame(int n) const=0;
  virtual void set_frame(int n, const LMatrix4d &mat)=0;
  virtual bool add_frame(const LMatrix4d &mat);

  void begin_rebuild();
  virtual bool add_rebuild_frame(const LMatrix4d &mat);
  virtual bool do_rebuild();

  virtual void optimize();

protected:
  typedef pvector<LMatrix4d> RebuildFrames;
  RebuildFrames _rebuild_frames;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggBackPointer::init_type();
    register_type(_type_handle, "EggJointPointer",
                  EggBackPointer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif


