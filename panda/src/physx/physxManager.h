// Filename: physxManager.h
// Created by:  pratt (Apr 6, 2006)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PHYSXMANAGER_H
#define PHYSXMANAGER_H

#ifdef HAVE_PHYSX

#include "pandabase.h"
#include "pointerTo.h"
#include "luse.h"

#include "physx_enumerations.h"

#include "NxPhysics.h"

class PhysxScene;
class PhysxSceneDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxManager
// Description : This is the central manager for the Panda interface
//               to the Ageia PhysX SDK.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxManager {
PUBLISHED:
  PhysxManager();
  ~PhysxManager();

// make enumerations available to Python interface
#undef PHYSX_ENUMERATIONS_H
#ifdef CPPPARSER
#include "physx_enumerations_for_interrogate.h"
#else
#include "physx_enumerations.h"
#endif

  void set_parameter(PhysxParameter parameterType, float value);
  float get_parameter(PhysxParameter parameterType);

  PhysxScene *create_scene(PhysxSceneDesc &sceneDesc);
  void release_scene(PhysxScene *scene);

  unsigned int get_num_scenes();
  PhysxScene *get_scene(unsigned int i);

  bool is_hardware_available();

public:
  INLINE static LVecBase3f nxVec3_to_lVecBase3(const NxVec3 &vec);
  INLINE static LMatrix3f nxMat33_to_lMatrix3(const NxMat33 &mat);
  INLINE static LMatrix4f nxMat34_to_lMatrix4(const NxMat34 &mat);
  INLINE static LQuaternionf nxQuat_to_lQuaternion(const NxQuat &quat);

  INLINE static NxVec3 lVecBase3_to_nxVec3(const LVecBase3f &vec);
  INLINE static NxMat33 lMatrix3_to_nxMat33(const LMatrix3f &mat);
  INLINE static NxMat34 lMatrix4_to_nxMat34(const LMatrix4f &mat);
  INLINE static NxQuat lQuaternion_to_nxQuat(const LQuaternionf &quat);

private:
  NxPhysicsSDK *nPhysicsSDK;
  NxRemoteDebugger *nRemoteDebugger;

  class PhysxOutputStream : public NxUserOutputStream {
    void reportError(NxErrorCode code, const char *message, const char *file, int line);
    NxAssertResponse reportAssertViolation(const char *message, const char *file, int line);
    void print(const char *message);
  };
  PhysxOutputStream debugStream;
};

#include "physxManager.I"

#endif // HAVE_PHYSX

#endif // PHYSXMANAGER_H
