/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialPool.cxx
 * @author drose
 * @date 2001-04-30
 */

#include "materialPool.h"
#include "config_gobj.h"
#include "lightMutexHolder.h"

MaterialPool *MaterialPool::_global_ptr = nullptr;


/**
 * Lists the contents of the material pool to the indicated output stream.
 */
void MaterialPool::
write(std::ostream &out) {
  get_global_ptr()->ns_list_contents(out);
}

/**
 * The nonstatic implementation of get_material().
 */
Material *MaterialPool::
ns_get_material(Material *temp) {
  LightMutexHolder holder(_lock);

  CPT(Material) cpttemp = temp;
  Materials::iterator mi = _materials.find(cpttemp);
  if (mi == _materials.end()) {
    mi = _materials.insert(Materials::value_type(new Material(*temp), temp)).first;
  } else {
    if (*(*mi).first != *(*mi).second) {
      // The pointer no longer matches its original value.  Save a new one.
      (*mi).second = temp;
    }
  }
  return (*mi).second;
}

/**
 * The nonstatic implementation of release_material().
 */
void MaterialPool::
ns_release_material(Material *temp) {
  LightMutexHolder holder(_lock);

  CPT(Material) cpttemp = temp;
  _materials.erase(cpttemp);
}

/**
 * The nonstatic implementation of release_all_materials().
 */
void MaterialPool::
ns_release_all_materials() {
  LightMutexHolder holder(_lock);

  _materials.clear();
}

/**
 * The nonstatic implementation of garbage_collect().
 */
int MaterialPool::
ns_garbage_collect() {
  LightMutexHolder holder(_lock);

  int num_released = 0;
  Materials new_set;

  Materials::iterator mi;
  for (mi = _materials.begin(); mi != _materials.end(); ++mi) {
    const Material *mat1 = (*mi).first;
    Material *mat2 = (*mi).second;
    if ((*mat1) != (*mat2) || mat2->get_ref_count() == 1) {
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Releasing " << *mat1 << "\n";
      }
      ++num_released;
    } else {
      new_set.insert(new_set.end(), *mi);
    }
  }

  _materials.swap(new_set);
  return num_released;
}

/**
 * The nonstatic implementation of list_contents().
 */
void MaterialPool::
ns_list_contents(std::ostream &out) const {
  LightMutexHolder holder(_lock);

  out << _materials.size() << " materials:\n";
  Materials::const_iterator mi;
  for (mi = _materials.begin(); mi != _materials.end(); ++mi) {
    const Material *mat1 = (*mi).first;
    Material *mat2 = (*mi).second;
    out << "  " << *mat1
        << " (count = " << mat2->get_ref_count() << ")\n";
  }
}

/**
 * Initializes and/or returns the global pointer to the one MaterialPool
 * object in the system.
 */
MaterialPool *MaterialPool::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new MaterialPool;
  }
  return _global_ptr;
}
