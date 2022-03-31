/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fadeLodNodeData.h
 * @author drose
 * @date 2004-09-29
 */

#ifndef FADELODNODEDATA_H
#define FADELODNODEDATA_H

#include "pandabase.h"

#include "auxSceneData.h"

/**
 * This is the data that is associated with a particular instance of the
 * FadeLODNode for the scene graph.
 */
class EXPCL_PANDA_PGRAPHNODES FadeLODNodeData : public AuxSceneData {
public:
  enum FadeMode {
    FM_solid,
    FM_more_detail,
    FM_less_detail,
  };
  FadeMode _fade_mode;
  PN_stdfloat _fade_start;
  int _fade_out;
  int _fade_in;

  virtual void output(std::ostream &out) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AuxSceneData::init_type();
    register_type(_type_handle, "FadeLODNodeData",
                  AuxSceneData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
