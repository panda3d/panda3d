/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggAnimData.h
 * @author drose
 * @date 1999-02-19
 */

#ifndef EGGANIMDATA_H
#define EGGANIMDATA_H

#include "pandabase.h"

#include "eggNode.h"

#include "pointerToArray.h"
#include "pta_double.h"

#include <math.h>

/**
 * A base class for EggSAnimData and EggXfmAnimData, which contain rows and
 * columns of numbers.
 */
class EXPCL_PANDA_EGG EggAnimData : public EggNode {
PUBLISHED:
  INLINE explicit EggAnimData(const std::string &name = "");
  INLINE EggAnimData(const EggAnimData &copy);
  INLINE EggAnimData &operator = (const EggAnimData &copy);

  INLINE void set_fps(double type);
  INLINE void clear_fps();
  INLINE bool has_fps() const;
  INLINE double get_fps() const;

  INLINE void clear_data();
  INLINE void add_data(double value);

  INLINE int get_size() const;

public:
  INLINE PTA_double get_data() const;
  INLINE void set_data(const PTA_double &data);

PUBLISHED:
  void quantize(double quantum);

protected:
  PTA_double _data;

private:
  double _fps;
  bool _has_fps;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    register_type(_type_handle, "EggAnimData",
                  EggNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggAnimData.I"

#endif
