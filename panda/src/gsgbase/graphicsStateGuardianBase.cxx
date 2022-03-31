/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsStateGuardianBase.cxx
 * @author drose
 * @date 1999-10-06
 */

#include "graphicsStateGuardianBase.h"
#include "lightMutexHolder.h"
#include <algorithm>

AtomicAdjust::Pointer GraphicsStateGuardianBase::_gsg_list;
UpdateSeq GraphicsStateGuardianBase::_generated_shader_seq;
TypeHandle GraphicsStateGuardianBase::_type_handle;

/**
 * Returns a pointer to the "default" GSG.  This is typically the first GSG
 * created in an application; in a single-window application, it will be the
 * only GSG. This GSG is used to determine default optimization choices for
 * loaded geometry.
 *
 * The return value may be NULL if a GSG has not been created.
 */
GraphicsStateGuardianBase *GraphicsStateGuardianBase::
get_default_gsg() {
  GSGList *gsg_list = (GSGList *)AtomicAdjust::get_ptr(_gsg_list);
  if (gsg_list == nullptr) {
    // Nobody created a GSG list, so we won't have any GSGs either.
    return nullptr;
  }
  LightMutexHolder holder(gsg_list->_lock);
  return gsg_list->_default_gsg;
}

/**
 * Specifies a particular GSG to use as the "default" GSG.  See
 * get_default_gsg().
 */
void GraphicsStateGuardianBase::
set_default_gsg(GraphicsStateGuardianBase *default_gsg) {
  GSGList *gsg_list = (GSGList *)AtomicAdjust::get_ptr(_gsg_list);
  if (gsg_list == nullptr) {
    // Nobody ever created a GSG list.  How could we have a GSG?
    nassertv(false);
    return;
  }

  LightMutexHolder holder(gsg_list->_lock);
  if (find(gsg_list->_gsgs.begin(), gsg_list->_gsgs.end(), default_gsg) == gsg_list->_gsgs.end()) {
    // The specified GSG doesn't exist or it has already destructed.
    nassert_raise("GSG not found or already destructed");
    return;
  }

  gsg_list->_default_gsg = default_gsg;
}

/**
 * Returns the total number of GSG's in the universe.
 */
size_t GraphicsStateGuardianBase::
get_num_gsgs() {
  GSGList *gsg_list = (GSGList *)AtomicAdjust::get_ptr(_gsg_list);
  if (gsg_list == nullptr) {
    // Nobody created a GSG list, so we won't have any GSGs either.
    return 0;
  }
  LightMutexHolder holder(gsg_list->_lock);
  return gsg_list->_gsgs.size();
}

/**
 * Returns the nth GSG in the universe.  GSG's automatically add themselves
 * and remove themselves from this list as they are created and destroyed.
 */
GraphicsStateGuardianBase *GraphicsStateGuardianBase::
get_gsg(size_t n) {
  GSGList *gsg_list = (GSGList *)AtomicAdjust::get_ptr(_gsg_list);
  nassertr(gsg_list != nullptr, nullptr);

  LightMutexHolder holder(gsg_list->_lock);
  nassertr(n < gsg_list->_gsgs.size(), nullptr);
  return gsg_list->_gsgs[n];
}

/**
 * Called by a GSG after it has been initialized, to add a new GSG to the
 * available list.
 */
void GraphicsStateGuardianBase::
add_gsg(GraphicsStateGuardianBase *gsg) {
  GSGList *gsg_list = (GSGList *)AtomicAdjust::get_ptr(_gsg_list);
  if (gsg_list == nullptr) {
    gsg_list = new GSGList;
    gsg_list->_default_gsg = nullptr;

    GSGList *orig_gsg_list = (GSGList *)
      AtomicAdjust::compare_and_exchange_ptr(_gsg_list, nullptr, gsg_list);

    if (orig_gsg_list != nullptr) {
      // Another thread beat us to it.  No problem, we'll use that.
      delete gsg_list;
      gsg_list = orig_gsg_list;
    }
  }

  LightMutexHolder holder(gsg_list->_lock);

  if (find(gsg_list->_gsgs.begin(), gsg_list->_gsgs.end(), gsg) != gsg_list->_gsgs.end()) {
    // Already on the list.
    return;
  }

  gsg_list->_gsgs.push_back(gsg);

  if (gsg_list->_default_gsg == nullptr) {
    gsg_list->_default_gsg = gsg;
  }
}

/**
 * Called by a GSG destructor to remove a GSG from the available list.
 */
void GraphicsStateGuardianBase::
remove_gsg(GraphicsStateGuardianBase *gsg) {
  GSGList *gsg_list = (GSGList *)AtomicAdjust::get_ptr(_gsg_list);
  if (gsg_list == nullptr) {
    // No GSGs were added yet, or the program is destructing anyway.
    return;
  }

  LightMutexHolder holder(gsg_list->_lock);

  GSGList::GSGs::iterator gi;
  gi = find(gsg_list->_gsgs.begin(), gsg_list->_gsgs.end(), gsg);
  if (gi == gsg_list->_gsgs.end()) {
    // Already removed, or never added.
    return;
  }

  gsg_list->_gsgs.erase(gi);

  if (gsg_list->_default_gsg == gsg) {
    if (!gsg_list->_gsgs.empty()) {
      gsg_list->_default_gsg = *gsg_list->_gsgs.begin();
    } else {
      gsg_list->_default_gsg = nullptr;
    }
  }
}
