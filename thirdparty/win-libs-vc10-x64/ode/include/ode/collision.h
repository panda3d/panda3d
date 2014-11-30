/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#ifndef _ODE_COLLISION_H_
#define _ODE_COLLISION_H_

#include <ode/common.h>
#include <ode/collision_space.h>
#include <ode/contact.h>
// Include odeinit.h for backward compatibility as some of initialization APIs 
// were initally declared in current header.
#include <ode/odeinit.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup collide Collision Detection
 *
 * ODE has two main components: a dynamics simulation engine and a collision
 * detection engine. The collision engine is given information about the
 * shape of each body. At each time step it figures out which bodies touch
 * each other and passes the resulting contact point information to the user.
 * The user in turn creates contact joints between bodies.
 *
 * Using ODE's collision detection is optional - an alternative collision
 * detection system can be used as long as it can supply the right kinds of
 * contact information.
 */


/* ************************************************************************ */
/* general functions */

/**
 * @brief Destroy a geom, removing it from any space.
 *
 * Destroy a geom, removing it from any space it is in first. This one
 * function destroys a geom of any type, but to create a geom you must call
 * a creation function for that type.
 *
 * When a space is destroyed, if its cleanup mode is 1 (the default) then all
 * the geoms in that space are automatically destroyed as well.
 *
 * @param geom the geom to be destroyed.
 * @ingroup collide
 */
ODE_API void dGeomDestroy (dGeomID geom);


/**
 * @brief Set the user-defined data pointer stored in the geom.
 *
 * @param geom the geom to hold the data
 * @param data the data pointer to be stored
 * @ingroup collide
 */
ODE_API void dGeomSetData (dGeomID geom, void* data);


/**
 * @brief Get the user-defined data pointer stored in the geom.
 *
 * @param geom the geom containing the data
 * @ingroup collide
 */
ODE_API void *dGeomGetData (dGeomID geom);


/**
 * @brief Set the body associated with a placeable geom.
 *
 * Setting a body on a geom automatically combines the position vector and
 * rotation matrix of the body and geom, so that setting the position or
 * orientation of one will set the value for both objects. Setting a body
 * ID of zero gives the geom its own position and rotation, independent
 * from any body. If the geom was previously connected to a body then its
 * new independent position/rotation is set to the current position/rotation
 * of the body.
 *
 * Calling these functions on a non-placeable geom results in a runtime
 * error in the debug build of ODE.
 *
 * @param geom the geom to connect
 * @param body the body to attach to the geom
 * @ingroup collide
 */
ODE_API void dGeomSetBody (dGeomID geom, dBodyID body);


/**
 * @brief Get the body associated with a placeable geom.
 * @param geom the geom to query.
 * @sa dGeomSetBody
 * @ingroup collide
 */
ODE_API dBodyID dGeomGetBody (dGeomID geom);


/**
 * @brief Set the position vector of a placeable geom.
 *
 * If the geom is attached to a body, the body's position will also be changed.
 * Calling this function on a non-placeable geom results in a runtime error in
 * the debug build of ODE.
 *
 * @param geom the geom to set.
 * @param x the new X coordinate.
 * @param y the new Y coordinate.
 * @param z the new Z coordinate.
 * @sa dBodySetPosition
 * @ingroup collide
 */
ODE_API void dGeomSetPosition (dGeomID geom, dReal x, dReal y, dReal z);


/**
 * @brief Set the rotation matrix of a placeable geom.
 *
 * If the geom is attached to a body, the body's rotation will also be changed.
 * Calling this function on a non-placeable geom results in a runtime error in
 * the debug build of ODE.
 *
 * @param geom the geom to set.
 * @param R the new rotation matrix.
 * @sa dBodySetRotation
 * @ingroup collide
 */
ODE_API void dGeomSetRotation (dGeomID geom, const dMatrix3 R);


/**
 * @brief Set the rotation of a placeable geom.
 *
 * If the geom is attached to a body, the body's rotation will also be changed.
 *
 * Calling this function on a non-placeable geom results in a runtime error in
 * the debug build of ODE.
 *
 * @param geom the geom to set.
 * @param Q the new rotation.
 * @sa dBodySetQuaternion
 * @ingroup collide
 */
ODE_API void dGeomSetQuaternion (dGeomID geom, const dQuaternion Q);


/**
 * @brief Get the position vector of a placeable geom.
 *
 * If the geom is attached to a body, the body's position will be returned.
 *
 * Calling this function on a non-placeable geom results in a runtime error in
 * the debug build of ODE.
 *
 * @param geom the geom to query.
 * @returns A pointer to the geom's position vector.
 * @remarks The returned value is a pointer to the geom's internal
 *          data structure. It is valid until any changes are made
 *          to the geom.
 * @sa dBodyGetPosition
 * @ingroup collide
 */
ODE_API const dReal * dGeomGetPosition (dGeomID geom);


/**
 * @brief Copy the position of a geom into a vector.
 * @ingroup collide
 * @param geom  the geom to query
 * @param pos   a copy of the geom position
 * @sa dGeomGetPosition
 */
ODE_API void dGeomCopyPosition (dGeomID geom, dVector3 pos);


/**
 * @brief Get the rotation matrix of a placeable geom.
 *
 * If the geom is attached to a body, the body's rotation will be returned.
 *
 * Calling this function on a non-placeable geom results in a runtime error in
 * the debug build of ODE.
 *
 * @param geom the geom to query.
 * @returns A pointer to the geom's rotation matrix.
 * @remarks The returned value is a pointer to the geom's internal
 *          data structure. It is valid until any changes are made
 *          to the geom.
 * @sa dBodyGetRotation
 * @ingroup collide
 */
ODE_API const dReal * dGeomGetRotation (dGeomID geom);


/**
 * @brief Get the rotation matrix of a placeable geom.
 *
 * If the geom is attached to a body, the body's rotation will be returned.
 *
 * Calling this function on a non-placeable geom results in a runtime error in
 * the debug build of ODE.
 *
 * @param geom   the geom to query.
 * @param R      a copy of the geom rotation
 * @sa dGeomGetRotation
 * @ingroup collide
 */
ODE_API void dGeomCopyRotation(dGeomID geom, dMatrix3 R);


/**
 * @brief Get the rotation quaternion of a placeable geom.
 *
 * If the geom is attached to a body, the body's quaternion will be returned.
 *
 * Calling this function on a non-placeable geom results in a runtime error in
 * the debug build of ODE.
 *
 * @param geom the geom to query.
 * @param result a copy of the rotation quaternion.
 * @sa dBodyGetQuaternion
 * @ingroup collide
 */
ODE_API void dGeomGetQuaternion (dGeomID geom, dQuaternion result);


/**
 * @brief Return the axis-aligned bounding box.
 *
 * Return in aabb an axis aligned bounding box that surrounds the given geom.
 * The aabb array has elements (minx, maxx, miny, maxy, minz, maxz). If the
 * geom is a space, a bounding box that surrounds all contained geoms is
 * returned.
 *
 * This function may return a pre-computed cached bounding box, if it can
 * determine that the geom has not moved since the last time the bounding
 * box was computed.
 *
 * @param geom the geom to query
 * @param aabb the returned bounding box
 * @ingroup collide
 */
ODE_API void dGeomGetAABB (dGeomID geom, dReal aabb[6]);


/**
 * @brief Determing if a geom is a space.
 * @param geom the geom to query
 * @returns Non-zero if the geom is a space, zero otherwise.
 * @ingroup collide
 */
ODE_API int dGeomIsSpace (dGeomID geom);


/**
 * @brief Query for the space containing a particular geom.
 * @param geom the geom to query
 * @returns The space that contains the geom, or NULL if the geom is
 *          not contained by a space.
 * @ingroup collide
 */
ODE_API dSpaceID dGeomGetSpace (dGeomID);


/**
 * @brief Given a geom, this returns its class.
 *
 * The ODE classes are:
 *  @li dSphereClass
 *  @li dBoxClass
 *  @li dCylinderClass
 *  @li dPlaneClass
 *  @li dRayClass
 *  @li dConvexClass
 *  @li dGeomTransformClass
 *  @li dTriMeshClass
 *  @li dSimpleSpaceClass
 *  @li dHashSpaceClass
 *  @li dQuadTreeSpaceClass
 *  @li dFirstUserClass
 *  @li dLastUserClass
 *
 * User-defined class will return their own number.
 *
 * @param geom the geom to query
 * @returns The geom class ID.
 * @ingroup collide
 */
ODE_API int dGeomGetClass (dGeomID geom);


/**
 * @brief Set the "category" bitfield for the given geom.
 *
 * The category bitfield is used by spaces to govern which geoms will
 * interact with each other. The bitfield is guaranteed to be at least
 * 32 bits wide. The default category values for newly created geoms
 * have all bits set.
 *
 * @param geom the geom to set
 * @param bits the new bitfield value
 * @ingroup collide
 */
ODE_API void dGeomSetCategoryBits (dGeomID geom, unsigned long bits);


/**
 * @brief Set the "collide" bitfield for the given geom.
 *
 * The collide bitfield is used by spaces to govern which geoms will
 * interact with each other. The bitfield is guaranteed to be at least
 * 32 bits wide. The default category values for newly created geoms
 * have all bits set.
 *
 * @param geom the geom to set
 * @param bits the new bitfield value
 * @ingroup collide
 */
ODE_API void dGeomSetCollideBits (dGeomID geom, unsigned long bits);


/**
 * @brief Get the "category" bitfield for the given geom.
 *
 * @param geom the geom to set
 * @param bits the new bitfield value
 * @sa dGeomSetCategoryBits
 * @ingroup collide
 */
ODE_API unsigned long dGeomGetCategoryBits (dGeomID);


/**
 * @brief Get the "collide" bitfield for the given geom.
 *
 * @param geom the geom to set
 * @param bits the new bitfield value
 * @sa dGeomSetCollideBits
 * @ingroup collide
 */
ODE_API unsigned long dGeomGetCollideBits (dGeomID);


/**
 * @brief Enable a geom.
 *
 * Disabled geoms are completely ignored by dSpaceCollide and dSpaceCollide2,
 * although they can still be members of a space. New geoms are created in
 * the enabled state.
 *
 * @param geom   the geom to enable
 * @sa dGeomDisable
 * @sa dGeomIsEnabled
 * @ingroup collide
 */
ODE_API void dGeomEnable (dGeomID geom);


/**
 * @brief Disable a geom.
 *
 * Disabled geoms are completely ignored by dSpaceCollide and dSpaceCollide2,
 * although they can still be members of a space. New geoms are created in
 * the enabled state.
 *
 * @param geom   the geom to disable
 * @sa dGeomDisable
 * @sa dGeomIsEnabled
 * @ingroup collide
 */
ODE_API void dGeomDisable (dGeomID geom);


/**
 * @brief Check to see if a geom is enabled.
 *
 * Disabled geoms are completely ignored by dSpaceCollide and dSpaceCollide2,
 * although they can still be members of a space. New geoms are created in
 * the enabled state.
 *
 * @param geom   the geom to query
 * @returns Non-zero if the geom is enabled, zero otherwise.
 * @sa dGeomDisable
 * @sa dGeomIsEnabled
 * @ingroup collide
 */
ODE_API int dGeomIsEnabled (dGeomID geom);


enum
{
	dGeomCommonControlClass = 0,
	dGeomColliderControlClass = 1
};

enum
{
	dGeomCommonAnyControlCode = 0,

	dGeomColliderSetMergeSphereContactsControlCode = 1,
	dGeomColliderGetMergeSphereContactsControlCode = 2
};

enum
{
	dGeomColliderMergeContactsValue__Default = 0, // Used with Set... to restore default value
	dGeomColliderMergeContactsValue_None = 1,
	dGeomColliderMergeContactsValue_Normals = 2,
	dGeomColliderMergeContactsValue_Full = 3
};

/**
 * @brief Execute low level control operation for geometry.
 *
 * The variable the dataSize points to must be initialized before the call.
 * If the size does not match the one expected for the control class/code function
 * changes it to the size expected and returns failure. This implies the function 
 * can be called with NULL data and zero size to test if control class/code is supported
 * and obtain required data size for it.
 *
 * dGeomCommonAnyControlCode applies to any control class and returns success if 
 * at least one control code is available for the given class with given geom.
 *
 * Currently there are the folliwing control classes supported:
 *  @li dGeomColliderControlClass
 *
 * For dGeomColliderControlClass there are the following codes available:
 *  @li dGeomColliderSetMergeSphereContactsControlCode (arg of type int, dGeomColliderMergeContactsValue_*)
 *  @li dGeomColliderGetMergeSphereContactsControlCode (arg of type int, dGeomColliderMergeContactsValue_*)
 *
 * @param geom   the geom to control
 * @param controlClass   the control class
 * @param controlCode   the control code for the class
 * @param dataValue   the control argument pointer
 * @param dataSize   the control argument size provided or expected
 * @returns Boolean execution status
 * @ingroup collide
 */
ODE_API int dGeomLowLevelControl (dGeomID geom, int controlClass, int controlCode, void *dataValue, int *dataSize);


/**
 * @brief Get world position of a relative point on geom.
 *
 * Calling this function on a non-placeable geom results in the same point being
 * returned.
 *
 * @ingroup collide
 * @param result will contain the result.
 */
ODE_API void dGeomGetRelPointPos
(
  dGeomID geom, dReal px, dReal py, dReal pz,
  dVector3 result
);

/**
 * @brief takes a point in global coordinates and returns
 * the point's position in geom-relative coordinates.
 *
 * Calling this function on a non-placeable geom results in the same point being
 * returned.
 *
 * @remarks
 * This is the inverse of dGeomGetRelPointPos()
 * @ingroup collide
 * @param result will contain the result.
 */
ODE_API void dGeomGetPosRelPoint
(
  dGeomID geom, dReal px, dReal py, dReal pz,
  dVector3 result
);

/**
 * @brief Convert from geom-local to world coordinates.
 *
 * Calling this function on a non-placeable geom results in the same vector being
 * returned.
 *
 * @ingroup collide
 * @param result will contain the result.
 */
ODE_API void dGeomVectorToWorld
(
  dGeomID geom, dReal px, dReal py, dReal pz,
  dVector3 result
);

/**
 * @brief Convert from world to geom-local coordinates.
 *
 * Calling this function on a non-placeable geom results in the same vector being
 * returned.
 *
 * @ingroup collide
 * @param result will contain the result.
 */
ODE_API void dGeomVectorFromWorld
(
  dGeomID geom, dReal px, dReal py, dReal pz,
  dVector3 result
);


/* ************************************************************************ */
/* geom offset from body */

/**
 * @brief Set the local offset position of a geom from its body.
 *
 * Sets the geom's positional offset in local coordinates.
 * After this call, the geom will be at a new position determined from the
 * body's position and the offset.
 * The geom must be attached to a body.
 * If the geom did not have an offset, it is automatically created.
 *
 * @param geom the geom to set.
 * @param x the new X coordinate.
 * @param y the new Y coordinate.
 * @param z the new Z coordinate.
 * @ingroup collide
 */
ODE_API void dGeomSetOffsetPosition (dGeomID geom, dReal x, dReal y, dReal z);


/**
 * @brief Set the local offset rotation matrix of a geom from its body.
 *
 * Sets the geom's rotational offset in local coordinates.
 * After this call, the geom will be at a new position determined from the
 * body's position and the offset.
 * The geom must be attached to a body.
 * If the geom did not have an offset, it is automatically created.
 *
 * @param geom the geom to set.
 * @param R the new rotation matrix.
 * @ingroup collide
 */
ODE_API void dGeomSetOffsetRotation (dGeomID geom, const dMatrix3 R);


/**
 * @brief Set the local offset rotation of a geom from its body.
 *
 * Sets the geom's rotational offset in local coordinates.
 * After this call, the geom will be at a new position determined from the
 * body's position and the offset.
 * The geom must be attached to a body.
 * If the geom did not have an offset, it is automatically created.
 *
 * @param geom the geom to set.
 * @param Q the new rotation.
 * @ingroup collide
 */
ODE_API void dGeomSetOffsetQuaternion (dGeomID geom, const dQuaternion Q);


/**
 * @brief Set the offset position of a geom from its body.
 *
 * Sets the geom's positional offset to move it to the new world
 * coordinates.
 * After this call, the geom will be at the world position passed in,
 * and the offset will be the difference from the current body position.
 * The geom must be attached to a body.
 * If the geom did not have an offset, it is automatically created.
 *
 * @param geom the geom to set.
 * @param x the new X coordinate.
 * @param y the new Y coordinate.
 * @param z the new Z coordinate.
 * @ingroup collide
 */
ODE_API void dGeomSetOffsetWorldPosition (dGeomID geom, dReal x, dReal y, dReal z);


/**
 * @brief Set the offset rotation of a geom from its body.
 *
 * Sets the geom's rotational offset to orient it to the new world
 * rotation matrix.
 * After this call, the geom will be at the world orientation passed in,
 * and the offset will be the difference from the current body orientation.
 * The geom must be attached to a body.
 * If the geom did not have an offset, it is automatically created.
 *
 * @param geom the geom to set.
 * @param R the new rotation matrix.
 * @ingroup collide
 */
ODE_API void dGeomSetOffsetWorldRotation (dGeomID geom, const dMatrix3 R);


/**
 * @brief Set the offset rotation of a geom from its body.
 *
 * Sets the geom's rotational offset to orient it to the new world
 * rotation matrix.
 * After this call, the geom will be at the world orientation passed in,
 * and the offset will be the difference from the current body orientation.
 * The geom must be attached to a body.
 * If the geom did not have an offset, it is automatically created.
 *
 * @param geom the geom to set.
 * @param Q the new rotation.
 * @ingroup collide
 */
ODE_API void dGeomSetOffsetWorldQuaternion (dGeomID geom, const dQuaternion);


/**
 * @brief Clear any offset from the geom.
 *
 * If the geom has an offset, it is eliminated and the geom is
 * repositioned at the body's position.  If the geom has no offset,
 * this function does nothing.
 * This is more efficient than calling dGeomSetOffsetPosition(zero)
 * and dGeomSetOffsetRotation(identiy), because this function actually
 * eliminates the offset, rather than leaving it as the identity transform.
 *
 * @param geom the geom to have its offset destroyed.
 * @ingroup collide
 */
ODE_API void dGeomClearOffset(dGeomID geom);


/**
 * @brief Check to see whether the geom has an offset.
 *
 * This function will return non-zero if the offset has been created.
 * Note that there is a difference between a geom with no offset,
 * and a geom with an offset that is the identity transform.
 * In the latter case, although the observed behaviour is identical,
 * there is a unnecessary computation involved because the geom will
 * be applying the transform whenever it needs to recalculate its world
 * position.
 *
 * @param geom the geom to query.
 * @returns Non-zero if the geom has an offset, zero otherwise.
 * @ingroup collide
 */
ODE_API int dGeomIsOffset(dGeomID geom);


/**
 * @brief Get the offset position vector of a geom.
 *
 * Returns the positional offset of the geom in local coordinates.
 * If the geom has no offset, this function returns the zero vector.
 *
 * @param geom the geom to query.
 * @returns A pointer to the geom's offset vector.
 * @remarks The returned value is a pointer to the geom's internal
 *          data structure. It is valid until any changes are made
 *          to the geom.
 * @ingroup collide
 */
ODE_API const dReal * dGeomGetOffsetPosition (dGeomID geom);


/**
 * @brief Copy the offset position vector of a geom.
 *
 * Returns the positional offset of the geom in local coordinates.
 * If the geom has no offset, this function returns the zero vector.
 *
 * @param geom   the geom to query.
 * @param pos    returns the offset position
 * @ingroup collide
 */
ODE_API void dGeomCopyOffsetPosition (dGeomID geom, dVector3 pos);


/**
 * @brief Get the offset rotation matrix of a geom.
 *
 * Returns the rotational offset of the geom in local coordinates.
 * If the geom has no offset, this function returns the identity
 * matrix.
 *
 * @param geom the geom to query.
 * @returns A pointer to the geom's offset rotation matrix.
 * @remarks The returned value is a pointer to the geom's internal
 *          data structure. It is valid until any changes are made
 *          to the geom.
 * @ingroup collide
 */
ODE_API const dReal * dGeomGetOffsetRotation (dGeomID geom);


/**
 * @brief Copy the offset rotation matrix of a geom.
 *
 * Returns the rotational offset of the geom in local coordinates.
 * If the geom has no offset, this function returns the identity
 * matrix.
 *
 * @param geom   the geom to query.
 * @param R      returns the rotation matrix.
 * @ingroup collide
 */
ODE_API void dGeomCopyOffsetRotation (dGeomID geom, dMatrix3 R);


/**
 * @brief Get the offset rotation quaternion of a geom.
 *
 * Returns the rotation offset of the geom as a quaternion.
 * If the geom has no offset, the identity quaternion is returned.
 *
 * @param geom the geom to query.
 * @param result a copy of the rotation quaternion.
 * @ingroup collide
 */
ODE_API void dGeomGetOffsetQuaternion (dGeomID geom, dQuaternion result);


/* ************************************************************************ */
/* collision detection */

/*
 *	Just generate any contacts (disables any contact refining).
 */
#define CONTACTS_UNIMPORTANT			0x80000000

/**
 *
 * @brief Given two geoms o1 and o2 that potentially intersect,
 * generate contact information for them.
 *
 * Internally, this just calls the correct class-specific collision
 * functions for o1 and o2.
 *
 * @param o1 The first geom to test.
 * @param o2 The second geom to test.
 *
 * @param flags The flags specify how contacts should be generated if
 * the geoms touch. The lower 16 bits of flags is an integer that
 * specifies the maximum number of contact points to generate. You must
 * ask for at least one contact. 
 * Additionally, following bits may be set:
 * CONTACTS_UNIMPORTANT -- just generate any contacts (skip contact refining).
 * All other bits in flags must be set to zero. In the future the other bits 
 * may be used to select from different contact generation strategies.
 *
 * @param contact Points to an array of dContactGeom structures. The array
 * must be able to hold at least the maximum number of contacts. These
 * dContactGeom structures may be embedded within larger structures in the
 * array -- the skip parameter is the byte offset from one dContactGeom to
 * the next in the array. If skip is sizeof(dContactGeom) then contact
 * points to a normal (C-style) array. It is an error for skip to be smaller
 * than sizeof(dContactGeom).
 *
 * @returns If the geoms intersect, this function returns the number of contact
 * points generated (and updates the contact array), otherwise it returns 0
 * (and the contact array is not touched).
 *
 * @remarks If a space is passed as o1 or o2 then this function will collide
 * all objects contained in o1 with all objects contained in o2, and return
 * the resulting contact points. This method for colliding spaces with geoms
 * (or spaces with spaces) provides no user control over the individual
 * collisions. To get that control, use dSpaceCollide or dSpaceCollide2 instead.
 *
 * @remarks If o1 and o2 are the same geom then this function will do nothing
 * and return 0. Technically speaking an object intersects with itself, but it
 * is not useful to find contact points in this case.
 *
 * @remarks This function does not care if o1 and o2 are in the same space or not
 * (or indeed if they are in any space at all).
 *
 * @ingroup collide
 */
ODE_API int dCollide (dGeomID o1, dGeomID o2, int flags, dContactGeom *contact,
	      int skip);

/**
 * @brief Determines which pairs of geoms in a space may potentially intersect,
 * and calls the callback function for each candidate pair.
 *
 * @param space The space to test.
 *
 * @param data Passed from dSpaceCollide directly to the callback
 * function. Its meaning is user defined. The o1 and o2 arguments are the
 * geoms that may be near each other.
 *
 * @param callback A callback function is of type @ref dNearCallback.
 *
 * @remarks Other spaces that are contained within the colliding space are
 * not treated specially, i.e. they are not recursed into. The callback
 * function may be passed these contained spaces as one or both geom
 * arguments.
 *
 * @remarks dSpaceCollide() is guaranteed to pass all intersecting geom
 * pairs to the callback function, but may also pass close but
 * non-intersecting pairs. The number of these calls depends on the
 * internal algorithms used by the space. Thus you should not expect
 * that dCollide will return contacts for every pair passed to the
 * callback.
 *
 * @sa dSpaceCollide2
 * @ingroup collide
 */
ODE_API void dSpaceCollide (dSpaceID space, void *data, dNearCallback *callback);


/**
 * @brief Determines which geoms from one space may potentially intersect with 
 * geoms from another space, and calls the callback function for each candidate 
 * pair. 
 *
 * @param space1 The first space to test.
 *
 * @param space2 The second space to test.
 *
 * @param data Passed from dSpaceCollide directly to the callback
 * function. Its meaning is user defined. The o1 and o2 arguments are the
 * geoms that may be near each other.
 *
 * @param callback A callback function is of type @ref dNearCallback.
 *
 * @remarks This function can also test a single non-space geom against a 
 * space. This function is useful when there is a collision hierarchy, i.e. 
 * when there are spaces that contain other spaces.
 *
 * @remarks Other spaces that are contained within the colliding space are
 * not treated specially, i.e. they are not recursed into. The callback
 * function may be passed these contained spaces as one or both geom
 * arguments.
 *
 * @remarks Sublevel value of space affects how the spaces are iterated.
 * Both spaces are recursed only if their sublevels match. Otherwise, only
 * the space with greater sublevel is recursed and the one with lesser sublevel
 * is used as a geom itself.
 *
 * @remarks dSpaceCollide2() is guaranteed to pass all intersecting geom
 * pairs to the callback function, but may also pass close but
 * non-intersecting pairs. The number of these calls depends on the
 * internal algorithms used by the space. Thus you should not expect
 * that dCollide will return contacts for every pair passed to the
 * callback.
 *
 * @sa dSpaceCollide
 * @sa dSpaceSetSublevel
 * @ingroup collide
 */
ODE_API void dSpaceCollide2 (dGeomID space1, dGeomID space2, void *data, dNearCallback *callback);


/* ************************************************************************ */
/* standard classes */

/* the maximum number of user classes that are supported */
enum {
  dMaxUserClasses = 4
};

/* class numbers - each geometry object needs a unique number */
enum {
  dSphereClass = 0,
  dBoxClass,
  dCapsuleClass,
  dCylinderClass,
  dPlaneClass,
  dRayClass,
  dConvexClass,
  dGeomTransformClass,
  dTriMeshClass,
  dHeightfieldClass,

  dFirstSpaceClass,
  dSimpleSpaceClass = dFirstSpaceClass,
  dHashSpaceClass,
  dSweepAndPruneSpaceClass, // SAP
  dQuadTreeSpaceClass,
  dLastSpaceClass = dQuadTreeSpaceClass,

  dFirstUserClass,
  dLastUserClass = dFirstUserClass + dMaxUserClasses - 1,
  dGeomNumClasses
};


/**
 * @defgroup collide_sphere Sphere Class
 * @ingroup collide
 */

/**
 * @brief Create a sphere geom of the given radius, and return its ID. 
 *
 * @param space   a space to contain the new geom. May be null.
 * @param radius  the radius of the sphere.
 *
 * @returns A new sphere geom.
 *
 * @remarks The point of reference for a sphere is its center.
 *
 * @sa dGeomDestroy
 * @sa dGeomSphereSetRadius
 * @ingroup collide_sphere
 */
ODE_API dGeomID dCreateSphere (dSpaceID space, dReal radius);


/**
 * @brief Set the radius of a sphere geom.
 *
 * @param sphere  the sphere to set.
 * @param radius  the new radius.
 *
 * @sa dGeomSphereGetRadius
 * @ingroup collide_sphere
 */
ODE_API void dGeomSphereSetRadius (dGeomID sphere, dReal radius);


/**
 * @brief Retrieves the radius of a sphere geom.
 *
 * @param sphere  the sphere to query.
 *
 * @sa dGeomSphereSetRadius
 * @ingroup collide_sphere
 */
ODE_API dReal dGeomSphereGetRadius (dGeomID sphere);


/**
 * @brief Calculate the depth of the a given point within a sphere.
 *
 * @param sphere  the sphere to query.
 * @param x       the X coordinate of the point.
 * @param y       the Y coordinate of the point.
 * @param z       the Z coordinate of the point.
 *
 * @returns The depth of the point. Points inside the sphere will have a 
 * positive depth, points outside it will have a negative depth, and points
 * on the surface will have a depth of zero.
 *
 * @ingroup collide_sphere
 */
ODE_API dReal dGeomSpherePointDepth (dGeomID sphere, dReal x, dReal y, dReal z);


//--> Convex Functions
ODE_API dGeomID dCreateConvex (dSpaceID space,
			       dReal *_planes,
			       unsigned int _planecount,
			       dReal *_points,
			       unsigned int _pointcount,unsigned int *_polygons);

ODE_API void dGeomSetConvex (dGeomID g,
			     dReal *_planes,
			     unsigned int _count,
			     dReal *_points,
			     unsigned int _pointcount,unsigned int *_polygons);
//<-- Convex Functions

/**
 * @defgroup collide_box Box Class
 * @ingroup collide
 */

/**
 * @brief Create a box geom with the provided side lengths.
 *
 * @param space   a space to contain the new geom. May be null.
 * @param lx      the length of the box along the X axis
 * @param ly      the length of the box along the Y axis
 * @param lz      the length of the box along the Z axis
 *
 * @returns A new box geom.
 *
 * @remarks The point of reference for a box is its center.
 *
 * @sa dGeomDestroy
 * @sa dGeomBoxSetLengths
 * @ingroup collide_box
 */
ODE_API dGeomID dCreateBox (dSpaceID space, dReal lx, dReal ly, dReal lz);


/**
 * @brief Set the side lengths of the given box.
 *
 * @param box  the box to set
 * @param lx      the length of the box along the X axis
 * @param ly      the length of the box along the Y axis
 * @param lz      the length of the box along the Z axis
 *
 * @sa dGeomBoxGetLengths
 * @ingroup collide_box
 */
ODE_API void dGeomBoxSetLengths (dGeomID box, dReal lx, dReal ly, dReal lz);


/**
 * @brief Get the side lengths of a box.
 *
 * @param box     the box to query
 * @param result  the returned side lengths
 *
 * @sa dGeomBoxSetLengths
 * @ingroup collide_box
 */
ODE_API void dGeomBoxGetLengths (dGeomID box, dVector3 result);


/**
 * @brief Return the depth of a point in a box.
 * 
 * @param box  the box to query
 * @param x    the X coordinate of the point to test.
 * @param y    the Y coordinate of the point to test.
 * @param z    the Z coordinate of the point to test.
 *
 * @returns The depth of the point. Points inside the box will have a 
 * positive depth, points outside it will have a negative depth, and points
 * on the surface will have a depth of zero.
 */
ODE_API dReal dGeomBoxPointDepth (dGeomID box, dReal x, dReal y, dReal z);


ODE_API dGeomID dCreatePlane (dSpaceID space, dReal a, dReal b, dReal c, dReal d);
ODE_API void dGeomPlaneSetParams (dGeomID plane, dReal a, dReal b, dReal c, dReal d);
ODE_API void dGeomPlaneGetParams (dGeomID plane, dVector4 result);
ODE_API dReal dGeomPlanePointDepth (dGeomID plane, dReal x, dReal y, dReal z);

ODE_API dGeomID dCreateCapsule (dSpaceID space, dReal radius, dReal length);
ODE_API void dGeomCapsuleSetParams (dGeomID ccylinder, dReal radius, dReal length);
ODE_API void dGeomCapsuleGetParams (dGeomID ccylinder, dReal *radius, dReal *length);
ODE_API dReal dGeomCapsulePointDepth (dGeomID ccylinder, dReal x, dReal y, dReal z);

// For now we want to have a backwards compatible C-API, note: C++ API is not.
#define dCreateCCylinder dCreateCapsule
#define dGeomCCylinderSetParams dGeomCapsuleSetParams
#define dGeomCCylinderGetParams dGeomCapsuleGetParams
#define dGeomCCylinderPointDepth dGeomCapsulePointDepth
#define dCCylinderClass dCapsuleClass

ODE_API dGeomID dCreateCylinder (dSpaceID space, dReal radius, dReal length);
ODE_API void dGeomCylinderSetParams (dGeomID cylinder, dReal radius, dReal length);
ODE_API void dGeomCylinderGetParams (dGeomID cylinder, dReal *radius, dReal *length);

ODE_API dGeomID dCreateRay (dSpaceID space, dReal length);
ODE_API void dGeomRaySetLength (dGeomID ray, dReal length);
ODE_API dReal dGeomRayGetLength (dGeomID ray);
ODE_API void dGeomRaySet (dGeomID ray, dReal px, dReal py, dReal pz,
		  dReal dx, dReal dy, dReal dz);
ODE_API void dGeomRayGet (dGeomID ray, dVector3 start, dVector3 dir);

/*
 * Set/get ray flags that influence ray collision detection.
 * These flags are currently only noticed by the trimesh collider, because
 * they can make a major differences there.
 */
ODE_API void dGeomRaySetParams (dGeomID g, int FirstContact, int BackfaceCull);
ODE_API void dGeomRayGetParams (dGeomID g, int *FirstContact, int *BackfaceCull);
ODE_API void dGeomRaySetClosestHit (dGeomID g, int closestHit);
ODE_API int dGeomRayGetClosestHit (dGeomID g);

#include "collision_trimesh.h"

ODE_API dGeomID dCreateGeomTransform (dSpaceID space);
ODE_API void dGeomTransformSetGeom (dGeomID g, dGeomID obj);
ODE_API dGeomID dGeomTransformGetGeom (dGeomID g);
ODE_API void dGeomTransformSetCleanup (dGeomID g, int mode);
ODE_API int dGeomTransformGetCleanup (dGeomID g);
ODE_API void dGeomTransformSetInfo (dGeomID g, int mode);
ODE_API int dGeomTransformGetInfo (dGeomID g);


/* ************************************************************************ */
/* heightfield functions */


// Data storage for heightfield data.
struct dxHeightfieldData;
typedef struct dxHeightfieldData* dHeightfieldDataID;


/**
 * @brief Callback prototype
 *
 * Used by the callback heightfield data type to sample a height for a
 * given cell position.
 *
 * @param p_user_data User data specified when creating the dHeightfieldDataID
 * @param x The index of a sample in the local x axis. It is a value
 * in the range zero to ( nWidthSamples - 1 ).
 * @param x The index of a sample in the local z axis. It is a value
 * in the range zero to ( nDepthSamples - 1 ).
 *
 * @return The sample height which is then scaled and offset using the
 * values specified when the heightfield data was created.
 *
 * @ingroup collide
 */
typedef dReal dHeightfieldGetHeight( void* p_user_data, int x, int z );



/**
 * @brief Creates a heightfield geom.
 *
 * Uses the information in the given dHeightfieldDataID to construct
 * a geom representing a heightfield in a collision space.
 *
 * @param space The space to add the geom to.
 * @param data The dHeightfieldDataID created by dGeomHeightfieldDataCreate and
 * setup by dGeomHeightfieldDataBuildCallback, dGeomHeightfieldDataBuildByte,
 * dGeomHeightfieldDataBuildShort or dGeomHeightfieldDataBuildFloat.
 * @param bPlaceable If non-zero this geom can be transformed in the world using the
 * usual functions such as dGeomSetPosition and dGeomSetRotation. If the geom is
 * not set as placeable, then it uses a fixed orientation where the global y axis
 * represents the dynamic 'height' of the heightfield.
 *
 * @return A geom id to reference this geom in other calls.
 *
 * @ingroup collide
 */
ODE_API dGeomID dCreateHeightfield( dSpaceID space,
					dHeightfieldDataID data, int bPlaceable );


/**
 * @brief Creates a new empty dHeightfieldDataID.
 *
 * Allocates a new dHeightfieldDataID and returns it. You must call
 * dGeomHeightfieldDataDestroy to destroy it after the geom has been removed.
 * The dHeightfieldDataID value is used when specifying a data format type.
 *
 * @return A dHeightfieldDataID for use with dGeomHeightfieldDataBuildCallback,
 * dGeomHeightfieldDataBuildByte, dGeomHeightfieldDataBuildShort or
 * dGeomHeightfieldDataBuildFloat.
 * @ingroup collide
 */
ODE_API dHeightfieldDataID dGeomHeightfieldDataCreate(void);


/**
 * @brief Destroys a dHeightfieldDataID.
 *
 * Deallocates a given dHeightfieldDataID and all managed resources.
 *
 * @param d A dHeightfieldDataID created by dGeomHeightfieldDataCreate
 * @ingroup collide
 */
ODE_API void dGeomHeightfieldDataDestroy( dHeightfieldDataID d );



/**
 * @brief Configures a dHeightfieldDataID to use a callback to
 * retrieve height data.
 *
 * Before a dHeightfieldDataID can be used by a geom it must be
 * configured to specify the format of the height data.
 * This call specifies that the heightfield data is computed by
 * the user and it should use the given callback when determining
 * the height of a given element of it's shape.
 *
 * @param d A new dHeightfieldDataID created by dGeomHeightfieldDataCreate
 *
 * @param width Specifies the total 'width' of the heightfield along
 * the geom's local x axis.
 * @param depth Specifies the total 'depth' of the heightfield along
 * the geom's local z axis.
 *
 * @param widthSamples Specifies the number of vertices to sample
 * along the width of the heightfield. Each vertex has a corresponding
 * height value which forms the overall shape.
 * Naturally this value must be at least two or more.
 * @param depthSamples Specifies the number of vertices to sample
 * along the depth of the heightfield.
 *
 * @param scale A uniform scale applied to all raw height data.
 * @param offset An offset applied to the scaled height data.
 *
 * @param thickness A value subtracted from the lowest height
 * value which in effect adds an additional cuboid to the base of the
 * heightfield. This is used to prevent geoms from looping under the
 * desired terrain and not registering as a collision. Note that the
 * thickness is not affected by the scale or offset parameters.
 *
 * @param bWrap If non-zero the heightfield will infinitely tile in both
 * directions along the local x and z axes. If zero the heightfield is
 * bounded from zero to width in the local x axis, and zero to depth in
 * the local z axis.
 *
 * @ingroup collide
 */
ODE_API void dGeomHeightfieldDataBuildCallback( dHeightfieldDataID d,
				void* pUserData, dHeightfieldGetHeight* pCallback,
				dReal width, dReal depth, int widthSamples, int depthSamples,
				dReal scale, dReal offset, dReal thickness, int bWrap );

/**
 * @brief Configures a dHeightfieldDataID to use height data in byte format.
 *
 * Before a dHeightfieldDataID can be used by a geom it must be
 * configured to specify the format of the height data.
 * This call specifies that the heightfield data is stored as a rectangular
 * array of bytes (8 bit unsigned) representing the height at each sample point.
 *
 * @param d A new dHeightfieldDataID created by dGeomHeightfieldDataCreate
 *
 * @param pHeightData A pointer to the height data.
 * @param bCopyHeightData When non-zero the height data is copied to an
 * internal store. When zero the height data is accessed by reference and
 * so must persist throughout the lifetime of the heightfield.
 *
 * @param width Specifies the total 'width' of the heightfield along
 * the geom's local x axis.
 * @param depth Specifies the total 'depth' of the heightfield along
 * the geom's local z axis.
 *
 * @param widthSamples Specifies the number of vertices to sample
 * along the width of the heightfield. Each vertex has a corresponding
 * height value which forms the overall shape.
 * Naturally this value must be at least two or more.
 * @param depthSamples Specifies the number of vertices to sample
 * along the depth of the heightfield.
 *
 * @param scale A uniform scale applied to all raw height data.
 * @param offset An offset applied to the scaled height data.
 *
 * @param thickness A value subtracted from the lowest height
 * value which in effect adds an additional cuboid to the base of the
 * heightfield. This is used to prevent geoms from looping under the
 * desired terrain and not registering as a collision. Note that the
 * thickness is not affected by the scale or offset parameters.
 *
 * @param bWrap If non-zero the heightfield will infinitely tile in both
 * directions along the local x and z axes. If zero the heightfield is
 * bounded from zero to width in the local x axis, and zero to depth in
 * the local z axis.
 *
 * @ingroup collide
 */
ODE_API void dGeomHeightfieldDataBuildByte( dHeightfieldDataID d,
				const unsigned char* pHeightData, int bCopyHeightData,
				dReal width, dReal depth, int widthSamples, int depthSamples,
				dReal scale, dReal offset, dReal thickness,	int bWrap );

/**
 * @brief Configures a dHeightfieldDataID to use height data in short format.
 *
 * Before a dHeightfieldDataID can be used by a geom it must be
 * configured to specify the format of the height data.
 * This call specifies that the heightfield data is stored as a rectangular
 * array of shorts (16 bit signed) representing the height at each sample point.
 *
 * @param d A new dHeightfieldDataID created by dGeomHeightfieldDataCreate
 *
 * @param pHeightData A pointer to the height data.
 * @param bCopyHeightData When non-zero the height data is copied to an
 * internal store. When zero the height data is accessed by reference and
 * so must persist throughout the lifetime of the heightfield.
 *
 * @param width Specifies the total 'width' of the heightfield along
 * the geom's local x axis.
 * @param depth Specifies the total 'depth' of the heightfield along
 * the geom's local z axis.
 *
 * @param widthSamples Specifies the number of vertices to sample
 * along the width of the heightfield. Each vertex has a corresponding
 * height value which forms the overall shape.
 * Naturally this value must be at least two or more.
 * @param depthSamples Specifies the number of vertices to sample
 * along the depth of the heightfield.
 *
 * @param scale A uniform scale applied to all raw height data.
 * @param offset An offset applied to the scaled height data.
 *
 * @param thickness A value subtracted from the lowest height
 * value which in effect adds an additional cuboid to the base of the
 * heightfield. This is used to prevent geoms from looping under the
 * desired terrain and not registering as a collision. Note that the
 * thickness is not affected by the scale or offset parameters.
 *
 * @param bWrap If non-zero the heightfield will infinitely tile in both
 * directions along the local x and z axes. If zero the heightfield is
 * bounded from zero to width in the local x axis, and zero to depth in
 * the local z axis.
 *
 * @ingroup collide
 */
ODE_API void dGeomHeightfieldDataBuildShort( dHeightfieldDataID d,
				const short* pHeightData, int bCopyHeightData,
				dReal width, dReal depth, int widthSamples, int depthSamples,
				dReal scale, dReal offset, dReal thickness, int bWrap );

/**
 * @brief Configures a dHeightfieldDataID to use height data in 
 * single precision floating point format.
 *
 * Before a dHeightfieldDataID can be used by a geom it must be
 * configured to specify the format of the height data.
 * This call specifies that the heightfield data is stored as a rectangular
 * array of single precision floats representing the height at each
 * sample point.
 *
 * @param d A new dHeightfieldDataID created by dGeomHeightfieldDataCreate
 *
 * @param pHeightData A pointer to the height data.
 * @param bCopyHeightData When non-zero the height data is copied to an
 * internal store. When zero the height data is accessed by reference and
 * so must persist throughout the lifetime of the heightfield.
 *
 * @param width Specifies the total 'width' of the heightfield along
 * the geom's local x axis.
 * @param depth Specifies the total 'depth' of the heightfield along
 * the geom's local z axis.
 *
 * @param widthSamples Specifies the number of vertices to sample
 * along the width of the heightfield. Each vertex has a corresponding
 * height value which forms the overall shape.
 * Naturally this value must be at least two or more.
 * @param depthSamples Specifies the number of vertices to sample
 * along the depth of the heightfield.
 *
 * @param scale A uniform scale applied to all raw height data.
 * @param offset An offset applied to the scaled height data.
 *
 * @param thickness A value subtracted from the lowest height
 * value which in effect adds an additional cuboid to the base of the
 * heightfield. This is used to prevent geoms from looping under the
 * desired terrain and not registering as a collision. Note that the
 * thickness is not affected by the scale or offset parameters.
 *
 * @param bWrap If non-zero the heightfield will infinitely tile in both
 * directions along the local x and z axes. If zero the heightfield is
 * bounded from zero to width in the local x axis, and zero to depth in
 * the local z axis.
 *
 * @ingroup collide
 */
ODE_API void dGeomHeightfieldDataBuildSingle( dHeightfieldDataID d,
				const float* pHeightData, int bCopyHeightData,
				dReal width, dReal depth, int widthSamples, int depthSamples,
				dReal scale, dReal offset, dReal thickness, int bWrap );

/**
 * @brief Configures a dHeightfieldDataID to use height data in 
 * double precision floating point format.
 *
 * Before a dHeightfieldDataID can be used by a geom it must be
 * configured to specify the format of the height data.
 * This call specifies that the heightfield data is stored as a rectangular
 * array of double precision floats representing the height at each
 * sample point.
 *
 * @param d A new dHeightfieldDataID created by dGeomHeightfieldDataCreate
 *
 * @param pHeightData A pointer to the height data.
 * @param bCopyHeightData When non-zero the height data is copied to an
 * internal store. When zero the height data is accessed by reference and
 * so must persist throughout the lifetime of the heightfield.
 *
 * @param width Specifies the total 'width' of the heightfield along
 * the geom's local x axis.
 * @param depth Specifies the total 'depth' of the heightfield along
 * the geom's local z axis.
 *
 * @param widthSamples Specifies the number of vertices to sample
 * along the width of the heightfield. Each vertex has a corresponding
 * height value which forms the overall shape.
 * Naturally this value must be at least two or more.
 * @param depthSamples Specifies the number of vertices to sample
 * along the depth of the heightfield.
 *
 * @param scale A uniform scale applied to all raw height data.
 * @param offset An offset applied to the scaled height data.
 *
 * @param thickness A value subtracted from the lowest height
 * value which in effect adds an additional cuboid to the base of the
 * heightfield. This is used to prevent geoms from looping under the
 * desired terrain and not registering as a collision. Note that the
 * thickness is not affected by the scale or offset parameters.
 *
 * @param bWrap If non-zero the heightfield will infinitely tile in both
 * directions along the local x and z axes. If zero the heightfield is
 * bounded from zero to width in the local x axis, and zero to depth in
 * the local z axis.
 *
 * @ingroup collide
 */
ODE_API void dGeomHeightfieldDataBuildDouble( dHeightfieldDataID d,
				const double* pHeightData, int bCopyHeightData,
				dReal width, dReal depth, int widthSamples, int depthSamples,
				dReal scale, dReal offset, dReal thickness, int bWrap );

/**
 * @brief Manually set the minimum and maximum height bounds.
 *
 * This call allows you to set explicit min / max values after initial
 * creation typically for callback heightfields which default to +/- infinity,
 * or those whose data has changed. This must be set prior to binding with a
 * geom, as the the AABB is not recomputed after it's first generation.
 *
 * @remarks The minimum and maximum values are used to compute the AABB
 * for the heightfield which is used for early rejection of collisions.
 * A close fit will yield a more efficient collision check.
 *
 * @param d A dHeightfieldDataID created by dGeomHeightfieldDataCreate
 * @param min_height The new minimum height value. Scale, offset and thickness is then applied.
 * @param max_height The new maximum height value. Scale and offset is then applied.
 * @ingroup collide
 */
ODE_API void dGeomHeightfieldDataSetBounds( dHeightfieldDataID d,
				dReal minHeight, dReal maxHeight );


/**
 * @brief Assigns a dHeightfieldDataID to a heightfield geom.
 *
 * Associates the given dHeightfieldDataID with a heightfield geom.
 * This is done without affecting the GEOM_PLACEABLE flag.
 *
 * @param g A geom created by dCreateHeightfield
 * @param d A dHeightfieldDataID created by dGeomHeightfieldDataCreate
 * @ingroup collide
 */
ODE_API void dGeomHeightfieldSetHeightfieldData( dGeomID g, dHeightfieldDataID d );


/**
 * @brief Gets the dHeightfieldDataID bound to a heightfield geom.
 *
 * Returns the dHeightfieldDataID associated with a heightfield geom.
 *
 * @param g A geom created by dCreateHeightfield
 * @return The dHeightfieldDataID which may be NULL if none was assigned.
 * @ingroup collide
 */
ODE_API dHeightfieldDataID dGeomHeightfieldGetHeightfieldData( dGeomID g );



/* ************************************************************************ */
/* utility functions */

ODE_API void dClosestLineSegmentPoints (const dVector3 a1, const dVector3 a2,
				const dVector3 b1, const dVector3 b2,
				dVector3 cp1, dVector3 cp2);

ODE_API int dBoxTouchesBox (const dVector3 _p1, const dMatrix3 R1,
		    const dVector3 side1, const dVector3 _p2,
		    const dMatrix3 R2, const dVector3 side2);

// The meaning of flags parameter is the same as in dCollide()
ODE_API int dBoxBox (const dVector3 p1, const dMatrix3 R1,
	     const dVector3 side1, const dVector3 p2,
	     const dMatrix3 R2, const dVector3 side2,
	     dVector3 normal, dReal *depth, int *return_code,
	     int flags, dContactGeom *contact, int skip);

ODE_API void dInfiniteAABB (dGeomID geom, dReal aabb[6]);


/* ************************************************************************ */
/* custom classes */

typedef void dGetAABBFn (dGeomID, dReal aabb[6]);
typedef int dColliderFn (dGeomID o1, dGeomID o2,
			 int flags, dContactGeom *contact, int skip);
typedef dColliderFn * dGetColliderFnFn (int num);
typedef void dGeomDtorFn (dGeomID o);
typedef int dAABBTestFn (dGeomID o1, dGeomID o2, dReal aabb[6]);

typedef struct dGeomClass {
  int bytes;
  dGetColliderFnFn *collider;
  dGetAABBFn *aabb;
  dAABBTestFn *aabb_test;
  dGeomDtorFn *dtor;
} dGeomClass;

ODE_API int dCreateGeomClass (const dGeomClass *classptr);
ODE_API void * dGeomGetClassData (dGeomID);
ODE_API dGeomID dCreateGeom (int classnum);

/**
 * @brief Sets a custom collider function for two geom classes. 
 *
 * @param i The first geom class handled by this collider
 * @param j The second geom class handled by this collider
 * @param fn The collider function to use to determine collisions.
 * @ingroup collide
 */
ODE_API void dSetColliderOverride (int i, int j, dColliderFn *fn);


/* ************************************************************************ */

#ifdef __cplusplus
}
#endif

#endif
