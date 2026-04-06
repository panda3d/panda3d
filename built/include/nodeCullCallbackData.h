/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodeCullCallbackData.h
 * @author drose
 * @date 2009-03-13
 */

#ifndef NODECULLCALLBACKDATA_H
#define NODECULLCALLBACKDATA_H

#include "pandabase.h"
#include "callbackData.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"

/**
 * This kind of CallbackData is passed to the CallbackObject added to
 * CallbackNode:set_cull_callback().
 */
class EXPCL_PANDA_PGRAPHNODES NodeCullCallbackData : public CallbackData {
public:
  INLINE NodeCullCallbackData(CullTraverser *trav, CullTraverserData &data);

PUBLISHED:
  virtual void output(std::ostream &out) const;

  INLINE CullTraverser *get_trav() const;
  INLINE CullTraverserData &get_data() const;

  virtual void upcall();

private:
  CullTraverser *_trav;
  CullTraverserData &_data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    register_type(_type_handle, "NodeCullCallbackData",
                  CallbackData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "nodeCullCallbackData.I"

#endif
