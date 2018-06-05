/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLwoLayer.cxx
 * @author drose
 * @date 2001-04-25
 */

#include "cLwoLayer.h"
#include "lwoToEggConverter.h"

#include "eggData.h"


/**
 * Creates the egg structures associated with this Lightwave object.
 */
void CLwoLayer::
make_egg() {
  _egg_group = new EggGroup(_layer->_name);

  if (_layer->_pivot != LPoint3::zero()) {
    // If we have a nonzero pivot point, that's a translation transform.
    LPoint3d translate = LCAST(double, _layer->_pivot);
    _egg_group->set_transform3d(LMatrix4d::translate_mat(translate));
    _egg_group->set_group_type(EggGroup::GT_instance);
  }
}

/**
 * Connects all the egg structures together.
 */
void CLwoLayer::
connect_egg() {
  if (_layer->_parent != -1) {
    const CLwoLayer *parent = _converter->get_layer(_layer->_parent);
    if (parent != nullptr) {
      parent->_egg_group->add_child(_egg_group.p());
      return;
    }

    nout << "No layer found with number " << _layer->_parent
         << "; cannot parent layer " << _layer->_number << " properly.\n";
  }

  _converter->get_egg_data()->add_child(_egg_group.p());
}
