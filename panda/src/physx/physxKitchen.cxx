/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxKitchen.cxx
 * @author enn0x
 * @date 2009-10-12
 */

#include "physxKitchen.h"
#include "physxConvexMesh.h"
#include "physxConvexMeshDesc.h"
#include "physxTriangleMesh.h"
#include "physxTriangleMeshDesc.h"
#include "physxFileStream.h"
#include "physxMemoryReadBuffer.h"
#include "physxMemoryWriteBuffer.h"
#include "physxClothMesh.h"
#include "physxClothMeshDesc.h"
#include "physxSoftBodyMesh.h"
#include "physxSoftBodyMeshDesc.h"

/**
 * Sets two parameters which affect mesh cooking:
 *
 * Skin width for convex meshes: Specifies the amount to inflate the convex
 * mesh by when the new convex hull generator is used.  Inflating the mesh
 * allows the user to hide interpenetration errors by increasing the size of
 * the collision mesh with respect to the size of the rendered geometry.
 * Default value: 0.025f
 *
 * Hint to choose speed or less memory for collision structures.  Default
 * value: false
 */
void PhysxKitchen::
set_cooking_params(float skinWidth, bool hintCollisionSpeed) {

  NxCookingParams params;

  params.targetPlatform = PLATFORM_PC;
  params.skinWidth = skinWidth;
  params.hintCollisionSpeed = hintCollisionSpeed;

  _cooking->NxSetCookingParams(params);
}

/**
 *
 */
bool PhysxKitchen::
cook_convex_mesh(const PhysxConvexMeshDesc &meshDesc, const Filename &filename) {

  nassertr_always(!filename.empty(), false);
  nassertr_always(filename.touch(), false);
  nassertr_always(meshDesc.is_valid(), false);

  PhysxFileStream fs = PhysxFileStream(filename, false);
  return _cooking->NxCookConvexMesh(meshDesc.get_desc(), fs);
}

/**
 *
 */
bool PhysxKitchen::
cook_triangle_mesh(const PhysxTriangleMeshDesc &meshDesc, const Filename &filename) {

  nassertr_always(!filename.empty(), false);
  nassertr_always(filename.touch(), false);
  nassertr_always(meshDesc.is_valid(), false);

  PhysxFileStream fs = PhysxFileStream(filename, false);
  return _cooking->NxCookTriangleMesh(meshDesc.get_desc(), fs);
}

/**
 *
 */
bool PhysxKitchen::
cook_cloth_mesh(const PhysxClothMeshDesc &meshDesc, const Filename &filename) {

  nassertr_always(!filename.empty(), false);
  nassertr_always(filename.touch(), false);
  nassertr_always(meshDesc.is_valid(), false);

  PhysxFileStream fs = PhysxFileStream(filename, false);
  return _cooking->NxCookClothMesh(meshDesc.get_desc(), fs);
}

/**
 *
 */
bool PhysxKitchen::
cook_soft_body_mesh(const PhysxSoftBodyMeshDesc &meshDesc, const Filename &filename) {

  nassertr_always(!filename.empty(), false);
  nassertr_always(filename.touch(), false);
  nassertr_always(meshDesc.is_valid(), false);

  PhysxFileStream fs = PhysxFileStream(filename, false);
  return _cooking->NxCookSoftBodyMesh(meshDesc.get_desc(), fs);
}

/**
 *
 */
bool PhysxKitchen::
cook_texcoords(const PhysxClothMeshDesc &meshDesc, const Filename &filename) {

  nassertr_always(!filename.empty(), false);
  nassertr_always(filename.touch(), false);
  nassertr_always(meshDesc.is_valid(), false);

  const plist<LPoint2f> texcoords = meshDesc.get_texcoords();

  // Write texcoords to binary file
  PhysxFileStream fs = PhysxFileStream(filename.c_str(), false);

  // Header
  fs.storeByte('N');
  fs.storeByte('X');
  fs.storeByte('S');
  fs.storeByte(1);
  fs.storeByte('T');
  fs.storeByte('E');
  fs.storeByte('X');
  fs.storeByte('C');
  fs.storeByte(1);

  // Size
  fs.storeDword(texcoords.size());

  // Texcoords
  plist<LPoint2f>::const_iterator it;
  for(it=texcoords.begin(); it!=texcoords.end(); it++) {
    LPoint2f v = *it;

    fs.storeFloat(v.get_x());
    fs.storeFloat(v.get_y());
  }

  return true;
}

/**
 *
 */
PhysxConvexMesh *PhysxKitchen::
cook_convex_mesh(const PhysxConvexMeshDesc &meshDesc) {

  nassertr_always(meshDesc.is_valid(), nullptr);

  PhysxMemoryWriteBuffer buffer;
  bool status = _cooking->NxCookConvexMesh(meshDesc.get_desc(), buffer);
  nassertr(status, nullptr);

  NxPhysicsSDK *sdk = NxGetPhysicsSDK();
  nassertr(sdk, nullptr);

  PhysxConvexMesh *mesh = new PhysxConvexMesh();
  nassertr(mesh, nullptr);

  NxConvexMesh *meshPtr = sdk->createConvexMesh(PhysxMemoryReadBuffer(buffer.data));
  nassertr(meshPtr, nullptr);

  mesh->link(meshPtr);

  return mesh;
}

/**
 *
 */
PhysxTriangleMesh *PhysxKitchen::
cook_triangle_mesh(const PhysxTriangleMeshDesc &meshDesc) {

  nassertr_always(meshDesc.is_valid(), nullptr);

  PhysxMemoryWriteBuffer buffer;
  bool status = _cooking->NxCookTriangleMesh(meshDesc.get_desc(), buffer);
  nassertr(status, nullptr);

  NxPhysicsSDK *sdk = NxGetPhysicsSDK();
  nassertr(sdk, nullptr);

  PhysxTriangleMesh *mesh = new PhysxTriangleMesh();
  nassertr(mesh, nullptr);

  NxTriangleMesh *meshPtr = sdk->createTriangleMesh(PhysxMemoryReadBuffer(buffer.data));
  nassertr(meshPtr, nullptr);

  mesh->link(meshPtr);

  return mesh;
}

/**
 *
 */
PhysxClothMesh *PhysxKitchen::
cook_cloth_mesh(const PhysxClothMeshDesc &meshDesc) {

  nassertr_always(meshDesc.is_valid(), nullptr);

  PhysxMemoryWriteBuffer wbuffer;
  bool status = _cooking->NxCookClothMesh(meshDesc.get_desc(), wbuffer);
  nassertr(status, nullptr);

  NxPhysicsSDK *sdk = NxGetPhysicsSDK();
  nassertr(sdk, nullptr);

  PhysxClothMesh *mesh = new PhysxClothMesh();
  nassertr(mesh, nullptr);

  PhysxMemoryReadBuffer rbuffer(wbuffer.data);
  NxClothMesh *meshPtr = sdk->createClothMesh(rbuffer);
  nassertr(meshPtr, nullptr);

  mesh->link(meshPtr);

  return mesh;
}

/**
 *
 */
PhysxSoftBodyMesh *PhysxKitchen::
cook_soft_body_mesh(const PhysxSoftBodyMeshDesc &meshDesc) {

  nassertr_always(meshDesc.is_valid(), nullptr);

  PhysxMemoryWriteBuffer wbuffer;
  bool status = _cooking->NxCookSoftBodyMesh(meshDesc.get_desc(), wbuffer);
  nassertr(status, nullptr);

  NxPhysicsSDK *sdk = NxGetPhysicsSDK();
  nassertr(sdk, nullptr);

  PhysxSoftBodyMesh *mesh = new PhysxSoftBodyMesh();
  nassertr(mesh, nullptr);

  PhysxMemoryReadBuffer rbuffer(wbuffer.data);
  NxSoftBodyMesh *meshPtr = sdk->createSoftBodyMesh(rbuffer);
  nassertr(meshPtr, nullptr);

  mesh->link(meshPtr);

  return mesh;
}
