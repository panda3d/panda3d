// Filename: auxSceneData.h
// Created by:  drose (27Sep04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef AUXSCENEDATA_H
#define AUXSCENEDATA_H

#include "pandabase.h"

#include "typedReferenceCount.h"

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
class EXPCL_PANDA AuxSceneData : public TypedReferenceCount {
public:
  INLINE AuxSceneData();
  
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

#include "auxSceneData.I"

#endif
