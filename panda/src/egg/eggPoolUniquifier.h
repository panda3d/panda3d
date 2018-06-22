/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPoolUniquifier.h
 * @author drose
 * @date 2000-11-09
 */

#ifndef EGGPOOLUNIQUIFIER_H
#define EGGPOOLUNIQUIFIER_H

#include "pandabase.h"

#include "eggNameUniquifier.h"

/**
 * This is a specialization of EggNameUniquifier to generate unique names for
 * textures, materials, and vertex pools prior to writing out an egg file.
 * It's automatically called by EggData prior to writing out an egg file.
 */
class EXPCL_PANDA_EGG EggPoolUniquifier : public EggNameUniquifier {
PUBLISHED:
  EggPoolUniquifier();

  virtual std::string get_category(EggNode *node);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNameUniquifier::init_type();
    register_type(_type_handle, "EggPoolUniquifier",
                  EggNameUniquifier::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#endif
