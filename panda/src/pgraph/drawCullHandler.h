// Filename: drawCullHandler.h
// Created by:  drose (25Feb02)
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

#ifndef DRAWCULLHANDLER_H
#define DRAWCULLHANDLER_H

#include "pandabase.h"
#include "cullHandler.h"

class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
//       Class : DrawCullHandler
// Description : This special kind of CullHandler immediately draws
//               its contents as soon as it receives them.  This draws
//               geometry immediately as it is encountered in the
//               scene graph by cull, mixing the draw and cull
//               traversals into one traversal, and prohibiting state
//               sorting.  However, it has somewhat lower overhead
//               than separating out draw and cull, if state sorting
//               and multiprocessing are not required.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DrawCullHandler : public CullHandler {
public:
  INLINE DrawCullHandler(GraphicsStateGuardianBase *gsg);

  virtual void record_object(CullableObject *object, 
                             const CullTraverser *traverser);

private:
  GraphicsStateGuardianBase *_gsg;
};

#include "drawCullHandler.I"

#endif


  
