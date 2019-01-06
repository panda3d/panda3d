/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSoftBody.h
 * @author enn0x
 * @date 2010-09-13
 */

#ifndef PHYSXSOFTBODY_H
#define PHYSXSOFTBODY_H

#include "pandabase.h"
#include "luse.h"

#include "physxObject.h"
#include "physxObjectCollection.h"
#include "physxEnums.h"
#include "physx_includes.h"

class PhysxScene;
class PhysxGroupsMask;
class PhysxSoftBodyNode;

/**
 *
 */
class EXPCL_PANDAPHYSX PhysxSoftBody : public PhysxObject, public PhysxEnums {

PUBLISHED:
  INLINE PhysxSoftBody();
  INLINE ~PhysxSoftBody();

  PhysxScene *get_scene() const;
  PhysxSoftBodyNode *get_soft_body_node() const;
  PhysxSoftBodyNode *create_soft_body_node(const char *name);

  void set_name(const char *name);
  void set_flag(PhysxSoftBodyFlag flag, bool value);
  void set_groups_mask(const PhysxGroupsMask &mask);
  void set_group(unsigned int group);
  void set_solver_iterations(unsigned int iterations);
  void set_particle_radius(float radius);
#if NX_SDK_VERSION_NUMBER > 281
  void set_self_collision_thickness(float thickness);
  void set_hard_stretch_limitation_factor(float factor);
#endif
  void set_volume_stiffness(float stiffness);
  void set_stretching_stiffness(float stiffness);
  void set_damping_coefficient(float coef);
  void set_friction(float friction);
  void set_tear_factor(float factor);
  void set_attachment_tear_factor(float factor);

  const char *get_name() const;
  bool get_flag(PhysxSoftBodyFlag flag) const;
  PhysxGroupsMask get_groups_mask() const;
  unsigned int get_group() const;
  unsigned int get_num_particles();
  unsigned int get_solver_iterations() const;
  float get_particle_radius() const;
  float get_density() const;
  float get_relative_grid_spacing() const;
#if NX_SDK_VERSION_NUMBER > 281
  float get_self_collision_thickness() const;
  float get_hard_stretch_limitation_factor() const;
#endif
  float get_volume_stiffness() const;
  float get_stretching_stiffness() const;
  float get_damping_coefficient() const;
  float get_friction() const;
  float get_tear_factor() const;
  float get_attachment_tear_factor() const;

/*
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
*/

  // Sleeping
  bool is_sleeping() const;
  void wake_up(float wakeCounterValue=NX_SLEEP_INTERVAL);
  void put_to_sleep();
  void set_sleep_linear_velocity(float threshold);
  float get_sleep_linear_velocity() const;

/*
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
*/


/*
virtual void  getWorldBounds (NxBounds3 &bounds) const =0
virtual void  attachToShape (const NxShape *shape, NxU32 attachmentFlags)=0
virtual void  attachToCollidingShapes (NxU32 attachmentFlags)=0
virtual void  detachFromShape (const NxShape *shape)=0
virtual void  attachVertexToShape (NxU32 vertexId, const NxShape *shape, const NxVec3 &localPos, NxU32 attachmentFlags)=0
virtual void  attachVertexToGlobalPosition (const NxU32 vertexId, const NxVec3 &pos)=0
virtual void  freeVertex (const NxU32 vertexId)=0
virtual bool  tearVertex (const NxU32 vertexId, const NxVec3 &normal)=0
virtual bool  raycast (const NxRay &worldRay, NxVec3 &hit, NxU32 &vertexId)=0
virtual void  setMeshData (NxMeshData &meshData)=0
virtual NxMeshData  getMeshData ()=0
virtual void  setSplitPairData (NxSoftBodySplitPairData &splitPairData)=0
virtual NxSoftBodySplitPairData  getSplitPairData ()=0
virtual void  setValidBounds (const NxBounds3 &validBounds)=0
virtual void  getValidBounds (NxBounds3 &validBounds) const =0
virtual void  setPosition (const NxVec3 &position, NxU32 vertexId)=0
virtual void  setPositions (void *buffer, NxU32 byteStride=sizeof(NxVec3))=0
virtual NxVec3  getPosition (NxU32 vertexId) const =0
virtual void  getPositions (void *buffer, NxU32 byteStride=sizeof(NxVec3))=0
virtual void  setVelocity (const NxVec3 &velocity, NxU32 vertexId)=0
virtual void  setVelocities (void *buffer, NxU32 byteStride=sizeof(NxVec3))=0
virtual NxVec3  getVelocity (NxU32 vertexId) const =0
virtual void  getVelocities (void *buffer, NxU32 byteStride=sizeof(NxVec3))=0
virtual void  setConstrainPositions (void *buffer, NxU32 byteStride=sizeof(NxVec3))=0
virtual void  setConstrainNormals (void *buffer, NxU32 byteStride=sizeof(NxVec3))=0
virtual void  setConstrainCoefficients (const NxSoftBodyConstrainCoefficients *coefficients, NxU32 byteStride=sizeof(NxSoftBodyConstrainCoefficients))=0
virtual NxU32  queryShapePointers ()=0
virtual NxU32  getStateByteSize ()=0
virtual void  getShapePointers (NxShape **shapePointers, NxU32 *flags)=0
virtual void  setShapePointers (NxShape **shapePointers, unsigned int numShapes)=0
virtual void  saveStateToStream (NxStream &stream, bool permute=false)=0
virtual void  loadStateFromStream (NxStream &stream)=0
virtual void  setCollisionResponseCoefficient (NxReal coefficient)=0
virtual NxReal  getCollisionResponseCoefficient () const =0
virtual void  setAttachmentResponseCoefficient (NxReal coefficient)=0
virtual NxReal  getAttachmentResponseCoefficient () const =0
virtual void  setFromFluidResponseCoefficient (NxReal coefficient)=0
virtual NxReal  getFromFluidResponseCoefficient () const =0
virtual void  setToFluidResponseCoefficient (NxReal coefficient)=0
virtual NxReal  getToFluidResponseCoefficient () const =0
virtual void  setExternalAcceleration (NxVec3 acceleration)=0
virtual NxVec3  getExternalAcceleration () const =0
virtual void  setMinAdhereVelocity (NxReal velocity)=0
virtual NxReal  getMinAdhereVelocity () const =0
virtual void  addForceAtVertex (const NxVec3 &force, NxU32 vertexId, NxForceMode mode=NX_FORCE)=0
virtual void  addForceAtPos (const NxVec3 &position, NxReal magnitude, NxReal radius, NxForceMode mode=NX_FORCE)=0
virtual void  addDirectedForceAtPos (const NxVec3 &position, const NxVec3 &force, NxReal radius, NxForceMode mode=NX_FORCE)=0
virtual bool  overlapAABBTetrahedra (const NxBounds3 &bounds, NxU32 &nb, const NxU32 *&indices) const =0
virtual NxCompartment *  getCompartment () const =0
virtual NxForceFieldMaterial  getForceFieldMaterial () const =0
virtual void  setForceFieldMaterial (NxForceFieldMaterial)=0
*/

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  void update();

PUBLISHED:
  void release();

public:
  INLINE NxSoftBody *ptr() const { return _ptr; };

  void link(NxSoftBody *ptr);
  void unlink();

private:
  NxSoftBody *_ptr;
  PT(PhysxSoftBodyNode) _node;
  std::string _name;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxSoftBody",
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

#include "physxSoftBody.I"

#endif // PHYSXSOFTBODY_H
