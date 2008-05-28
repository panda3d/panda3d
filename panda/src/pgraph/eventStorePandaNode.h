// Filename: eventStorePandaNode.h
// Created by:  drose (13Sep06)
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
class EXPCL_PANDA_PGRAPH EventStorePandaNode : public EventStoreValueBase {
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
