/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPolysetMaker.h
 * @author drose
 * @date 2001-06-19
 */

#ifndef EGGPOLYSETMAKER_H
#define EGGPOLYSETMAKER_H

#include "pandabase.h"

#include "eggBinMaker.h"

#include "dcast.h"

/**
 * A specialization on EggBinMaker for making polysets that share the same
 * basic rendering characteristic.  This really just defines the example
 * functions described in the leading comment to EggBinMaker.
 *
 * It makes some common assumptions about how polysets should be grouped; if
 * these are not sufficient, you can always rederive your own further
 * specialization of this class.
 */
class EXPCL_PANDA_EGG EggPolysetMaker : public EggBinMaker {
PUBLISHED:
  // The BinNumber serves to identify why a particular EggBin was created.
  enum BinNumber {
    BN_none = 0,
    BN_polyset,
  };

  enum Properties {
    P_has_texture        = 0x001,
    P_texture            = 0x002,
    P_has_material       = 0x004,
    P_material           = 0x008,
    P_has_poly_color     = 0x010,
    P_poly_color         = 0x020,
    P_has_poly_normal    = 0x040,
    P_has_vertex_normal  = 0x080,
    P_has_vertex_color   = 0x100,
    P_bface              = 0x200,
  };

  EggPolysetMaker();
  void set_properties(int properties);

public:
  virtual int
  get_bin_number(const EggNode *node);

  virtual bool
  sorts_less(int bin_number, const EggNode *a, const EggNode *b);

private:
  int _properties;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggBinMaker::init_type();
    register_type(_type_handle, "EggPolysetMaker",
                  EggBinMaker::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


#endif
