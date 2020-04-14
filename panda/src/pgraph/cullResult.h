/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullResult.h
 * @author drose
 * @date 2002-02-27
 */

#ifndef CULLRESULT_H
#define CULLRESULT_H

#include "pandabase.h"
#include "cullBin.h"
#include "cullBinManager.h"
#include "renderState.h"
#include "cullableObject.h"
#include "geomMunger.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pset.h"
#include "pmap.h"
#include "rescaleNormalAttrib.h"

class CullTraverser;
class GraphicsStateGuardianBase;
class RenderState;
class SceneSetup;
class TransformState;

/**
 * This stores the result of a BinCullHandler traversal: an ordered collection
 * of CullBins, each of which holds a number of Geoms and RenderStates to be
 * rendered in some defined order.
 *
 * This is also used to keep the results of last frame's cull traversal around
 * to make next frame's traversal of the same scene a little easier.
 */
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

  INLINE void check_flash_bin(CPT(RenderState) &state, CullBinManager *bin_manager, int bin_index);
  INLINE void check_flash_transparency(CPT(RenderState) &state, const LColor &color);

#ifndef NDEBUG
  void apply_flash_color(CPT(RenderState) &state, const LColor &flash_color);
#endif

  static const RenderState *get_rescale_normal_state(RescaleNormalAttrib::Mode mode);
  static const RenderState *get_alpha_state();
  static const RenderState *get_binary_state();
  static const RenderState *get_dual_transparent_state();
  static const RenderState *get_dual_opaque_state();
  static const RenderState *get_wireframe_filled_state();
  static CPT(RenderState) get_wireframe_overlay_state(const RenderModeAttrib *rmode);

  GraphicsStateGuardianBase *_gsg;
  PStatCollector _draw_region_pcollector;

  typedef pvector< PT(CullBin) > Bins;
  Bins _bins;

  bool _show_transparency = false;

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
