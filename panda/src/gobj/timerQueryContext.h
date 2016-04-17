/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file timerQueryContext.h
 * @author rdb
 * @date 2014-08-22
 */

#ifndef TIMERQUERYCONTEXT_H
#define TIMERQUERYCONTEXT_H

#include "pandabase.h"
#include "queryContext.h"
#include "clockObject.h"
#include "pStatCollector.h"

/**
 *
 */
class EXPCL_PANDA_GOBJ TimerQueryContext : public QueryContext {
public:
  INLINE TimerQueryContext(int pstats_index);

  ALLOC_DELETED_CHAIN(TimerQueryContext);

  virtual double get_timestamp() const=0;

  int _frame_index;
  int _pstats_index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    QueryContext::init_type();
    register_type(_type_handle, "TimerQueryContext",
                  QueryContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PreparedGraphicsObjects;
};

#include "timerQueryContext.I"

#endif
