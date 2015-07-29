// Filename: paramNodePath.h
// Created by:  rdb (25Feb15)
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

#ifndef PARAMNODEPATH_H
#define PARAMNODEPATH_H

#include "pandabase.h"
#include "paramValue.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : ParamNodePath
// Description : A class object for storing a NodePath as a parameter.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ ParamNodePath : public ParamValueBase {
protected:
  INLINE ParamNodePath() {};

PUBLISHED:
  INLINE ParamNodePath(const NodePath &node_path);

#ifdef USE_MOVE_SEMANTICS
  INLINE ParamNodePath(NodePath &&node_path) NOEXCEPT;
#endif

  INLINE virtual TypeHandle get_value_type() const;
  INLINE const NodePath &get_value() const;

  virtual void output(ostream &out) const;

private:
  NodePath _node_path;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParamValueBase::init_type();
    register_type(_type_handle, "ParamNodePath",
                  ParamValueBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "paramNodePath.I"

#endif
