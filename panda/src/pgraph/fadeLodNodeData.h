// Filename: fadeLodNodeData.h
// Created by:  drose (29Sep04)
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

#ifndef FADELODNODEDATA_H
#define FADELODNODEDATA_H

#include "pandabase.h"

#include "auxSceneData.h"

////////////////////////////////////////////////////////////////////
//       Class : FadeLODNodeData
// Description : This is the data that is associated with a particular
//               instance of the FadeLODNode for the scene graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FadeLODNodeData : public AuxSceneData {
public:
  bool _fade_mode;
  float _fade_start;
  int _fade_out;
  int _fade_in;

  virtual void output(ostream &out) const;
  
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

