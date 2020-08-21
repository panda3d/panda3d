/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSAnimData.h
 * @author drose
 * @date 1999-02-19
 */

#ifndef EGGSANIMDATA_H
#define EGGSANIMDATA_H

#include "pandabase.h"

#include "eggAnimData.h"

/**
 * Corresponding to an <S$Anim> entry, this stores a single column of numbers,
 * for instance for a morph target, or as one column in an EggXfmSAnim.
 */
class EXPCL_PANDA_EGG EggSAnimData : public EggAnimData {
PUBLISHED:
  INLINE explicit EggSAnimData(const std::string &name = "");
  INLINE EggSAnimData(const EggSAnimData &copy);
  INLINE EggSAnimData &operator = (const EggSAnimData &copy);

  INLINE int get_num_rows() const;
  INLINE double get_value(int row) const;
  INLINE void set_value(int row, double value);

  void optimize();

  virtual void write(std::ostream &out, int indent_level) const;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggAnimData::init_type();
    register_type(_type_handle, "EggSAnimData",
                  EggAnimData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggSAnimData.I"

#endif
