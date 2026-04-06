/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggXfmSAnim.h
 * @author drose
 * @date 1999-02-19
 */

#ifndef EGGXFMSANIM_H
#define EGGXFMSANIM_H

#include "pandabase.h"
#include "eggGroupNode.h"

class EggXfmAnimData;

/**
 * This corresponds to an <Xfm$Anim_S$> entry, which is a collection of up to
 * nine <S$Anim> entries that specify the nine components of a transformation.
 * It's implemented as a group that can contain any number of EggSAnimData
 * children.
 */
class EXPCL_PANDA_EGG EggXfmSAnim : public EggGroupNode {
PUBLISHED:
  INLINE explicit EggXfmSAnim(const std::string &name = "",
                              CoordinateSystem cs = CS_default);
  EggXfmSAnim(const EggXfmAnimData &convert_from);

  INLINE EggXfmSAnim(const EggXfmSAnim &copy);
  INLINE EggXfmSAnim &operator = (const EggXfmSAnim &copy);

  INLINE void set_fps(double fps);
  INLINE void clear_fps();
  INLINE bool has_fps() const;
  INLINE double get_fps() const;

  INLINE void set_order(const std::string &order);
  INLINE void clear_order();
  INLINE bool has_order() const;
  INLINE const std::string &get_order() const;
  INLINE static const std::string &get_standard_order();

  INLINE CoordinateSystem get_coordinate_system() const;

  void optimize();
  void optimize_to_standard_order();
  void normalize();

  int get_num_rows() const;
  void get_value(int row, LMatrix4d &mat) const;
  bool set_value(int row, const LMatrix4d &mat);

  INLINE void clear_data();
  bool add_data(const LMatrix4d &mat);
  void add_component_data(const std::string &component_name, double value);
  void add_component_data(int component, double value);

  virtual bool is_anim_matrix() const;
  virtual void write(std::ostream &out, int indent_level) const;

  static void compose_with_order(LMatrix4d &mat,
                                 const LVecBase3d &scale,
                                 const LVecBase3d &shear,
                                 const LVecBase3d &hpr,
                                 const LVecBase3d &trans,
                                 const std::string &order,
                                 CoordinateSystem cs);

protected:
  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
                           CoordinateSystem to_cs);
  virtual void r_mark_coordsys(CoordinateSystem cs);

private:
  void normalize_by_rebuilding();
  void normalize_by_expanding();


private:
  double _fps;
  bool _has_fps;
  std::string _order;
  CoordinateSystem _coordsys;

  static const std::string _standard_order;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggGroupNode::init_type();
    register_type(_type_handle, "EggXfmSAnim",
                  EggGroupNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggXfmSAnim.I"

#endif
