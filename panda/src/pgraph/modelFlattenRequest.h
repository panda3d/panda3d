// Filename: modelFlattenRequest.h
// Created by:  drose (30Mar07)
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

#ifndef MODELFLATTENREQUEST
#define MODELFLATTENREQUEST

#include "pandabase.h"

#include "asyncTask.h"
#include "pandaNode.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : ModelFlattenRequest
// Description : This class object manages a single asynchronous
//               request to flatten a model.  The model will be
//               duplicated and flattened in a sub-thread (if
//               threading is available), without affecting the
//               original model; and when the result is done it may be
//               retrieved from this object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH ModelFlattenRequest : public AsyncTask {
public:
  ALLOC_DELETED_CHAIN(ModelFlattenRequest);

PUBLISHED:
  INLINE ModelFlattenRequest(PandaNode *orig);
  
  INLINE PandaNode *get_orig() const;
  
  INLINE bool is_ready() const;
  INLINE PandaNode *get_model() const;
  
protected:
  virtual DoneStatus do_task();
  
private:
  PT(PandaNode) _orig;
  bool _is_ready;
  PT(PandaNode) _model;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTask::init_type();
    register_type(_type_handle, "ModelFlattenRequest",
                  AsyncTask::get_class_type());
    }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  
private:
  static TypeHandle _type_handle;
};

#include "modelFlattenRequest.I"

#endif
