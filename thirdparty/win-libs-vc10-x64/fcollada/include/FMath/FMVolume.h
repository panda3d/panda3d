/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMVolume.h
	This file contains the functions for working with volumes.
	This includes finding volumes of various shapes.
*/

#ifndef _FM_VOLUME_H_
#define _FM_VOLUME_H_

namespace FMVolume
{
	/** Calculates the volume of an axis-aligned box.
		@param halfExtents The three half-lengths the compose the box. 
		@return The volume. */
	FCOLLADA_EXPORT float CalculateBoxVolume(const FMVector3& halfExtents);
	
	/** Calculates the volume of a sphere.
		@param radius The radius of the sphere. 
		@return The volume. */
	FCOLLADA_EXPORT float CalculateSphereVolume(float radius);
	
	/** Calculates the volume of an ellipsoid.
		@param radius1 The radius along one axis.
		@param radius2 The radius along another axis.
		@param radius3 The radius along the third axis.
		@return The volume. */
	FCOLLADA_EXPORT float CalculateEllipsoidVolume(float radius1, float radius2, float radius3);
	
	/** Cacluates the volume of an ellipsoid if it is at the end of an elliptical geometry.
		The cross section at the end is given by the radius.
		@param radius The radius of the cross section at the end of the geometry. It may be elliptical.
		@return The volume. */
	FCOLLADA_EXPORT float CalculateEllipsoidEndVolume(const FMVector2& radius);
	
	/** Calculates the volume of a cylinder.
		@param radius The radius of the cylinder caps. It may be elliptical.
		@param height The height of the cylinder shaft. 
		@return The volume. */
	FCOLLADA_EXPORT float CalculateCylinderVolume(const FMVector2& radius, float height);
	
	/** Calculates the volume of a capsule.
		@param radius The radius of the capsule caps. It may be elliptical
		@param height The radius of the capsule shaft. 
		@return The volume. */
	FCOLLADA_EXPORT float CalculateCapsuleVolume(const FMVector2& radius, float height);
	
	/** Calculates the volume of a cone.
		@param radius The radius of the cone cap or bottom. It may be elliptical
		@param height The height of the cone. 
		@return The volume. */
	FCOLLADA_EXPORT float CalculateConeVolume(const FMVector2& radius, float height);
	
	/** Calculates the volume of a tapered cylinder.
		@param radius The radius of the first cap of the cylinder.
		@param radius2 The radius of the second cap of the cylinder.
		@param height The height of the cylinder shaft. 
		@return The volume. */
	FCOLLADA_EXPORT float CalculateTaperedCylinderVolume(const FMVector2& radius, const FMVector2& radius2, float height);
}

#endif // _FM_VOLUME_H_
