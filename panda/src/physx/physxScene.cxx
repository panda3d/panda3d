/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxScene.cxx
 * @author enn0x
 * @date 2009-09-14
 */

#include "physxScene.h"
#include "physxManager.h"
#include "physxActorDesc.h"
#include "physxForceFieldDesc.h"
#include "physxForceFieldShapeGroupDesc.h"
#include "physxControllerDesc.h"
#include "physxSceneStats2.h"
#include "physxConstraintDominance.h"
#include "physxVehicle.h"
#include "physxVehicleDesc.h"
#include "physxCloth.h"
#include "physxClothDesc.h"
#include "physxSoftBody.h"
#include "physxSoftBodyDesc.h"

TypeHandle PhysxScene::_type_handle;

PStatCollector PhysxScene::_pcollector_fetch_results("App:PhysX:Fetch Results");
PStatCollector PhysxScene::_pcollector_update_transforms("App:PhysX:Update Transforms");
PStatCollector PhysxScene::_pcollector_debug_renderer("App:PhysX:Debug Renderer");
PStatCollector PhysxScene::_pcollector_simulate("App:PhysX:Simulate");
PStatCollector PhysxScene::_pcollector_cloth("App:PhysX:Cloth");
PStatCollector PhysxScene::_pcollector_softbody("App:PhysX:Softbody");

/**
 *
 */
void PhysxScene::
link(NxScene *scenePtr) {

  // Link self
  _ptr = scenePtr;
  _ptr->userData = this;
  _error_type = ET_ok;
  PhysxManager::get_global_ptr()->_scenes.add(this);

  _cm = NxCreateControllerManager(NxGetPhysicsSDKAllocator());
  nassertv_always(_cm);

  // Link materials
  NxMaterial *materials[5];
  NxU32 iterator = 0;

  while (NxU32 i=_ptr->getMaterialArray(materials, 5, iterator)) {
    while(i--) {
      PhysxMaterial *material = new PhysxMaterial();
      material->link(materials[i]);
    }
  }
}

/**
 *
 */
void PhysxScene::
unlink() {

  // Unlink vehicles
  for (unsigned int i=0; i < _vehicles.size(); i++) {
    _vehicles[i]->release();
  }

  // Unlink controllers
  NxU32 nControllers = _cm->getNbControllers();

  for (NxU32 i=0; i < nControllers; i++) {
    NxController *controllerPtr = _cm->getController(i);
    PhysxController *controller = (PhysxController *)controllerPtr->getUserData();
    controller->unlink();
  }

  // Unlink actors
  NxActor **actors = _ptr->getActors();
  NxU32 nActors = _ptr->getNbActors();

  for (NxU32 i=0; i < nActors; i++) {
    PhysxActor *actor = (PhysxActor *)actors[i]->userData;

    // Actor could have already been unlinked by controller
    if (actor) {
      actor->unlink();
    }
  }

  // Unlink joints
  NxU32 nJoints = _ptr->getNbJoints();

  _ptr->resetJointIterator();
  for (NxU32 i=0; i < nJoints; i++) {
    NxJoint *jointPtr = _ptr->getNextJoint();
    PhysxJoint *joint = (PhysxJoint *)jointPtr->userData;
    joint->unlink();
  }

  // Unlink force fields
  NxForceField **fields = _ptr->getForceFields();
  NxU32 nFields = _ptr->getNbForceFields();

  for (NxU32 i=0; i < nFields; i++) {
    PhysxForceField *field = (PhysxForceField *)fields[i]->userData;
    field->unlink();
  }

  // Unlink force field shape groups
  NxU32 nGroups = _ptr->getNbForceFieldShapeGroups();

  _ptr->resetForceFieldShapeGroupsIterator();
  for (NxU32 i=0; i < nGroups; i++) {
    NxForceFieldShapeGroup *groupPtr = _ptr->getNextForceFieldShapeGroup();
    PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)groupPtr->userData;
    group->unlink();
  }

  // Unlink cloths
  NxCloth **cloths = _ptr->getCloths();
  NxU32 nCloths = _ptr->getNbCloths();

  for (NxU32 i=0; i < nCloths; i++) {
    PhysxCloth *cloth = (PhysxCloth *)cloths[i]->userData;
    cloth->unlink();
  }

  // Unlink softbodies
  NxSoftBody **softbodies = _ptr->getSoftBodies();
  NxU32 nSoftbodies = _ptr->getNbSoftBodies();

  for (NxU32 i=0; i < nSoftbodies; i++) {
    PhysxSoftBody *softbody = (PhysxSoftBody *)softbodies[i]->userData;
    softbody->unlink();
  }

  // Unlink materials
  NxMaterial *materials[5];
  NxU32 iterator = 0;

  while (NxU32 i=_ptr->getMaterialArray(materials, 5, iterator)) {
    while(i--) {
      PhysxMaterial *material = (PhysxMaterial *)materials[i]->userData;
      material->unlink();
    }
  }

  // Unlink self
  _cm->purgeControllers();
  NxReleaseControllerManager(_cm);

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxManager::get_global_ptr()->_scenes.remove(this);
}

/**
 *
 */
void PhysxScene::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxPhysicsSDK *sdk = NxGetPhysicsSDK();
  sdk->releaseScene(*_ptr);
  _ptr = nullptr;
}

/**
 * Advances the simulation by an elapsedTime time.  The elapsed time has to be
 * in the range (0, inf).
 *
 * It is not allowed to modify the physics scene in between the simulate(dt)
 * and the fetch_results calls!  But it is allowed to read from the scene and
 * do additional computations, e.  g.  AI, in between these calls.
 */
void PhysxScene::
simulate(float dt) {

  nassertv(_error_type == ET_ok);

  _pcollector_simulate.start();

  // Update all vehicles
  for (unsigned int i=0; i < _vehicles.size(); i++) {
    PhysxVehicle *vehicle = _vehicles[i];
    vehicle->update_vehicle(dt);
  }

  // Update all controllers
  for (NxU32 i=0; i < _cm->getNbControllers(); i++) {
    NxController *controllerPtr = _cm->getController(i);
    PhysxController *controller = (PhysxController *)controllerPtr->getUserData();
    controller->update_controller(dt);
  }

  _cm->updateControllers();

  // Simulate and flush streams
  _ptr->simulate(dt);
  _ptr->flushStream();

  _pcollector_simulate.stop();
}

/**
 * Waits until the simulation has finished, and then updates the scene graph
 * with with simulation results.
 *
 * It is not allowed to modify the physics scene in between the simulate(dt)
 * and the fetch_results calls!  But it is allowed to read from the scene and
 * do additional computations, e.  g.  AI, in between these calls.
 */
void PhysxScene::
fetch_results() {

  nassertv(_error_type == ET_ok);
  nassertv(_ptr != nullptr);

  _pcollector_fetch_results.start();
  _ptr->fetchResults(NX_RIGID_BODY_FINISHED, true);
  _pcollector_fetch_results.stop();

  // Update node transforms
  _pcollector_update_transforms.start();

  NxU32 nbTransforms = 0;
  NxActiveTransform *activeTransforms = _ptr->getActiveTransforms(nbTransforms);

  if (nbTransforms && activeTransforms) {
    for (NxU32 i=0; i<nbTransforms; ++i) {

      // Objects created by the Visual Remote Debugger might not have user
      // data.  So check if user data ist set.
      void *userData = activeTransforms[i].userData;
      if (userData) {
        LMatrix4f m = PhysxManager::nxMat34_to_mat4(activeTransforms[i].actor2World);
        PhysxActor *actor = (PhysxActor *)userData;
        actor->update_transform(m);
      }
    }
  }
  _pcollector_update_transforms.stop();

  // Update debug node
  _pcollector_debug_renderer.start();
  _debugNode->update(_ptr);
  _pcollector_debug_renderer.stop();

  nassertv(_ptr->isWritable());

  // Update cloth nodes
  _pcollector_cloth.start();

  NxCloth **cloths = _ptr->getCloths();
  for (NxU32 i=0; i < _ptr->getNbCloths(); i++) {
    PT(PhysxCloth) cloth = (PhysxCloth *)cloths[i]->userData;
    cloth->update();
  }

  _pcollector_cloth.stop();

  // Update softbody nodes
  _pcollector_softbody.start();

  NxSoftBody **softbodies = _ptr->getSoftBodies();
  for (NxU32 i=0; i < _ptr->getNbSoftBodies(); i++) {
    PT(PhysxSoftBody) softbody = (PhysxSoftBody *)softbodies[i]->userData;
    softbody->update();
  }

  _pcollector_softbody.stop();
}

/**
 * Sets simulation timing parameters used in simulate.
 */
void PhysxScene::
set_timing_variable() {

  nassertv(_error_type == ET_ok);
  _ptr->setTiming(0, 0, NX_TIMESTEP_VARIABLE);
}

/**
 * Sets simulation timing parameters used in simulate.  The elapsed time
 * (parameter "dt" in simulate()) is internally subdivided into up to maxIter
 * substeps no larger than maxTimestep.  If the elapsed time is not a multiple
 * of maxTimestep then any remaining time is accumulated to be added onto the
 * elapsed time for the next time step.  If more sub steps than maxIter are
 * needed to advance the simulation by elapsed time, then the remaining time
 * is also accumulated for the next call to simulate().
 *
 * This timing method is strongly preferred for stable, reproducible
 * simulation.
 */
void PhysxScene::
set_timing_fixed(float maxTimestep, unsigned int maxIter) {

  nassertv(_error_type == ET_ok);
  _ptr->setTiming(maxTimestep, maxIter, NX_TIMESTEP_FIXED);
}

/**
 * Sets a constant gravity for the entire scene.
 */
void PhysxScene::
set_gravity(const LVector3f &gravity) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!gravity.is_nan());

  _ptr->setGravity(PhysxManager::vec3_to_nxVec3(gravity));
}

/**
 * Retrieves the current gravity setting.
 */
LVector3f PhysxScene::
get_gravity() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());

  NxVec3 gravity;
  _ptr->getGravity(gravity);
  return PhysxManager::nxVec3_to_vec3(gravity);
}

/**
 *
 */
unsigned int PhysxScene::
get_num_actors() const {

  nassertr(_error_type == ET_ok,-1);

  return _ptr->getNbActors();
}

/**
 *
 */
PhysxActor *PhysxScene::
create_actor(PhysxActorDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr(desc.is_valid(), nullptr);

  PhysxActor *actor = new PhysxActor();
  nassertr(actor, nullptr);

  NxActor *actorPtr = _ptr->createActor(desc._desc);
  nassertr(actorPtr, nullptr);

  actor->link(actorPtr);

  return actor;
}

/**
 *
 */
PhysxActor *PhysxScene::
get_actor(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _ptr->getNbActors(), nullptr);

  NxActor *actorPtr = _ptr->getActors()[idx];
  PhysxActor *actor = (PhysxActor *)(actorPtr->userData);

  return actor;
}

/**
 * Retrieves the debug geom node for this scene.  The debug geom node is used
 * to visualize information about the physical scene which can be useful for
 * debugging an application.
 *
 * The debug geom node geometry is generated in global coordinates.  In order
 * to see correct information it is important not to dislocate the debug node.
 * Reparent it to render and leave position at (0,0,0).
 */
PhysxDebugGeomNode *PhysxScene::
get_debug_geom_node() {

  nassertr(_error_type == ET_ok, nullptr);
  return _debugNode;
}

/**
 *
 */
void PhysxScene::
enable_contact_reporting(bool enabled) {

  nassertv(_error_type == ET_ok);

  if (enabled) {
    _ptr->setUserContactReport(&_contact_report);
    _contact_report.enable();
  }
  else {
    _ptr->setUserContactReport(nullptr);
    _contact_report.disable();
  }
}

/**
 *
 */
bool PhysxScene::
is_contact_reporting_enabled() const {

  nassertr(_error_type == ET_ok, false);

  return _contact_report.is_enabled();
}

/**
 *
 */
void PhysxScene::
enable_trigger_reporting(bool enabled) {

  nassertv(_error_type == ET_ok);

  if (enabled) {
    _ptr->setUserTriggerReport(&_trigger_report);
    _trigger_report.enable();
  }
  else {
    _ptr->setUserTriggerReport(nullptr);
    _trigger_report.disable();
  }
}

/**
 *
 */
bool PhysxScene::
is_trigger_reporting_enabled() const {

  nassertr(_error_type == ET_ok, false);

  return _trigger_report.is_enabled();
}

/**
 *
 */
void PhysxScene::
enable_controller_reporting(bool enabled) {

  nassertv(_error_type == ET_ok);

  if (enabled) {
    _controller_report.enable();
  }
  else {
    _controller_report.disable();
  }
}

/**
 *
 */
bool PhysxScene::
is_controller_reporting_enabled() const {

  nassertr(_error_type == ET_ok, false);

  return _controller_report.is_enabled();
}

/**
 * Return the number of materials in the scene.
 *
 * Note that the returned value is not related to material indices.  Those may
 * not be allocated continuously, and its values may be higher than
 * get_num_materials(). This will also include the default material which
 * exists without having to be created.
 */
unsigned int PhysxScene::
get_num_materials() const {

  nassertr(_error_type == ET_ok, -1);
  return _ptr->getNbMaterials();
}

/**
 * Creates a new PhysxMaterial.
 *
 * The material library consists of an array of material objects.  Each
 * material has a well defined index that can be used to refer to it.  If an
 * object references an undefined material, the default material with index 0
 * is used instead.
 */
PhysxMaterial *PhysxScene::
create_material(PhysxMaterialDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr(desc.is_valid(), nullptr);

  PhysxMaterial *material = new PhysxMaterial();
  nassertr(material, nullptr);

  NxMaterial *materialPtr = _ptr->createMaterial(desc._desc);
  nassertr(materialPtr, nullptr);

  material->link(materialPtr);

  return material;
}

/**
 * Creates a new PhysxMaterial using the default settings of
 * PhysxMaterialDesc.
 */
PhysxMaterial *PhysxScene::
create_material() {

  nassertr(_error_type == ET_ok, nullptr);

  PhysxMaterial *material = new PhysxMaterial();
  nassertr(material, nullptr);

  NxMaterialDesc desc;
  desc.setToDefault();
  NxMaterial *materialPtr = _ptr->createMaterial(desc);
  nassertr(materialPtr, nullptr);

  material->link(materialPtr);

  return material;
}

/**
 * Returns current highest valid material index.
 *
 * Note that not all indices below this are valid if some of them belong to
 * meshes that have beed freed.
 */
unsigned int PhysxScene::
get_hightest_material_index() const {

  nassertr(_error_type == ET_ok, -1);
  return _ptr->getHighestMaterialIndex();
}

/**
 * Retrieves the material with the given material index.
 *
 * There is always at least one material in the Scene, the default material
 * (index 0). If the specified material index is out of range (larger than
 * get_hightest_material_index) or belongs to a material that has been
 * released, then the default material is returned, but no error is reported.
 */
PhysxMaterial *PhysxScene::
get_material_from_index(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);

  NxMaterial *materialPtr = _ptr->getMaterialFromIndex(idx);

  return (PhysxMaterial *)(materialPtr->userData);
}

/**
 * Retrieves the n-th material from the array of materials.  See also
 * get_material_from_index, which retrieves a material by it's material index.
 */
PhysxMaterial *PhysxScene::
get_material(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _ptr->getNbMaterials(), nullptr);

  NxU32 n = _ptr->getNbMaterials();
  NxMaterial **materials = new NxMaterial *[n];
  NxU32 materialCount;
  NxU32 iterator = 0;

  materialCount = _ptr->getMaterialArray(materials, n, iterator);
  nassertr((materialCount == n), nullptr);

  NxMaterial *materialPtr = materials[idx];
  delete[] materials;

  return (PhysxMaterial *)(materialPtr->userData);
}

/**
 * Return the number of controllers in the scene.
 */
unsigned int PhysxScene::
get_num_controllers() const {

  nassertr(_error_type == ET_ok, -1);
  return _cm->getNbControllers();
}


/**
 * Creates a new character controller.
 */
PhysxController *PhysxScene::
create_controller(PhysxControllerDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr(desc.is_valid(), nullptr);

  PhysxController *controller = PhysxController::factory(desc.ptr()->getType());
  nassertr(controller, nullptr);

  desc.ptr()->callback = &_controller_report;
  desc.ptr()->userData = controller;

  NxController *controllerPtr = _cm->createController(_ptr,*desc.ptr());
  nassertr(controllerPtr, nullptr);

  controller->link(controllerPtr);
  controller->get_actor()->set_name("");

  return controller;
}

/**
 * Retrieves the n-th controller within the scene.
 */
PhysxController *PhysxScene::
get_controller(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _cm->getNbControllers(), nullptr);

  NxController *controllerPtr = _cm->getController(idx);
  PhysxController *controller = (PhysxController *)(controllerPtr->getUserData());

  return controller;
}

/**
 * Returns the number of joints in the scene (excluding "dead" joints). Note
 * that this includes compartments.
 */
unsigned int PhysxScene::
get_num_joints() const {

  nassertr(_error_type == ET_ok, -1);
  return _ptr->getNbJoints();
}

/**
 * Creates a joint in this scene.
 */
PhysxJoint *PhysxScene::
create_joint(PhysxJointDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr(desc.is_valid(), nullptr);

  PhysxJoint *joint = PhysxJoint::factory(desc.ptr()->getType());
  nassertr(joint, nullptr);

  NxJoint *jointPtr = _ptr->createJoint(*desc.ptr());
  nassertr(jointPtr, nullptr);

  joint->link(jointPtr);

  return joint;
}

/**
 * Retrieve the n-th joint from the array of all the joints in the scene.
 */
PhysxJoint *PhysxScene::
get_joint(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _ptr->getNbJoints(), nullptr);

  NxJoint *jointPtr;
  NxU32 nJoints = _ptr->getNbJoints();

  _ptr->resetJointIterator();
  for (NxU32 i=0; i <= idx; i++) {
    jointPtr = _ptr->getNextJoint();
  }

  return (PhysxJoint *)(jointPtr->userData);
}

/**
 * Gets the number of force fields in the scene.
 */
unsigned int PhysxScene::
get_num_force_fields() const {

  nassertr(_error_type == ET_ok, -1);
  return _ptr->getNbForceFields();
}

/**
 * Creates a force field in this scene.
 */
PhysxForceField *PhysxScene::
create_force_field(PhysxForceFieldDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);

  // Create the kernel
  desc.create_kernel(_ptr);
  nassertr(desc.is_valid(), nullptr);

  // Create the force field
  PhysxForceField *field = new PhysxForceField();
  nassertr(field, nullptr);

  NxForceField *fieldPtr = _ptr->createForceField(desc._desc);
  nassertr(fieldPtr, nullptr);

  field->link(fieldPtr);

  return field;
}

/**
 * Returns the n-th force field from the array of all the force fields in the
 * scene.
 */
PhysxForceField *PhysxScene::
get_force_field(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _ptr->getNbForceFields(), nullptr);

  NxForceField **fields = _ptr->getForceFields();
  NxForceField *fieldPtr = fields[idx];

  return (PhysxForceField *)(fieldPtr->userData);
}

/**
 * Gets the number of force field shape groups in the scene.
 */
unsigned int PhysxScene::
get_num_force_field_shape_groups() const {

  nassertr(_error_type == ET_ok, -1);
  return _ptr->getNbForceFieldShapeGroups();
}

/**
 * Creates a new force field shape group in this scene.
 */
PhysxForceFieldShapeGroup *PhysxScene::
create_force_field_shape_group(PhysxForceFieldShapeGroupDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);

  PhysxForceFieldShapeGroup *group = new PhysxForceFieldShapeGroup();
  nassertr(group, nullptr);

  NxForceFieldShapeGroup *groupPtr = _ptr->createForceFieldShapeGroup(desc._desc);
  nassertr(groupPtr, nullptr);

  group->link(groupPtr);

  return group;
}

/**
 * Returns the n-th force field shape group in this scene
 */
PhysxForceFieldShapeGroup *PhysxScene::
get_force_field_shape_group(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _ptr->getNbForceFieldShapeGroups(), nullptr);

  _ptr->resetForceFieldShapeGroupsIterator();
  NxForceFieldShapeGroup *groupPtr = nullptr;
  idx++;
  while (idx-- > 0) {
    groupPtr = _ptr->getNextForceFieldShapeGroup();
  }

  return groupPtr ? (PhysxForceFieldShapeGroup *)groupPtr->userData : nullptr;
}

/**
 * Gets the number of cloths in the scene.
 */
unsigned int PhysxScene::
get_num_cloths() const {

  nassertr(_error_type == ET_ok, -1);
  return _ptr->getNbCloths();
}

/**
 * Creates a cloth in this scene.
 */
PhysxCloth *PhysxScene::
create_cloth(PhysxClothDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);

  PhysxCloth *cloth = new PhysxCloth();
  nassertr(cloth, nullptr);

  NxCloth *clothPtr = _ptr->createCloth(desc._desc);
  nassertr(clothPtr, nullptr);

  cloth->link(clothPtr);

  return cloth;
}

/**
 * Returns the n-th cloth from the array of all the cloths in the scene.
 */
PhysxCloth *PhysxScene::
get_cloth(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _ptr->getNbCloths(), nullptr);

  NxCloth **cloths = _ptr->getCloths();
  NxCloth *clothPtr = cloths[idx];

  return (PhysxCloth *)(clothPtr->userData);
}

/**
 * Gets the number of soft bodies in the scene.
 */
unsigned int PhysxScene::
get_num_soft_bodies() const {

  nassertr(_error_type == ET_ok, -1);
  return _ptr->getNbSoftBodies();
}

/**
 * Creates a soft body in this scene.
 */
PhysxSoftBody *PhysxScene::
create_soft_body(PhysxSoftBodyDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);

  PhysxSoftBody *softbody = new PhysxSoftBody();
  nassertr(softbody, nullptr);

  NxSoftBody *softbodyPtr = _ptr->createSoftBody(desc._desc);
  nassertr(softbodyPtr, nullptr);

  softbody->link(softbodyPtr);

  return softbody;
}

/**
 * Returns the n-th soft body from the array of all the soft bodies in the
 * scene.
 */
PhysxSoftBody *PhysxScene::
get_soft_body(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _ptr->getNbSoftBodies(), nullptr);

  NxSoftBody **softbodies = _ptr->getSoftBodies();
  NxSoftBody *softbodyPtr = softbodies[idx];

  return (PhysxSoftBody *)(softbodyPtr->userData);
}

/**
 * Returns the number of vehicles in the scene.
 */
unsigned int PhysxScene::
get_num_vehicles() const {

  nassertr(_error_type == ET_ok, -1);
  return _vehicles.size();
}

/**
 * Creates a vehicle in this scene.
 */
PhysxVehicle *PhysxScene::
create_vehicle(PhysxVehicleDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr(desc.is_valid(), nullptr);

  PhysxVehicle *vehicle = new PhysxVehicle();
  nassertr(vehicle, nullptr);

  vehicle->create(this, desc);

  return vehicle;
}

/**
 * Returns the n-th vehicle from the array of all the vehicles in the scene.
 */
PhysxVehicle *PhysxScene::
get_vehicle(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _vehicles.size(), nullptr);

  return _vehicles[idx];
}

/**
 *
 */
PhysxSceneStats2 PhysxScene::
get_stats2() const {

  nassertr(_error_type == ET_ok, nullptr);
  return PhysxSceneStats2(_ptr->getStats2());
}

/**
 * Returns true if any shape is intersected by the ray.
 */
bool PhysxScene::
raycast_any_shape(const PhysxRay &ray,
                        PhysxShapesType shapesType,
                        PhysxMask mask,
                        PhysxGroupsMask *groups) const {

  nassertr(_error_type == ET_ok, false);

  NxGroupsMask *groupsPtr = groups ? &(groups->_mask) : nullptr;

  return _ptr->raycastAnyShape(ray._ray, (NxShapesType)shapesType,
                               mask.get_mask(), ray._length, groupsPtr);
}

/**
 * Returns the first shape that is hit along the ray.  If not shape is hit
 * then an empty raycast hit is returned (is_empty() == true).
 */
PhysxRaycastHit PhysxScene::
raycast_closest_shape(const PhysxRay &ray,
                            PhysxShapesType shapesType,
                            PhysxMask mask,
                            PhysxGroupsMask *groups, bool smoothNormal) const {

  NxRaycastHit hit;

  nassertr(_error_type == ET_ok, hit);

  NxGroupsMask *groupsPtr = groups ? &(groups->_mask) : nullptr;

  NxU32 hints = NX_RAYCAST_SHAPE | NX_RAYCAST_IMPACT | NX_RAYCAST_DISTANCE;
  if (smoothNormal == true) {
    hints |= NX_RAYCAST_NORMAL;
  }
  else {
    hints |= NX_RAYCAST_FACE_NORMAL;
  }

  _ptr->raycastClosestShape(ray._ray, (NxShapesType)shapesType, hit,
                            mask.get_mask(), ray._length, hints, groupsPtr);


  return PhysxRaycastHit(hit);
}

/**
 * Returns a PhysxRaycastReport object which can be used to iterate over all
 * shapes that have been hit by the ray.
 */
PhysxRaycastReport PhysxScene::
raycast_all_shapes(const PhysxRay &ray,
                   PhysxShapesType shapesType,
                   PhysxMask mask,
                   PhysxGroupsMask *groups, bool smoothNormal) const {

  PhysxRaycastReport report;

  nassertr(_error_type == ET_ok, report);

  NxGroupsMask *groupsPtr = groups ? &(groups->_mask) : nullptr;

  NxU32 hints = NX_RAYCAST_SHAPE | NX_RAYCAST_IMPACT | NX_RAYCAST_DISTANCE;
  if (smoothNormal == true) {
    hints |= NX_RAYCAST_NORMAL;
  }
  else {
    hints |= NX_RAYCAST_FACE_NORMAL;
  }

  _ptr->raycastAllShapes(ray._ray, report, (NxShapesType)shapesType,
                         mask.get_mask(), ray._length, hints, groupsPtr);

  return report;
}

/**
 * Returns true if any axis aligned bounding box enclosing a shape is
 * intersected by the ray.
 */
bool PhysxScene::
raycast_any_bounds(const PhysxRay &ray,
                         PhysxShapesType shapesType,
                         PhysxMask mask,
                         PhysxGroupsMask *groups) const {

  nassertr(_error_type == ET_ok, false);

  NxGroupsMask *groupsPtr = groups ? &(groups->_mask) : nullptr;

  return _ptr->raycastAnyBounds(ray._ray, (NxShapesType)shapesType,
                                mask.get_mask(), ray._length, groupsPtr);
}

/**
 * Returns the first axis aligned bounding box enclosing a shape that is hit
 * along the ray.  If not shape is hit then an empty raycast hit is returned
 * (is_empty() == true).
 */
PhysxRaycastHit PhysxScene::
raycast_closest_bounds(const PhysxRay &ray, PhysxShapesType shapesType, PhysxMask mask, PhysxGroupsMask *groups, bool smoothNormal) const {

  NxRaycastHit hit;

  nassertr(_error_type == ET_ok, hit);

  NxGroupsMask *groupsPtr = groups ? &(groups->_mask) : nullptr;

  NxU32 hints = NX_RAYCAST_SHAPE | NX_RAYCAST_IMPACT | NX_RAYCAST_DISTANCE;
  if (smoothNormal == true) {
    hints |= NX_RAYCAST_NORMAL;
  }
  else {
    hints |= NX_RAYCAST_FACE_NORMAL;
  }

  _ptr->raycastClosestBounds(ray._ray, (NxShapesType)shapesType, hit,
                             mask.get_mask(), ray._length, hints, groupsPtr);


  return PhysxRaycastHit(hit);
}

/**
 * Returns a PhysxRaycastReport object which can be used to iterate over all
 * shapes that have been enclosed by axis aligned bounding boxes hit by the
 * ray.
 */
PhysxRaycastReport PhysxScene::
raycast_all_bounds(const PhysxRay &ray,
                         PhysxShapesType shapesType,
                         PhysxMask mask,
                         PhysxGroupsMask *groups, bool smoothNormal) const {

  PhysxRaycastReport report;

  nassertr(_error_type == ET_ok, report);

  NxGroupsMask *groupsPtr = groups ? &(groups->_mask) : nullptr;

  NxU32 hints = NX_RAYCAST_SHAPE | NX_RAYCAST_IMPACT | NX_RAYCAST_DISTANCE;
  if (smoothNormal == true) {
    hints |= NX_RAYCAST_NORMAL;
  }
  else {
    hints |= NX_RAYCAST_FACE_NORMAL;
  }

  _ptr->raycastAllBounds(ray._ray, report, (NxShapesType)shapesType,
                         mask.get_mask(), ray._length, hints, groupsPtr);

  return report;
}

/**
 * Returns the set of shapes overlapped by the world-space sphere.  You can
 * test against static and/or dynamic objects by adjusting 'shapeType'.
 */
PhysxOverlapReport PhysxScene::
overlap_sphere_shapes(const LPoint3f &center, float radius,
                      PhysxShapesType shapesType,
                      PhysxMask mask, bool accurateCollision) const {

  PhysxOverlapReport report;

  nassertr(_error_type == ET_ok, report);

  NxSphere worldSphere(PhysxManager::point3_to_nxVec3(center), radius);

  _ptr->overlapSphereShapes(worldSphere, (NxShapesType)shapesType, 0, nullptr, &report,
                            mask.get_mask(), nullptr, accurateCollision);

  return report;
}

/**
 * Returns the set of shapes overlapped by the world-space capsule.  You can
 * test against static and/or dynamic objects by adjusting 'shapeType'.
 */
PhysxOverlapReport PhysxScene::
overlap_capsule_shapes(const LPoint3f &p0, const LPoint3f &p1, float radius,
                       PhysxShapesType shapesType,
                       PhysxMask mask, bool accurateCollision) const {

  PhysxOverlapReport report;

  nassertr(_error_type == ET_ok, report);

  NxSegment segment(PhysxManager::point3_to_nxVec3(p0),
                    PhysxManager::point3_to_nxVec3(p1));
  NxCapsule worldCapsule(segment, radius);

  _ptr->overlapCapsuleShapes(worldCapsule, (NxShapesType)shapesType, 0, nullptr, &report,
                             mask.get_mask(), nullptr, accurateCollision);

  return report;
}

/**
 * Sets the pair flags for the given pair of actors.
 *
 * Calling this on an actor that has no shape(s) has no effect.  The two actor
 * references must not reference the same actor.
 *
 * It is important to note that the engine stores pair flags per shape, even
 * for actor pair flags.  This means that shapes should be created before
 * actor pair flags are set, otherwise the pair flags will be ignored.
 */
void PhysxScene::
set_actor_pair_flag(PhysxActor &actorA, PhysxActor &actorB,
                    PhysxContactPairFlag flag, bool value) {

  nassertv(_error_type == ET_ok);

  NxActor *ptrA = actorA.ptr();
  NxActor *ptrB = actorB.ptr();
  NxU32 flags = _ptr->getActorPairFlags(*ptrA, *ptrB);

  if (value == true) {
    flags |= flag;
  }
  else {
    flags &= ~(flag);
  }

  _ptr->setActorPairFlags(*ptrA, *ptrB, flags);
}

/**
 * Retrieves a single flag for the given pair of actors.
 *
 * The two actor references must not reference the same actor.
 */
bool PhysxScene::
get_actor_pair_flag(PhysxActor &actorA, PhysxActor &actorB,
                    PhysxContactPairFlag flag) {

  nassertr(_error_type == ET_ok, false);

  NxActor *ptrA = actorA.ptr();
  NxActor *ptrB = actorB.ptr();
  NxU32 flags = _ptr->getActorPairFlags(*ptrA, *ptrB);

  return (flags && flag) ? true : false;
}

/**
 * Disables or enables contact generation for a pair of shapes.
 *
 * The two shape references must not reference the same shape.
 */
void PhysxScene::
set_shape_pair_flag(PhysxShape &shapeA, PhysxShape &shapeB, bool value) {

  nassertv(_error_type == ET_ok);

  NxShape *ptrA = shapeA.ptr();
  NxShape *ptrB = shapeB.ptr();
  NxU32 flags = _ptr->getShapePairFlags(*ptrA, *ptrB);

  if (value == true) {
    flags |= NX_IGNORE_PAIR;
  }
  else {
    flags &= ~(NX_IGNORE_PAIR);
  }

  _ptr->setShapePairFlags(*ptrA, *ptrB, flags);
}

/**
 * Returns /true/ if contact generation between a pair of shapes is enabled,
 * and /false/ if contact generation is disables.
 *
 * The two shape references must not reference the same shape.
 */
bool PhysxScene::
get_shape_pair_flag(PhysxShape &shapeA, PhysxShape &shapeB) {

  nassertr(_error_type == ET_ok, false);

  NxShape *ptrA = shapeA.ptr();
  NxShape *ptrB = shapeB.ptr();
  NxU32 flags = _ptr->getShapePairFlags(*ptrA, *ptrB);

  return (flags && NX_IGNORE_PAIR) ? true : false;
}

/**
 * With this method one can set contact reporting flags between actors
 * belonging to a pair of groups.
 *
 * It is possible to assign each actor to a group using
 * PhysxActor::set_group(). This is a different set of groups from the shape
 * groups despite the similar name.  Here up to 0xffff different groups are
 * permitted, With this method one can set contact reporting flags between
 * actors belonging to a pair of groups.
 *
 * The following flags are permitted: - CPF_start_touch - CPF_end_touch -
 * CPF_touch - CPF_start_touch_treshold - CPF_end_touch_treshold -
 * CPF_touch_treshold
 *
 * Note that finer grain control of pairwise flags is possible using the
 * function PhysxScene::set_actor_pair_flags().
 */
void PhysxScene::
set_actor_group_pair_flag(unsigned int g1, unsigned int g2,
                          PhysxContactPairFlag flag, bool value) {

  nassertv(_error_type == ET_ok);

  NxU32 flags = _ptr->getActorGroupPairFlags(g1, g2);
  if (value == true) {
    flags |= flag;
  }
  else {
    flags &= ~(flag);
  }
  _ptr->setActorGroupPairFlags(g1, g2, flags);
}

/**
 * Retrieves a single flag set with PhysxScene::set_actor_group_pair_flag()
 */
bool PhysxScene::
get_actor_group_pair_flag(unsigned int g1, unsigned int g2,
                          PhysxContactPairFlag flag) {

  nassertr(_error_type == ET_ok, false);
  NxU32 flags = _ptr->getActorGroupPairFlags(g1, g2);
  return (flags && flag) ? true : false;
}

/**
 * Setups filtering operations.
 */
void PhysxScene::
set_filter_ops(PhysxFilterOp op0, PhysxFilterOp op1, PhysxFilterOp op2) {

  nassertv(_error_type == ET_ok);
  _ptr->setFilterOps((NxFilterOp)op0, (NxFilterOp)op1, (NxFilterOp)op2);
}

/**
 * Setups filtering's boolean value.
 */
void PhysxScene::
set_filter_bool(bool flag) {

  nassertv(_error_type == ET_ok);
  _ptr->setFilterBool(flag);
}

/**
 * Setups filtering's K0 value.
 */
void PhysxScene::
set_filter_constant0(const PhysxGroupsMask &mask) {

  nassertv(_error_type == ET_ok);
  _ptr->setFilterConstant0(mask.get_mask());
}

/**
 * Setups filtering's K1 value.
 */
void PhysxScene::
set_filter_constant1(const PhysxGroupsMask &mask) {

  nassertv(_error_type == ET_ok);
  _ptr->setFilterConstant1(mask.get_mask());
}

/**
 * Retrieves filtering's boolean value.
 */
bool PhysxScene::
get_filter_bool() const {

  nassertr(_error_type == ET_ok, false);
  return _ptr->getFilterBool();
}

/**
 * Gets filtering constant K0.
 */
PhysxGroupsMask PhysxScene::
get_filter_constant0() const {

  PhysxGroupsMask mask;

  nassertr(_error_type == ET_ok, mask);

  NxGroupsMask _mask = ptr()->getFilterConstant0();
  mask.set_mask(_mask);

  return mask;
}

/**
 * Gets filtering constant K1.
 */
PhysxGroupsMask PhysxScene::
get_filter_constant1() const {

  PhysxGroupsMask mask;

  nassertr(_error_type == ET_ok, mask);

  NxGroupsMask _mask = ptr()->getFilterConstant1();
  mask.set_mask(_mask);

  return mask;
}

/**
 * Retrieves the op0 filtering operation.
 */
PhysxEnums::PhysxFilterOp PhysxScene::
get_filter_op0() const {

  nassertr(_error_type == ET_ok, FO_and);

  NxFilterOp op0;
  NxFilterOp op1;
  NxFilterOp op2;

  _ptr->getFilterOps(op0, op1, op2);

  return (PhysxFilterOp)op0;
}

/**
 * Retrieves the op1 filtering operation.
 */
PhysxEnums::PhysxFilterOp PhysxScene::
get_filter_op1() const {

  nassertr(_error_type == ET_ok, FO_and);

  NxFilterOp op0;
  NxFilterOp op1;
  NxFilterOp op2;

  _ptr->getFilterOps(op0, op1, op2);

  return (PhysxFilterOp)op1;
}

/**
 * Retrieves the op2 filtering operation.
 */
PhysxEnums::PhysxFilterOp PhysxScene::
get_filter_op2() const {

  nassertr(_error_type == ET_ok, FO_and);

  NxFilterOp op0;
  NxFilterOp op1;
  NxFilterOp op2;

  _ptr->getFilterOps(op0, op1, op2);

  return (PhysxFilterOp)op2;
}

/**
 * Specifies if collision should be performed by a pair of shape groups.
 *
 * It is possible to assign each shape to a collision groups using
 * PhysxShape::set_group(). With this method one can set whether collisions
 * should be detected between shapes belonging to a given pair of groups.
 * Initially all pairs are enabled.
 *
 * Fluids can be assigned to collision groups as well.
 *
 * Collision groups are integers between 0 and 31.
 */
void PhysxScene::
set_group_collision_flag(unsigned int g1, unsigned int g2, bool enable) {

  nassertv(_error_type == ET_ok);
  nassertv(g1 >= 0 && g1 < 32);
  nassertv(g2 >= 0 && g2 < 32);

  _ptr->setGroupCollisionFlag((NxCollisionGroup)g1, (NxCollisionGroup)g2, enable);
}

/**
 * Determines if collision detection is performed between a pair of groups.
 * Collision groups are integers between 0 and 31.
 */
bool PhysxScene::
get_group_collision_flag(unsigned int g1, unsigned int g2) {

  nassertr(_error_type == ET_ok, false);
  nassertr(g1 >= 0 && g1 < 32, false);
  nassertr(g2 >= 0 && g2 < 32, false);

  return _ptr->getGroupCollisionFlag((NxCollisionGroup)g1, (NxCollisionGroup)g2);
}

/**
 * Return the specified scene flag flag.
 */
bool PhysxScene::
get_flag(PhysxSceneFlag flag) const {

  nassertr(_error_type == ET_ok, false);
  return (_ptr->getFlags() & flag) ? true : false;
}

/**
 * Returns TRUE if the the scene is simulated in hardware.  FALSE if the scene
 * is simulated in software.
 */
bool PhysxScene::
is_hardware_scene() const {

  nassertr(_error_type == ET_ok, false);
  return (_ptr->getSimType() & NX_SIMULATION_HW) ? true : false;
}

/**
 * Specifies the dominance behavior of constraints between two actors with two
 * certain dominance groups.
 *
 * It is possible to assign each actor to a dominance groups using
 * PhysxActor::set_dominance_group().
 *
 * With dominance groups one can have all constraints (contacts and joints)
 * created between actors act in one direction only.  This is useful if you
 * want to make sure that the movement of the rider of a vehicle or the pony
 * tail of a character doesn't influence the object it is attached to, while
 * keeping the motion of both inherently physical.
 *
 * Whenever a constraint (i.e.  joint or contact) between two actors (a0, a1)
 * needs to be solved, the groups (g0, g1) of both actors are retrieved.  Then
 * the constraint dominance setting for this group pair is retrieved.
 *
 * In the constraint, PhysxConstraintDominance::get_0() becomes the dominance
 * setting for a0, and PhysxConstraintDominance::get_1() becomes the dominance
 * setting for a1. A dominance setting of 1.0f, the default, will permit the
 * actor to be pushed or pulled by the other actor.  A dominance setting of
 * 0.0f will however prevent the actor to be pushed or pulled by the other
 * actor.  Thus, a PhysxConstraintDominance of (1.0f, 0.0f) makes the
 * interaction one-way.
 *
 * The dominance matrix is initialised by default such that: - if g1 == g2,
 * then (1.0f, 1.0f) is returned - if g1 < g2, then (0.0f, 1.0f) is returned -
 * if g1 > g2, then (1.0f, 0.0f) is returned
 *
 * In other words, actors in higher groups can be pushed around by actors in
 * lower groups by default.
 *
 * These settings should cover most applications, and in fact not overriding
 * these settings may likely result in higher performance.
 *
 * Dominance settings are currently specified as floats 0.0f or 1.0f because
 * in the future PhysX may permit arbitrary fractional settings to express
 * 'partly-one-way' interactions.
 */
void PhysxScene::
set_dominance_group_pair(unsigned int g1, unsigned int g2, PhysxConstraintDominance dominance ) {

  nassertv(_error_type == ET_ok);
  nassertv(g1 < 32);
  nassertv(g2 < 32);

  NxConstraintDominance d = dominance.get_dominance();
  _ptr->setDominanceGroupPair((NxDominanceGroup)g1, (NxDominanceGroup)g2, d);
}

/**
 * Samples the dominance matrix.
 */
PhysxConstraintDominance PhysxScene::
get_dominance_group_pair(unsigned int g1, unsigned int g2) {

  PhysxConstraintDominance result(1.0f, 1.0f);

  nassertr(_error_type == ET_ok, result);
  nassertr(g1 < 32, result);
  nassertr(g2 < 32, result);

  result.set_dominance(_ptr->getDominanceGroupPair((NxDominanceGroup)g1, (NxDominanceGroup)g2));
  return result;
}

/**
 * Gets the shared material for all wheel shapes.
 *
 * If this material is not already created then calling this method will
 * create the material.
 *
 * Normally users don't need to call this method.  It is used internally by
 * PhysWheel::create_wheel.
 */
PhysxMaterial *PhysxScene::
get_wheel_shape_material() {

  nassertr(_error_type == ET_ok, nullptr);

  if (_wheelShapeMaterial == nullptr) {
    PhysxMaterialDesc materialDesc;
    materialDesc.set_flag(PhysxMaterialDesc::MF_disable_friction, true);
    _wheelShapeMaterial = create_material(materialDesc);
  }

  return _wheelShapeMaterial;
}
