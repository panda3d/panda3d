/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxManager.h
 * @author enn0x
 * @date 2009-09-01
 */

#ifndef PHYSXMANAGER_H
#define PHYSXMANAGER_H

#include "pandabase.h"
#include "pointerTo.h"
#include "luse.h"

#include "physxEnums.h"
#include "physxObjectCollection.h"
#include "physx_includes.h"

class PhysxScene;
class PhysxSceneDesc;
class PhysxHeightField;
class PhysxHeightFieldDesc;
class PhysxTriangleMesh;
class PhysxConvexMesh;
class PhysxClothMesh;
class PhysxSoftBodyMesh;
class PhysxOutputStream;
class PhysxCcdSkeleton;
class PhysxCcdSkeletonDesc;

/**
 * The central interface to the PhysX subsystem.  Used e.  g.  for
 * setting/retrieving global parameters or for creating scenes.
 */
class EXPCL_PANDAPHYSX PhysxManager : public PhysxEnums {

protected:
  PhysxManager();

public:
  ~PhysxManager();

PUBLISHED:
  static PhysxManager *get_global_ptr();

  void set_parameter(PhysxParameter param, float value);
  float get_parameter(PhysxParameter param);

  bool is_hardware_available();
  unsigned int get_num_ppus();
  unsigned int get_hw_version();
  const char *get_internal_version();

  unsigned int get_num_scenes() const;
  PhysxScene *create_scene(PhysxSceneDesc &desc);
  PhysxScene *get_scene(unsigned int idx) const;
  MAKE_SEQ(get_scenes, get_num_scenes, get_scene);

  unsigned int get_num_height_fields();
  PhysxHeightField *create_height_field(PhysxHeightFieldDesc &desc);
  PhysxHeightField *get_height_field(unsigned int idx);
  MAKE_SEQ(get_height_fields, get_num_height_fields, get_height_field);

  unsigned int get_num_convex_meshes();
  PhysxConvexMesh *get_convex_mesh(unsigned int idx);
  MAKE_SEQ(get_convex_meshes, get_num_convex_meshes, get_convex_mesh);

  unsigned int get_num_triangle_meshes();
  PhysxTriangleMesh *get_triangle_mesh(unsigned int idx);
  MAKE_SEQ(get_triangle_meshes, get_num_triangle_meshes, get_triangle_mesh);

  unsigned int get_num_cloth_meshes();
  PhysxClothMesh *get_cloth_mesh(unsigned int idx);
  MAKE_SEQ(get_cloth_meshes, get_num_cloth_meshes, get_cloth_mesh);

  unsigned int get_num_soft_body_meshes();
  PhysxSoftBodyMesh *get_soft_body_mesh(unsigned int idx);
  MAKE_SEQ(get_soft_body_meshes, get_num_soft_body_meshes, get_soft_body_mesh);

  unsigned int get_num_ccd_skeletons();
  PhysxCcdSkeleton *create_ccd_skeleton(PhysxCcdSkeletonDesc &desc);
  PhysxCcdSkeleton *get_ccd_skeleton(unsigned int idx);
  MAKE_SEQ(get_ccd_skeletons, get_num_ccd_skeletons, get_ccd_skeleton);

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  INLINE NxPhysicsSDK *get_sdk() const;

  PhysxObjectCollection<PhysxScene> _scenes;
  PhysxObjectCollection<PhysxHeightField> _heightfields;
  PhysxObjectCollection<PhysxConvexMesh> _convex_meshes;
  PhysxObjectCollection<PhysxTriangleMesh> _triangle_meshes;
  PhysxObjectCollection<PhysxClothMesh> _cloth_meshes;
  PhysxObjectCollection<PhysxSoftBodyMesh> _softbody_meshes;
  PhysxObjectCollection<PhysxCcdSkeleton> _ccd_skeletons;

  INLINE static NxVec3 vec3_to_nxVec3(const LVector3f &v);
  INLINE static LVector3f nxVec3_to_vec3(const NxVec3 &v);
  INLINE static NxExtendedVec3 vec3_to_nxExtVec3(const LVector3f &v);
  INLINE static LVector3f nxExtVec3_to_vec3(const NxExtendedVec3 &v);
  INLINE static NxVec3 point3_to_nxVec3(const LPoint3f &p);
  INLINE static LPoint3f nxVec3_to_point3(const NxVec3 &p);
  INLINE static NxExtendedVec3 point3_to_nxExtVec3(const LPoint3f &p);
  INLINE static LPoint3f nxExtVec3_to_point3(const NxExtendedVec3 &p);
  INLINE static NxQuat quat_to_nxQuat(const LQuaternionf &q);
  INLINE static LQuaternionf nxQuat_to_quat(const NxQuat &q);
  INLINE static NxMat34 mat4_to_nxMat34(const LMatrix4f &m);
  INLINE static LMatrix4f nxMat34_to_mat4(const NxMat34 &m);
  INLINE static NxMat33 mat3_to_nxMat33(const LMatrix3f &m);
  INLINE static LMatrix3f nxMat33_to_mat3(const NxMat33 &m);

  INLINE static void update_vec3_from_nxVec3(LVector3f &v, const NxVec3 &nVec);
  INLINE static void update_point3_from_nxVec3(LPoint3f &p, const NxVec3 &nVec);

private:
  NxPhysicsSDK *_sdk;

  static PhysxManager *_global_ptr;

  class PhysxOutputStream : public NxUserOutputStream {
    void reportError(NxErrorCode code, const char *message, const char *file, int line);
    NxAssertResponse reportAssertViolation(const char *message, const char *file, int line);
    void print(const char *message);
    const char *get_error_code_string(NxErrorCode code);
  };
  static PhysxOutputStream _outputStream;

  static const char *get_sdk_error_string(const NxSDKCreateError &error);
};

#include "physxManager.I"

#endif // PHYSXMANAGER_H
