// Filename: animGroup.h
// Created by:  drose (21Feb99)
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

#ifndef ANIMGROUP_H
#define ANIMGROUP_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "namable.h"
#include "luse.h"

class AnimBundle;
class BamReader;
class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : AnimGroup
// Description : This is the base class for AnimChannel and
//               AnimBundle.  It implements a hierarchy of
//               AnimChannels.  The root of the hierarchy must be an
//               AnimBundle.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_CHAN AnimGroup : public TypedWritableReferenceCount, public Namable {
protected:
  AnimGroup(const string &name = "");
  AnimGroup(AnimGroup *parent, const AnimGroup &copy);

PUBLISHED:
  // This is the normal AnimGroup constructor.
  AnimGroup(AnimGroup *parent, const string &name);
  virtual ~AnimGroup();

  int get_num_children() const;
  AnimGroup *get_child(int n) const;
  MAKE_SEQ(get_children, get_num_children, get_child);

  AnimGroup *get_child_named(const string &name) const;
  AnimGroup *find_child(const string &name) const;
  void sort_descendants();

public:
  virtual TypeHandle get_value_type() const;

PUBLISHED:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

protected:
  void write_descendants(ostream &out, int indent_level) const;

  virtual AnimGroup *make_copy(AnimGroup *parent) const;
  PT(AnimGroup) copy_subtree(AnimGroup *parent) const;
  
protected:
  typedef pvector< PT(AnimGroup) > Children;
  Children _children;
  AnimBundle *_root;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

  static TypedWritable *make_AnimGroup(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

private:
  typedef pvector< string > frozenJoints;
  int _num_children;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "AnimGroup",
                  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

inline ostream &operator << (ostream &out, const AnimGroup &anim) {
  anim.output(out);
  return out;
}

#include "animGroup.I"

#endif


