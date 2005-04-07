// Filename: cullHandler.h
// Created by:  drose (23Feb02)
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

#ifndef CULLHANDLER_H
#define CULLHANDLER_H

#include "pandabase.h"
#include "cullableObject.h"
#include "graphicsStateGuardianBase.h"

class CullTraverser;

////////////////////////////////////////////////////////////////////
//       Class : CullHandler
// Description : This defines the abstract interface for an object
//               that receives Geoms identified by the CullTraverser.
//               By itself, it's not a particularly useful class; to
//               use it, derive from it and redefine record_object().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullHandler {
public:
  virtual ~CullHandler();

  virtual void record_object(CullableObject *object, 
                             const CullTraverser *traverser);

  INLINE static void draw(CullableObject *object,
                          GraphicsStateGuardianBase *gsg);
  static void draw_with_decals(CullableObject *object,
                               GraphicsStateGuardianBase *gsg);
};

#include "cullHandler.I"

#endif


  
