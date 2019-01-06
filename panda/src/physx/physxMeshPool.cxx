/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMeshPool.cxx
 * @author enn0x
 * @date 2009-10-14
 */

#include "physxMeshPool.h"
#include "physxConvexMesh.h"
#include "physxTriangleMesh.h"
#include "physxClothMesh.h"
#include "physxSoftBodyMesh.h"

#include "physxFileStream.h"
#include "virtualFileSystem.h"

PhysxMeshPool::ConvexMeshes PhysxMeshPool::_convex_meshes;
PhysxMeshPool::TriangleMeshes PhysxMeshPool::_triangle_meshes;
PhysxMeshPool::ClothMeshes PhysxMeshPool::_cloth_meshes;
PhysxMeshPool::SoftbodyMeshes PhysxMeshPool::_softbody_meshes;

/**
 *
 */
bool PhysxMeshPool::
check_filename(const Filename &fn) {

  if (!(VirtualFileSystem::get_global_ptr()->exists(fn))) {
    physx_cat.error() << "File does not exists: " << fn << std::endl;
    return false;
  }

  if (!(VirtualFileSystem::get_global_ptr()->is_regular_file(fn))) {
    physx_cat.error() << "Not a regular file: " << fn << std::endl;
    return false;
  }

  return true;
}

/**
 *
 */
PhysxConvexMesh *PhysxMeshPool::
load_convex_mesh(const Filename &fn) {

  if (!check_filename(fn)) return nullptr;

  PhysxConvexMesh *mesh;

  ConvexMeshes::iterator it = _convex_meshes.find(fn);
  if (it == _convex_meshes.end()) {
    // Not found; load mesh.
    NxConvexMesh *meshPtr;
    PhysxFileStream stream = PhysxFileStream(fn, true);

    mesh = new PhysxConvexMesh();
    nassertr_always(mesh, nullptr);

    NxPhysicsSDK *sdk = NxGetPhysicsSDK();
    nassertr_always(sdk, nullptr);

    meshPtr = sdk->createConvexMesh(stream);
    nassertr_always(meshPtr, nullptr);

    mesh->link(meshPtr);

    _convex_meshes.insert(ConvexMeshes::value_type(fn, mesh));
  }
  else {
    // Found; return previously loaded mesh.
    mesh = (*it).second;
  }

  return mesh;
}

/**
 *
 */
PhysxTriangleMesh *PhysxMeshPool::
load_triangle_mesh(const Filename &fn) {

  if (!check_filename(fn)) return nullptr;

  PhysxTriangleMesh *mesh;

  TriangleMeshes::iterator it = _triangle_meshes.find(fn);
  if (it == _triangle_meshes.end()) {
    // Not found; load mesh.
    NxTriangleMesh *meshPtr;
    PhysxFileStream stream = PhysxFileStream(fn, true);

    mesh = new PhysxTriangleMesh();
    nassertr_always(mesh, nullptr);

    NxPhysicsSDK *sdk = NxGetPhysicsSDK();
    nassertr_always(sdk, nullptr);

    meshPtr = sdk->createTriangleMesh(stream);
    nassertr_always(meshPtr, nullptr);

    mesh->link(meshPtr);

    _triangle_meshes.insert(TriangleMeshes::value_type(fn, mesh));
  }
  else {
    // Found; return previously loaded mesh.
    mesh = (*it).second;
  }

  return mesh;
}

/**
 *
 */
PhysxClothMesh *PhysxMeshPool::
load_cloth_mesh(const Filename &fn) {

  if (!check_filename(fn)) return nullptr;

  PhysxClothMesh *mesh;

  ClothMeshes::iterator it = _cloth_meshes.find(fn);
  if (it == _cloth_meshes.end()) {
    // Not found; load mesh.
    NxClothMesh *meshPtr;
    PhysxFileStream stream = PhysxFileStream(fn, true);

    mesh = new PhysxClothMesh();
    nassertr_always(mesh, nullptr);

    NxPhysicsSDK *sdk = NxGetPhysicsSDK();
    nassertr_always(sdk, nullptr);

    meshPtr = sdk->createClothMesh(stream);
    nassertr_always(meshPtr, nullptr);

    mesh->link(meshPtr);

    _cloth_meshes.insert(ClothMeshes::value_type(fn, mesh));
  }
  else {
    // Found; return previously loaded mesh.
    mesh = (*it).second;
  }

  return mesh;
}

/**
 *
 */
PhysxSoftBodyMesh *PhysxMeshPool::
load_soft_body_mesh(const Filename &fn) {

  if (!check_filename(fn)) return nullptr;

  PhysxSoftBodyMesh *mesh;

  SoftbodyMeshes::iterator it = _softbody_meshes.find(fn);
  if (it == _softbody_meshes.end()) {
    // Not found; load mesh.
    NxSoftBodyMesh *meshPtr;
    PhysxFileStream stream = PhysxFileStream(fn, true);

    mesh = new PhysxSoftBodyMesh();
    nassertr_always(mesh, nullptr);

    NxPhysicsSDK *sdk = NxGetPhysicsSDK();
    nassertr_always(sdk, nullptr);

    meshPtr = sdk->createSoftBodyMesh(stream);
    nassertr_always(meshPtr, nullptr);

    mesh->link(meshPtr);

    _softbody_meshes.insert(SoftbodyMeshes::value_type(fn, mesh));
  }
  else {
    // Found; return previously loaded mesh.
    mesh = (*it).second;
  }

  return mesh;
}

/**
 *
 */
bool PhysxMeshPool::
release_convex_mesh(PhysxConvexMesh *mesh) {

  ConvexMeshes::iterator it;
  for (it=_convex_meshes.begin(); it != _convex_meshes.end(); ++it) {
    if (mesh == (*it).second) {
      _convex_meshes.erase(it);
      return true;
    }
  }

  return false;
}

/**
 *
 */
bool PhysxMeshPool::
release_triangle_mesh(PhysxTriangleMesh *mesh) {

  TriangleMeshes::iterator it;
  for (it=_triangle_meshes.begin(); it != _triangle_meshes.end(); ++it) {
    if (mesh == (*it).second) {
      _triangle_meshes.erase(it);
      return true;
    }
  }

  return false;
}

/**
 *
 */
bool PhysxMeshPool::
release_cloth_mesh(PhysxClothMesh *mesh) {

  ClothMeshes::iterator it;
  for (it=_cloth_meshes.begin(); it != _cloth_meshes.end(); ++it) {
    if (mesh == (*it).second) {
      _cloth_meshes.erase(it);
      return true;
    }
  }

  return false;
}

/**
 *
 */
bool PhysxMeshPool::
release_soft_body_mesh(PhysxSoftBodyMesh *mesh) {

  SoftbodyMeshes::iterator it;
  for (it=_softbody_meshes.begin(); it != _softbody_meshes.end(); ++it) {
    if (mesh == (*it).second) {
      _softbody_meshes.erase(it);
      return true;
    }
  }

  return false;
}

/**
 *
 */
void PhysxMeshPool::
list_contents() {
  list_contents(nout);
}

/**
 *
 */
void PhysxMeshPool::
list_contents(std::ostream &out) {

  out << "PhysX mesh pool contents:\n";

  // Convex meshes
  {
    ConvexMeshes::const_iterator it;
    for (it=_convex_meshes.begin(); it != _convex_meshes.end(); ++it) {
      Filename fn = (*it).first;
      PhysxConvexMesh *mesh = (*it).second;

      out << "  " << fn.get_fullpath()
          << " (convex mesh, " << mesh->ptr()->getReferenceCount()
          << " references)" << std::endl;
    }
  }

  // Triangle meshes
  {
    TriangleMeshes::const_iterator it;
    for (it=_triangle_meshes.begin(); it != _triangle_meshes.end(); ++it) {
      Filename fn = (*it).first;
      PhysxTriangleMesh *mesh = (*it).second;

      out << "  " << fn.get_fullpath()
          << " (triangle mesh, " << mesh->ptr()->getReferenceCount()
          << " references)\n";
    }
  }

  // Cloth meshes
  {
    ClothMeshes::const_iterator it;
    for (it=_cloth_meshes.begin(); it != _cloth_meshes.end(); ++it) {
      Filename fn = (*it).first;
      PhysxClothMesh *mesh = (*it).second;

      out << "  " << fn.get_fullpath()
          << " (cloth mesh, " << mesh->ptr()->getReferenceCount()
          << " references)\n";
    }
  }

  // Soft body meshes
  {
    SoftbodyMeshes::const_iterator it;
    for (it=_softbody_meshes.begin(); it != _softbody_meshes.end(); ++it) {
      Filename fn = (*it).first;
      PhysxSoftBodyMesh *mesh = (*it).second;

      out << "  " << fn.get_fullpath()
          << " (soft body mesh, " << mesh->ptr()->getReferenceCount()
          << " references)\n";
    }
  }

  // Summary
  NxPhysicsSDK *sdk = NxGetPhysicsSDK();

  out << "  Total number of convex meshes: " << sdk->getNbConvexMeshes()
      << " created, " << _convex_meshes.size() << " registred\n";

  out << "  Total number of triangle meshes: " << sdk->getNbTriangleMeshes()
      << " created, " << _triangle_meshes.size() << " registred\n";

  out << "  Total number of cloth meshes: " << sdk->getNbClothMeshes()
      << " created, " << _cloth_meshes.size() << " registred\n";

  out << "  Total number of soft body meshes: " << sdk->getNbSoftBodyMeshes()
      << " created, " << _softbody_meshes.size() << " registred\n";
}
