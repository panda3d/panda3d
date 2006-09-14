// Filename: eventStorePandaNode.h
// Created by:  drose (13Sep06)
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

#ifndef EVENTSTOREPANDANODE_H
#define EVENTSTOREPANDANODE_H

#include "pandabase.h"
#include "eventParameter.h"
#include "pandaNode.h"

////////////////////////////////////////////////////////////////////
//       Class : EventStorePandaNode
// Description : A class object for storing specifically objects of
//               type PandaNode.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA EventStorePandaNode : public EventStoreValueBase {
PUBLISHED:
  INLINE EventStorePandaNode(const PandaNode *value);
  virtual ~EventStorePandaNode();

  INLINE void set_value(const PandaNode *value);
  INLINE PandaNode *get_value() const;

  virtual void output(ostream &out) const;

public:
  PT(PandaNode) _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EventStoreValueBase::init_type();
    register_type(_type_handle, "EventStorePandaNode",
                  EventStoreValueBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "eventStorePandaNode.I"

#endif
