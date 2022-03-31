/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSwitchCondition.h
 * @author drose
 * @date 1999-02-08
 */

#ifndef EGGSWITCHCONDITION
#define EGGSWITCHCONDITION

#include "pandabase.h"

#include "eggObject.h"
#include "luse.h"

/**
 * This corresponds to a <SwitchCondition> entry within a group.  It indicates
 * the condition at which a level-of-detail is switched in or out.  This is
 * actually an abstract base class for potentially any number of specific
 * different kinds of switching conditions; presently, only a <Distance> type
 * is actually supported.
 */
class EXPCL_PANDA_EGG EggSwitchCondition : public EggObject {
PUBLISHED:
  virtual EggSwitchCondition *make_copy() const=0;
  virtual void write(std::ostream &out, int indent_level) const=0;

  virtual void transform(const LMatrix4d &mat)=0;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggObject::init_type();
    register_type(_type_handle, "EggSwitchCondition",
                  EggObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


/**
 * A SwitchCondition that switches the levels-of-detail based on distance from
 * the camera's eyepoint.
 */
class EXPCL_PANDA_EGG EggSwitchConditionDistance : public EggSwitchCondition {
PUBLISHED:
  explicit EggSwitchConditionDistance(double switch_in, double switch_out,
                                      const LPoint3d &center, double fade = 0.0);

  virtual EggSwitchCondition *make_copy() const;
  virtual void write(std::ostream &out, int indent_level) const;

  virtual void transform(const LMatrix4d &mat);

public:
  double _switch_in, _switch_out, _fade;
  LPoint3d _center;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggSwitchCondition::init_type();
    register_type(_type_handle, "EggSwitchConditionDistance",
                  EggSwitchCondition::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};


#endif
