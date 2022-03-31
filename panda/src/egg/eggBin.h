/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggBin.h
 * @author drose
 * @date 1999-01-21
 */

#ifndef EGGBIN_H
#define EGGBIN_H

#include "pandabase.h"

#include "eggGroup.h"

/**
 * A type of group node that holds related subnodes.  This is a special kind
 * of node that will never be read in from an egg file, but can only exist in
 * the egg scene graph if it is created via the use of an EggBinMaker.
 */
class EXPCL_PANDA_EGG EggBin : public EggGroup {
PUBLISHED:
  explicit EggBin(const std::string &name = "");
  EggBin(const EggGroup &copy);
  EggBin(const EggBin &copy);

  void set_bin_number(int bin_number);
  int get_bin_number() const;

private:
  int _bin_number;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggGroup::init_type();
    register_type(_type_handle, "EggBin",
                  EggGroup::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
