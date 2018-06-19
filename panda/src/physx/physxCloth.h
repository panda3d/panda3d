/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCloth.h
 * @author enn0x
 * @date 2010-03-30
 */

#ifndef PHYSXCLOTH_H
#define PHYSXCLOTH_H

#include "pandabase.h"
#include "luse.h"

#include "physxObject.h"
#include "physxObjectCollection.h"
#include "physxEnums.h"
#include "physx_includes.h"

class PhysxScene;
class PhysxGroupsMask;
class PhysxClothNode;

/**
 *
 */
class EXPCL_PANDAPHYSX PhysxCloth : public PhysxObject, public PhysxEnums {

PUBLISHED:
  INLINE PhysxCloth();
  INLINE ~PhysxCloth();

  PhysxScene *get_scene() const;
  PhysxClothNode *get_cloth_node() const;
  PhysxClothNode *create_cloth_node(const char *name);

  void set_name(const char *name);
  void set_group(unsigned int group);
  void set_groups_mask(const PhysxGroupsMask &mask);
  void set_flag(PhysxClothFlag flag, bool value);
  void set_thickness(float thickness);

  const char *get_name() const;
  unsigned int get_num_particles();
  unsigned int get_group() const;
  PhysxGroupsMask get_groups_mask() const;
  bool get_flag(PhysxClothFlag flag) const;
  float get_thickness() const;
  float get_density() const;
  float get_relative_grid_spacing() const;

  // Attachment
  void attach_vertex_to_global_pos(unsigned int vertexId, LPoint3f const &pos);
  void free_vertex(unsigned int vertexId);
  void attach_to_shape(PhysxShape *shape);
  void attach_to_colliding_shapes();
  void detach_from_shape(PhysxShape *shape);
  void attach_vertex_to_shape(unsigned int vertexId, PhysxShape *shape, LPoint3f const &localPos);
  PhysxVertexAttachmentStatus get_vertex_attachment_status(unsigned int vertexId) const;
  PhysxShape *get_vertex_attachment_shape(unsigned int vertexId) const;
  LPoint3f get_vertex_attachment_pos(unsigned int vertexId) const;

  // Sleeping
  bool is_sleeping() const;
  void wake_up(float wakeCounterValue=NX_SLEEP_INTERVAL);
  void put_to_sleep();
  void set_sleep_linear_velocity(float threshold);
  float get_sleep_linear_velocity() const;

  // Forces
  void set_external_acceleration(LVector3f const &acceleration);
  LVector3f get_external_acceleration() const;

  void set_wind_acceleration(LVector3f const &acceleration);
  LVector3f get_wind_acceleration() const;

  void add_force_at_vertex(LVector3f const &force, int vertexId,
                           PhysxForceMode mode=FM_force);
  void add_force_at_pos(LPoint3f const &pos, float magnitude, float radius,
                        PhysxForceMode mode=FM_force);
  void add_directed_force_at_pos(LPoint3f const &pos, LVector3f const &force, float radius,
                                 PhysxForceMode mode=FM_force);

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  void update();

PUBLISHED:
  void release();

public:
  INLINE NxCloth *ptr() const { return _ptr; };

  void link(NxCloth *ptr);
  void unlink();

private:
  NxCloth *_ptr;
  PT(PhysxClothNode) _node;
  std::string _name;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxCloth",
                  PhysxObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "physxCloth.I"

#endif // PHYSXCLOTH_H
