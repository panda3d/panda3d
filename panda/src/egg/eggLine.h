/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggLine.h
 * @author drose
 * @date 2003-10-14
 */

#ifndef EGGLINE_H
#define EGGLINE_H

#include "pandabase.h"

#include "eggCompositePrimitive.h"

/**
 * A line segment, or a series of connected line segments, defined by a <Line>
 * entry.
 */
class EXPCL_PANDA_EGG EggLine : public EggCompositePrimitive {
PUBLISHED:
  INLINE explicit EggLine(const std::string &name = "");
  INLINE EggLine(const EggLine &copy);
  INLINE EggLine &operator = (const EggLine &copy);
  virtual ~EggLine();

  virtual EggLine *make_copy() const override;

  virtual void write(std::ostream &out, int indent_level) const override;

  INLINE bool has_thick() const;
  INLINE double get_thick() const;
  INLINE void set_thick(double thick);
  INLINE void clear_thick();

protected:
  virtual int get_num_lead_vertices() const override;

private:
  double _thick;
  bool _has_thick;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggCompositePrimitive::init_type();
    register_type(_type_handle, "EggLine",
                  EggCompositePrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

};

#include "eggLine.I"

#endif
