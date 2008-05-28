// Filename: occlusionQueryContext.h
// Created by:  drose (27Mar06)
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

#ifndef OCCLUSIONQUERYCONTEXT_H
#define OCCLUSIONQUERYCONTEXT_H

#include "pandabase.h"
#include "queryContext.h"

////////////////////////////////////////////////////////////////////
//       Class : OcclusionQueryContext
// Description : Returned from a GSG in response to
//               begin_occlusion_query() .. end_occlusion_query(),
//               this records the number of fragments (pixels) that
//               passed the depth test between the bracketing calls.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ OcclusionQueryContext : public QueryContext {
public:
  INLINE OcclusionQueryContext();

  virtual int get_num_fragments() const=0;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    QueryContext::init_type();
    register_type(_type_handle, "OcclusionQueryContext",
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

#include "occlusionQueryContext.I"

#endif

