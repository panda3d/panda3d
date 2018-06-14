/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggBinner.cxx
 * @author drose
 * @date 2000-02-17
 */

#include "eggBinner.h"
#include "eggRenderState.h"
#include "eggPrimitive.h"
#include "eggNurbsSurface.h"
#include "eggNurbsCurve.h"
#include "eggPatch.h"
#include "eggSwitchCondition.h"
#include "eggGroup.h"
#include "dcast.h"

/**
 *
 */
EggBinner::
EggBinner(EggLoader &loader) :
  _loader(loader)
{
}

/**
 * May be overridden in derived classes to perform some setup work as each
 * node is encountered.  This will be called once for each node in the egg
 * hierarchy.
 */
void EggBinner::
prepare_node(EggNode *node) {
  if (node->is_of_type(EggPrimitive::get_class_type())) {
    EggPrimitive *egg_prim = DCAST(EggPrimitive, node);
    PT(EggRenderState) render_state = new EggRenderState(_loader);
    render_state->fill_state(egg_prim);
    egg_prim->set_user_data(render_state);
  }
}

/**
 *
 */
int EggBinner::
get_bin_number(const EggNode *node) {
  if (node->is_of_type(EggNurbsSurface::get_class_type())) {
    return (int)BN_nurbs_surface;

  } else if (node->is_of_type(EggNurbsCurve::get_class_type())) {
    return (int)BN_nurbs_curve;

  } else if (node->is_of_type(EggPatch::get_class_type())) {
    return (int)BN_patches;

  } else if (node->is_of_type(EggPrimitive::get_class_type())) {
    return (int)BN_polyset;

  } else if (node->is_of_type(EggGroup::get_class_type())) {
    const EggGroup *group = DCAST(EggGroup, node);
    if (group->has_lod()) {
      return (int)BN_lod;
    }
  }

  return (int)BN_none;
}

/**
 * May be overridden in derived classes to define a name for each new bin,
 * based on its bin number, and a sample child.
 */
std::string EggBinner::
get_bin_name(int bin_number, const EggNode *child) {
  if (bin_number == BN_polyset || bin_number == BN_patches) {
    return DCAST(EggPrimitive, child)->get_sort_name();
  }

  return std::string();
}

/**
 *
 */
bool EggBinner::
sorts_less(int bin_number, const EggNode *a, const EggNode *b) {
  switch (bin_number) {
  case BN_polyset:
  case BN_patches:
    {
      const EggPrimitive *pa, *pb;
      DCAST_INTO_R(pa, a, false);
      DCAST_INTO_R(pb, b, false);

      // Different render states are binned separately.
      const EggRenderState *rsa, *rsb;
      DCAST_INTO_R(rsa, pa->get_user_data(EggRenderState::get_class_type()), false);
      DCAST_INTO_R(rsb, pb->get_user_data(EggRenderState::get_class_type()), false);
      int compare = rsa->compare_to(*rsb);
      if (compare != 0) {
        return (compare < 0);
      }

      if (bin_number == BN_patches) {
        // For patches only, we group together patches of similar size.
        const EggPatch *patch_a, *patch_b;
        DCAST_INTO_R(patch_a, a, false);
        DCAST_INTO_R(patch_b, b, false);
        if (patch_a->size() != patch_b->size()) {
          return patch_a->size() < patch_b->size();
        }
      }

      // Also, if the primitive was given a name (that does not begin with a
      // digit), it gets binned with similar-named primitives.
      return pa->get_sort_name() < pb->get_sort_name();
    }

  case BN_lod:
    {
      const EggGroup *ga = DCAST(EggGroup, a);
      const EggGroup *gb = DCAST(EggGroup, b);

      const EggSwitchCondition &swa = ga->get_lod();
      const EggSwitchCondition &swb = gb->get_lod();

      // For now, this is the only kind of switch condition there is.
      const EggSwitchConditionDistance &swda =
        *DCAST(EggSwitchConditionDistance, &swa);
      const EggSwitchConditionDistance &swdb =
        *DCAST(EggSwitchConditionDistance, &swb);

      // Group LOD nodes in order by switching center.
      return (swda._center.compare_to(swdb._center) < 0);
    }

  case BN_nurbs_surface:
  case BN_nurbs_curve:
    // Nurbs curves and surfaces are always binned individually.
    return a < b;

  case BN_none:
    break;
  }

  // Shouldn't get here.
  return false;
}
