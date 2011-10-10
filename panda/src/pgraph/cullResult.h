// Filename: cullResult.h
// Created by:  drose (27Feb02)
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

#ifndef CULLRESULT_H
#define CULLRESULT_H

#include "pandabase.h"
#include "cullBin.h"
#include "renderState.h"
#include "cullableObject.h"
#include "geomMunger.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pset.h"
#include "pmap.h"


class GraphicsStateGuardianBase;
class CullTraverser;
class TransformState;
class RenderState;
class SceneSetup;

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
class EXPCL_PANDA_PGRAPH CullResult : public ReferenceCount {
public:
  CullResult(GraphicsStateGuardianBase *gsg,
             const PStatCollector &draw_region_pcollector);
  INLINE ~CullResult();

PUBLISHED:
  PT(CullResult) make_next() const;

  INLINE CullBin *get_bin(int bin_index);

  void add_object(CullableObject *object, const CullTraverser *traverser);
  void finish_cull(SceneSetup *scene_setup, Thread *current_thread);
  void draw(Thread *current_thread);

  PT(PandaNode) make_result_graph();

public:
  static void bin_removed(int bin_index);

private:
  CullBin *make_new_bin(int bin_index);
  void check_flash_bin(CPT(RenderState) &state, CullBin *bin);
  void check_flash_transparency(CPT(RenderState) &state, const LColor &color);

  static CPT(RenderState) get_alpha_state();
  static CPT(RenderState) get_binary_state();
  static CPT(RenderState) get_dual_transparent_state();
  static CPT(RenderState) get_dual_transparent_state_decals();
  static CPT(RenderState) get_dual_opaque_state();

  GraphicsStateGuardianBase *_gsg;
  PStatCollector _draw_region_pcollector;
  
  typedef pvector< PT(CullBin) > Bins;
  Bins _bins;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "CullResult",
                  ReferenceCount::get_class_type());
  }
  
private:
  static TypeHandle _type_handle;
};

#include "cullResult.I"

#endif


  
