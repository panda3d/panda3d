/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggXfmAnimData.h
 * @author drose
 * @date 1999-02-19
 */

#ifndef EGGXFMANIMDATA_H
#define EGGXFMANIMDATA_H

#include "pandabase.h"

#include "eggAnimData.h"
#include "eggXfmSAnim.h"


/**
 * Corresponding to an <Xfm$Anim> entry, this stores a two-dimensional table
 * with up to nine columns, one for each component of a transformation.  This
 * is an older syntax of egg anim table, not often used currently--it's
 * replaced by EggXfmSAnim.
 */
class EXPCL_PANDA_EGG EggXfmAnimData : public EggAnimData {
PUBLISHED:
  INLINE explicit EggXfmAnimData(const std::string &name = "",
                                 CoordinateSystem cs = CS_default);
  EggXfmAnimData(const EggXfmSAnim &convert_from);

  INLINE EggXfmAnimData(const EggXfmAnimData &copy);
  INLINE EggXfmAnimData &operator = (const EggXfmAnimData &copy);

  INLINE void set_order(const std::string &order);
  INLINE void clear_order();
  INLINE bool has_order() const;
  INLINE const std::string &get_order() const;
  INLINE static const std::string &get_standard_order();

  INLINE void set_contents(const std::string &contents);
  INLINE void clear_contents();
  INLINE bool has_contents() const;
  INLINE const std::string &get_contents() const;

  INLINE CoordinateSystem get_coordinate_system() const;

  INLINE int get_num_rows() const;
  INLINE int get_num_cols() const;
  INLINE double get_value(int row, int col) const;

  void get_value(int row, LMatrix4d &mat) const;

  virtual bool is_anim_matrix() const;
  virtual void write(std::ostream &out, int indent_level) const;

protected:
  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
                           CoordinateSystem to_cs);
  virtual void r_mark_coordsys(CoordinateSystem cs);

private:
  std::string _order;
  std::string _contents;
  CoordinateSystem _coordsys;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggAnimData::init_type();
    register_type(_type_handle, "EggXfmAnimData",
                  EggAnimData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggXfmAnimData.I"

#endif
