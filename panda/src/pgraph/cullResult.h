// Filename: cullResult.h
// Created by:  drose (27Feb02)
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

#ifndef CULLRESULT_H
#define CULLRESULT_H

#include "pandabase.h"
#include "cullBin.h"
#include "renderState.h"
#include "cullableObject.h"
#include "qpgeomMunger.h"

#include "referenceCount.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pset.h"
#include "pmap.h"


class GraphicsStateGuardianBase;
class TransformState;
class RenderState;

////////////////////////////////////////////////////////////////////
//       Class : CullResult
// Description : This stores the result of a BinCullHandler traversal:
//               an ordered collection of CullBins, each of which
//               holds a number of Geoms and RenderStates to be
//               rendered in some defined order.
//
//               This is also used to keep the results of last frame's
//               cull traversal around to make next frame's traversal
//               of the same scene a little easier.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullResult : public ReferenceCount {
public:
  INLINE CullResult(GraphicsStateGuardianBase *gsg);
  INLINE ~CullResult();

  PT(CullResult) make_next() const;

  INLINE CullBin *get_bin(int bin_index);

  void add_object(CullableObject *object);
  void finish_cull();
  void draw();

public:
  static void bin_removed(int bin_index);

private:
  CullBin *make_new_bin(int bin_index);
  INLINE CPT(qpGeomMunger) get_geom_munger(const RenderState *state);

  static CPT(RenderState) get_binary_state();
  static CPT(RenderState) get_dual_transparent_state();
  static CPT(RenderState) get_dual_transparent_state_decals();
  static CPT(RenderState) get_dual_opaque_state();

  GraphicsStateGuardianBase *_gsg;

  typedef pmap<CPT(RenderState), CPT(qpGeomMunger) > Mungers;
  Mungers _mungers;

  typedef pvector< PT(CullBin) > Bins;
  Bins _bins;
};

#include "cullResult.I"

#endif


  
