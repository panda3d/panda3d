// Filename: eggBinner.cxx
// Created by:  drose (17Feb00)
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

#include "eggBinner.h"
#include "eggRenderState.h"
#include "eggPrimitive.h"
#include "eggSwitchCondition.h"
#include "eggGroup.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: EggBinner::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggBinner::
EggBinner(EggLoader &loader) :
  _loader(loader)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggBinner::prepare_node
//       Access: Public, Virtual
//  Description: May be overridden in derived classes to perform some
//               setup work as each node is encountered.  This will be
//               called once for each node in the egg hierarchy.
////////////////////////////////////////////////////////////////////
void EggBinner::
prepare_node(EggNode *node) {
  if (node->is_of_type(EggPrimitive::get_class_type())) {
    EggPrimitive *egg_prim = DCAST(EggPrimitive, node);
    PT(EggRenderState) render_state = new EggRenderState(_loader);
    render_state->fill_state(egg_prim);
    egg_prim->set_user_data(render_state);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggBinner::get_bin_number
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int EggBinner::
get_bin_number(const EggNode *node) {
  if (node->is_of_type(EggPrimitive::get_class_type())) {
    return (int)BN_polyset;

  } else if (node->is_of_type(EggGroup::get_class_type())) {
    const EggGroup *group = DCAST(EggGroup, node);
    if (group->has_lod()) {
      return (int)BN_lod;
    }
  }

  return (int)BN_none;
}


////////////////////////////////////////////////////////////////////
//     Function: EggBinner::sorts_less
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool EggBinner::
sorts_less(int bin_number, const EggNode *a, const EggNode *b) {
  switch (bin_number) {
  case BN_polyset:
    {
      const EggPrimitive *pa, *pb;
      DCAST_INTO_R(pa, a, false);
      DCAST_INTO_R(pb, b, false);

      // Different render states are binned separately.
      const EggRenderState *rsa, *rsb;
      DCAST_INTO_R(rsa, a->get_user_data(EggRenderState::get_class_type()), false);
      DCAST_INTO_R(rsb, b->get_user_data(EggRenderState::get_class_type()), false);
      int compare = rsa->compare_to(*rsb);
      if (compare != 0) {
        return (compare < 0);
      }

      // If the render state indicates indexed, meaning we keep the
      // existing vertex pools, then different pools get sorted
      // separately.
      if (rsa->_indexed) {
        if (pa->get_pool() != pb->get_pool()) {
          return pa->get_pool() < pb->get_pool();
        }
      }

      return false;
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
      
  case BN_none:
    break;
  }

  // Shouldn't get here.
  return false;
}
