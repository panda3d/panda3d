// Filename: binCullHandler.h
// Created by:  drose (28Feb02)
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

#ifndef BINCULLHANDLER_H
#define BINCULLHANDLER_H

#include "pandabase.h"
#include "cullHandler.h"
#include "cullResult.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : BinCullHandler
// Description : This CullHandler sends all of the geoms it receives
//               into a CullResult object, for binning (and later
//               drawing).  This is the kind of CullHandler to use for
//               most normal rendering needs.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BinCullHandler : public CullHandler {
public:
  INLINE BinCullHandler(CullResult *cull_result);

  virtual void record_object(CullableObject *object);

private:
  PT(CullResult) _cull_result;
};

#include "binCullHandler.I"

#endif


  
