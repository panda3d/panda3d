#pragma once

class bt32BitAxisSweep3;
class btActionInterface;
class btAxisSweep3;
class btBoxShape;
class btBroadphaseInterface;
class btBroadphaseProxy;
class btBulletWorldImporter;
class btBvhTriangleMeshShape;
class btCapsuleShape;
class btCapsuleShapeX;
class btCapsuleShapeZ;
class btCharacterControllerInterface;
class btCollisionConfiguration;
class btCollisionDispatcher;
class btCollisionObject;
class btCollisionObjectWrapper;
class btCollisionShape;
class btCompoundShape;
class btConcaveShape;
class btConeShape;
class btConeShapeX;
class btConeShapeZ;
class btConeTwistConstraint;
class btConstraintSolver;
class btContactConstraint;
class btContinuousDynamicsWorld;
class btConvexShape;
class btConvexHullShape;
class btConvexInternalShape;
class btConvexPointCloudShape;
class btCylinderShape;
class btCylinderShapeX;
class btCylinderShapeZ;
class btDefaultCollisionConfiguration;
class btDefaultMotionState;
class btDiscreteDynamicsWorld;
class btDispatcher;
class btDynamicsWorld;
class btEmptyShape;
class btGeneric6DofConstraint;
class btGeneric6DofSpringConstraint;
class btGhostPairCallback;
class btGhostObject;
class btGImpactMeshShape;
class btHeightfieldTerrainShape;
class btHingeConstraint;
class btHinge2Constraint;
class btIDebugDraw;
class btKinematicCharacterController;
class btManifoldArray;
class btManifoldPoint;
class btMatrix3x3;
class btMotionState;
class btMinkowskiSumShape;
class btMultiSphereShape;
class btOverlapFilterCallback;
class btPairCachingGhostObject;
class btParalleSequentialImpulseSolver;
class btPersistentManifold;
class btRotationalLimitMotor;
class btPlaneShape;
class btPoint2PointConstraint;
class btPolyhedralConvexShape;
class btQuaternion;
class btSequentialImpulseConstraintSolver;
class btSliderConstraint;
class btSoftBodyHelpers;
class btSoftBodyRigidBodyCollisionConfiguration;
class btSoftBodyCollisionShape;
class btSoftBodyWorldInfo;
class btSoftRigidDynamicsWorld;
class btSphereShape;
class btStaticPlaneShape;
class btStridingMeshInterface;
class btTransform;
class btTranslationalLimitMotor;
class btTriangleIndexVertexArray;
class btTriangleMesh;
class btTypedConstraint;
class btTypedObject;
class btVehicleRaycaster;

template <typename T> class btAlignedObjectArray;

struct btVector3 {};
typedef double btScalar;

class btWheelInfo {
public:
  class RaycastInfo;
};

class btCollisionWorld {
public:
  struct LocalRayResult;
  struct ClosestRayResultCallback;
  struct RayResultCallback;
  struct ContactResultCallback;
  struct AllHitsRayResultCallback;
  struct ClosestConvexResultCallback;
};

class btRaycastVehicle {
public:
  class btVehicleTuning;
};

class btRigidBody {
public:
  class btRigidBodyConstructionInfo;
};

class btSoftBody {
public:
  struct Config;
  struct Joint;
  struct Link;
  struct Material;
  struct Node;
  struct AJoint {
    struct IControl;
  };
};

// BulletCollision/CollisionDispatch/btCollisionObject.h
#define ACTIVE_TAG 1
#define ISLAND_SLEEPING 2
#define WANTS_DEACTIVATION 3
#define DISABLE_DEACTIVATION 4
#define DISABLE_SIMULATION 5

// BulletCollision/CollisionDispatch/btManifoldResult.h
typedef bool (*ContactAddedCallback);
typedef bool (*ContactProcessedCallback);
typedef bool (*ContactDestroyedCallback);

