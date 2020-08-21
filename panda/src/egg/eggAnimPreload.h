/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggAnimPreload.h
 * @author drose
 * @date 2008-08-06
 */

#ifndef EGGANIMPRELOAD_H
#define EGGANIMPRELOAD_H

#include "pandabase.h"

#include "eggNode.h"

/**
 * This corresponds to an <AnimPreload> entry.
 */
class EXPCL_PANDA_EGG EggAnimPreload : public EggNode {
PUBLISHED:
  INLINE explicit EggAnimPreload(const std::string &name = "");
  INLINE EggAnimPreload(const EggAnimPreload &copy);
  INLINE EggAnimPreload &operator = (const EggAnimPreload &copy);

  INLINE void set_fps(double fps);
  INLINE void clear_fps();
  INLINE bool has_fps() const;
  INLINE double get_fps() const;

  INLINE void set_num_frames(int num_frames);
  INLINE void clear_num_frames();
  INLINE bool has_num_frames() const;
  INLINE int get_num_frames() const;

  virtual void write(std::ostream &out, int indent_level) const;

private:
  double _fps;
  bool _has_fps;
  int _num_frames;
  bool _has_num_frames;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    register_type(_type_handle, "EggAnimPreload",
                  EggNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggAnimPreload.I"

#endif
