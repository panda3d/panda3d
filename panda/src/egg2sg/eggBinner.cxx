// Filename: eggBinner.cxx
// Created by:  drose (17Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "eggBinner.h"

#include <eggSwitchCondition.h>
#include <eggGroup.h>

#include <assert.h>

////////////////////////////////////////////////////////////////////
//     Function: EggBinner::get_bin_number
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int EggBinner::
get_bin_number(const EggNode *node) {
  if (node->is_of_type(EggGroup::get_class_type())) {
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
  assert((BinNumber)bin_number == BN_lod);

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

////////////////////////////////////////////////////////////////////
//     Function: EggBinner::collapse_group
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool EggBinner::
collapse_group(const EggGroup *group, int) {
  if (group->get_dart_type() != EggGroup::DT_none) {
    // A group with the <Dart> flag set means to create a character.
    // We can't turn the top character node into an LOD.
    return false;
  }

  return true;
}
