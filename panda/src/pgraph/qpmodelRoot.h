// Filename: qpmodelRoot.h
// Created by:  drose (16Mar02)
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

#ifndef qpMODELROOT_H
#define qpMODELROOT_H

#include "pandabase.h"

#include "qpmodelNode.h"

////////////////////////////////////////////////////////////////////
//       Class : qpModelRoot
// Description : A node of this type is created automatically at the
//               root of each model file that is loaded.  It may
//               eventually contain some information about the
//               contents of the model; at the moment, it contains no
//               special information, but can be used as a flag to
//               indicate the presence of a loaded model file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpModelRoot : public qpModelNode {
PUBLISHED:
  INLINE qpModelRoot(const string &name );

protected:
  INLINE qpModelRoot(const qpModelRoot &copy);

public:
  virtual PandaNode *make_copy() const;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpModelNode::init_type();
    register_type(_type_handle, "qpModelRoot",
                  qpModelNode::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "qpmodelRoot.I"

#endif


