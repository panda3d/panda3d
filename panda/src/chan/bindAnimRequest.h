/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bindAnimRequest.h
 * @author drose
 * @date 2008-08-05
 */

#ifndef BINDANIMREQUEST
#define BINDANIMREQUEST

#include "pandabase.h"

#include "modelLoadRequest.h"
#include "partSubset.h"

class AnimControl;

/**
 * This class object manages an asynchronous load-and-bind animation request,
 * as issued through PartBundle::load_bind_anim().
 */
class EXPCL_PANDA_CHAN BindAnimRequest : public ModelLoadRequest {
public:
  ALLOC_DELETED_CHAIN(BindAnimRequest);

PUBLISHED:
  explicit BindAnimRequest(const std::string &name,
                           const Filename &filename,
                           const LoaderOptions &options,
                           Loader *loader,
                           AnimControl *control,
                           int hierarchy_match_flags,
                           const PartSubset &subset);

protected:
  virtual DoneStatus do_task();

private:
  PT(AnimControl) _control;
  int _hierarchy_match_flags;
  PartSubset _subset;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ModelLoadRequest::init_type();
    register_type(_type_handle, "BindAnimRequest",
                  ModelLoadRequest::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "bindAnimRequest.I"

#endif
