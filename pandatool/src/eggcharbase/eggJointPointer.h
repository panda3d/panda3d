// Filename: eggJointPointer.h
// Created by:  drose (26Feb01)
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

#ifndef EGGJOINTPOINTER_H
#define EGGJOINTPOINTER_H

#include "pandatoolbase.h"

#include "eggBackPointer.h"
#include "eggGroup.h"
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

  virtual void do_finish_reparent(EggJointPointer *new_parent)=0;
  virtual void move_vertices_to(EggJointPointer *new_joint);

  void begin_rebuild();
  virtual bool add_rebuild_frame(const LMatrix4d &mat);
  INLINE int get_num_rebuild_frames() const;
  INLINE const LMatrix4d &get_rebuild_frame(int n) const;
  virtual bool do_rebuild();

  INLINE void clear_net_frames();
  INLINE void add_net_frame(const LMatrix4d &mat);
  INLINE int get_num_net_frames() const;
  INLINE const LMatrix4d &get_net_frame(int n) const;

  INLINE void clear_net_frame_invs();
  INLINE void add_net_frame_inv(const LMatrix4d &mat);
  INLINE int get_num_net_frame_invs() const;
  INLINE const LMatrix4d &get_net_frame_inv(int n) const;

  virtual void optimize();
  virtual void expose(EggGroup::DCSType dcs_type);
  virtual void zero_channels(const string &components);

  virtual EggJointPointer *make_new_joint(const string &name)=0;

protected:
  typedef pvector<LMatrix4d> RebuildFrames;
  RebuildFrames _rebuild_frames;
  RebuildFrames _net_frames;
  RebuildFrames _net_frame_invs;

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

#include "eggJointPointer.I"

#endif


