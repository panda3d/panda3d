/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file displayRegionDrawCallbackData.h
 * @author drose
 * @date 2009-03-13
 */

#ifndef DISPLAYREGIONDRAWCALLBACKDATA_H
#define DISPLAYREGIONDRAWCALLBACKDATA_H

#include "pandabase.h"
#include "callbackData.h"

class CullResult;
class SceneSetup;

/**
 * This specialization on CallbackData is passed when the callback is
 * initiated from the draw traversal, for a DisplayRegion.
 */
class EXPCL_PANDA_DISPLAY DisplayRegionDrawCallbackData : public CallbackData {
public:
  DisplayRegionDrawCallbackData(CullResult *cull_result, SceneSetup *scene_setup);

PUBLISHED:
  virtual void output(std::ostream &out) const;

  INLINE CullResult *get_cull_result() const;
  INLINE SceneSetup *get_scene_setup() const;

  virtual void upcall();

private:
  CullResult *_cull_result;
  SceneSetup *_scene_setup;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    register_type(_type_handle, "DisplayRegionDrawCallbackData",
                  CallbackData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "displayRegionDrawCallbackData.I"

#endif
