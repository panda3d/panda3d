/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxOcclusionQueryContext9.h
 * @author drose
 * @date 2007-06-04
 */

#ifndef DXOCCLUSIONQUERYCONTEXT9_H
#define DXOCCLUSIONQUERYCONTEXT9_H

#include "pandabase.h"
#include "occlusionQueryContext.h"
#include "deletedChain.h"

class GraphicsStateGuardian;

/**
 *
 */
class EXPCL_PANDADX DXOcclusionQueryContext9 : public OcclusionQueryContext {
public:
  INLINE DXOcclusionQueryContext9(IDirect3DQuery9 *query);
  virtual ~DXOcclusionQueryContext9();
  ALLOC_DELETED_CHAIN(DXOcclusionQueryContext9);

  virtual bool is_answer_ready() const;
  virtual void waiting_for_answer();
  virtual int get_num_fragments() const;

  IDirect3DQuery9 *_query;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OcclusionQueryContext::init_type();
    register_type(_type_handle, "DXOcclusionQueryContext9",
                  OcclusionQueryContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxOcclusionQueryContext9.I"

#endif
