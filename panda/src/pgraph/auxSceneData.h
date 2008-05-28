// Filename: auxSceneData.h
// Created by:  drose (27Sep04)
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

#ifndef AUXSCENEDATA_H
#define AUXSCENEDATA_H

#include "pandabase.h"

#include "typedReferenceCount.h"
#include "clockObject.h"

////////////////////////////////////////////////////////////////////
//       Class : AuxSceneData
// Description : This is a base class for a generic data structure
//               that can be attached per-instance to the camera, to
//               store per-instance data that must be preserved over
//               multiple frames.
//
//               In particular, this is used to implement the
//               FadeLODNode, which must remember during traversal at
//               what point it is in the fade, separately for each
//               instance and for each camera.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH AuxSceneData : public TypedReferenceCount {
protected:
  INLINE AuxSceneData(double duration = 0.0);

PUBLISHED:
  INLINE void set_duration(double duration);
  INLINE double get_duration() const;

  INLINE void set_last_render_time(double render_time);
  INLINE double get_last_render_time() const;

  INLINE double get_expiration_time() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  double _duration;
  double _last_render_time;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AuxSceneData",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const AuxSceneData &data);

#include "auxSceneData.I"

#endif
