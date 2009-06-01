// Filename: physxManager.cxx
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

#ifdef HAVE_PHYSX

#include "physxManager.h"
#include "config_physx.h"

#include "physxScene.h"
#include "physxSceneDesc.h"
#include "physxMaterial.h"

////////////////////////////////////////////////////////////////////
//     Function : PhysxManager
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxManager::
PhysxManager() {
  nPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, &debugStream);
  if(!nPhysicsSDK) {
    //TODO: handle better
  }
  if(physx_want_visual_debugger) {
    physx_info( "connecting to visual debugger at (" << physx_visual_debugger_host << ":" << physx_visual_debugger_port << ")" );
    nRemoteDebugger = nPhysicsSDK->getFoundationSDK().getRemoteDebugger();
    nRemoteDebugger->connect( physx_visual_debugger_host.c_str(), physx_visual_debugger_port );
    if(! nRemoteDebugger->isConnected()) {
      physx_warning( "Warning: could not connect to visual debugger" );
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysxManager
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxManager::
~PhysxManager() {
  nPhysicsSDK->release();
}

////////////////////////////////////////////////////////////////////
//     Function : set_parameter
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxManager::
set_parameter(PhysxParameter parameterType, float value) {
  nPhysicsSDK->setParameter((NxParameter)parameterType, value);
}

////////////////////////////////////////////////////////////////////
//     Function : get_parameter
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxManager::
get_parameter(PhysxParameter parameterType) {
  return nPhysicsSDK->getParameter((NxParameter)parameterType);
}

////////////////////////////////////////////////////////////////////
//     Function : create_scene
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxScene * PhysxManager::
create_scene(PhysxSceneDesc &sceneDesc) {
  // use active transforms
  sceneDesc.nSceneDesc.flags |= NX_SF_ENABLE_ACTIVETRANSFORMS;

  NxScene *nScene = nPhysicsSDK->createScene(sceneDesc.nSceneDesc);
  PhysxScene *pScene = new PhysxScene();
  nScene->userData = pScene;
  pScene->nScene = nScene;

  // hack job for now -- set some defaults for the default material
  NxMaterial *defaultMaterial = nScene->getMaterialFromIndex(0);
  defaultMaterial->setRestitution(0.0f);
  defaultMaterial->setStaticFriction(0.5f);
  defaultMaterial->setDynamicFriction(0.5f);

  // Create PhysxMaterial for default material
  PhysxMaterial *pMaterial = new PhysxMaterial();
  pMaterial->nMaterial = defaultMaterial;
  defaultMaterial->userData = pMaterial;

  return pScene;
}

////////////////////////////////////////////////////////////////////
//     Function : release_scene
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxManager::
release_scene(PhysxScene *scene) {
  nPhysicsSDK->releaseScene(*(scene->nScene));
  delete scene;
}

////////////////////////////////////////////////////////////////////
//     Function : get_num_scenes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxManager::
get_num_scenes() {
  return nPhysicsSDK->getNbScenes();
}

////////////////////////////////////////////////////////////////////
//     Function : get_scene
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxScene * PhysxManager::
get_scene(unsigned int i) {
  return (PhysxScene *)nPhysicsSDK->getScene(i)->userData;
}

////////////////////////////////////////////////////////////////////
//     Function : is_hardware_available
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxManager::
is_hardware_available() {
  return nPhysicsSDK->getHWVersion() != NX_HW_VERSION_NONE;
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxOutputStream::reportError
//       Access : private
//  Description : pass on error messages from the PhysX SDK
////////////////////////////////////////////////////////////////////
void PhysxManager::PhysxOutputStream::
reportError(NxErrorCode code, const char *message, const char *file, int line) {
  string output = "PhysX SDK Error: ";

  switch(code) {
    case NXE_NO_ERROR:
      output += "NO_ERROR - ";
      break;
    case NXE_INVALID_PARAMETER:
      output += "INVALID_PARAMETER - ";
      break;
    case NXE_INVALID_OPERATION:
      output += "INVALID_OPERATION - ";
      break;
    case NXE_OUT_OF_MEMORY:
      output += "OUT_OF_MEMORY - ";
      break;
    case NXE_INTERNAL_ERROR:
      output += "INTERNAL_ERROR - ";
      break;
    case NXE_ASSERTION:
      output += "ASSERTION - ";
      break;
    case NXE_DB_INFO:
      output += "DB_INFO - ";
      break;
    case NXE_DB_WARNING:
      output += "DB_WARNING - ";
      break;
    case NXE_DB_PRINT:
      output += "DB_PRINT - ";
      break;
  }

  output += message;

  if(code == NXE_NO_ERROR || code == NXE_DB_INFO || code == NXE_DB_PRINT) {
    physx_debug(output);
  } else {
    physx_warning(output);
  }
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxOutputStream::reportAssertViolation
//       Access : private
//  Description :
////////////////////////////////////////////////////////////////////
NxAssertResponse PhysxManager::PhysxOutputStream::
reportAssertViolation(const char *message, const char *file, int line) {
  return NX_AR_CONTINUE;
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxOutputStream::print
//       Access : private
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxManager::PhysxOutputStream::
print(const char *message) {
}

#endif // HAVE_PHYSX


