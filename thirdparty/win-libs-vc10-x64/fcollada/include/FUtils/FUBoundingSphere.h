/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUBoundingSphere.h
	This file contains the FUBoundingSphere class.
*/

#ifndef _FU_BOUNDINGSPHERE_H_
#define _FU_BOUNDINGSPHERE_H_

class FUBoundingBox;

/**
	A bounding sphere.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUBoundingSphere
{
private:
	FMVector3 center;
	float radius;

public:
	/** Empty constructor.
		The center and radius parameters are set at the largest and most impossible values. */
	FUBoundingSphere();

	/** Constructor.
		@param center The center of the bounding sphere.
		@param radius The radius of the bounding sphere. */
	FUBoundingSphere(const FMVector3& center, float radius);

	/** Copy constructor.
		@param copy The bounding sphere to duplicate. */
	FUBoundingSphere(const FUBoundingSphere& copy);

	/** Destructor. */
	~FUBoundingSphere();

	/** Resets the bounding sphere.
		The center and radius are set at the largest and most impossible values.
		Including a freshly reset bounding sphere to a valid bounding sphere will have no effect. */
	void Reset();

	/** Retrieves whether the bounding sphere contains valid information.
		An invalid bounding sphere has a negative radius.
		Reseting the bounding sphere and the empty constructor generate invalid bounding spheres on purpose.
		@return The validity state of the bounding sphere. */
	bool IsValid() const;

	/** Retrieves the center of the bounding sphere.
		@return The bounding sphere center. */
	inline const FMVector3& GetCenter() const { return center; }

	/** Retrieves the radius of the bounding sphere.
		@return The bounding sphere radius. */
	inline float GetRadius() const { return radius; }

	/** Sets the center of the bounding sphere.
		@param _center The new center of the bounding sphere. */
	inline void SetCenter(const FMVector3& _center) { center = _center; }

	/** Sets the radius of the bounding sphere.
		@param _radius The radius of the bounding sphere. */
	inline void SetRadius(float _radius) { radius = _radius; }

	/** Retrieves whether a given point is contained within the bounding sphere.
		@param point A 3D coordinate.
		@return Whether the given 3D coordinate is inside the bounding sphere. */
	bool Contains(const FMVector3& point) const;

	/** Retrieves whether a bounding sphere overlaps with this bounding sphere.
		@param boundingSphere A bounding sphere.
		@param overlapCenter An optional pointer to retrieve the center of the overlap region.
		@return Whether the given bounding sphere overlaps with this bounding sphere. */
	bool Overlaps(const FUBoundingSphere& boundingSphere, FMVector3* overlapCenter = NULL) const;

	/** Retrieves whether a bounding box overlaps with this bounding sphere.
		@param boundingBox A bounding box.
		@param overlapCenter An optional pointer to retrieve the center of the overlap region.
			For this particular case, there is no guarantee that this is the exact center of the overlap region.
		@return Whether the given bounding box overlaps with this bounding sphere. */
	bool Overlaps(const FUBoundingBox& boundingBox, FMVector3* overlapCenter = NULL) const;

	/** Extends the bounding sphere to include the given 3D coordinate.
		@param point A 3D coordinate to include in the bounding sphere. */
	void Include(const FMVector3& point);

	/** Extends the bounding sphere to include another bounding sphere.
		@param boundingSphere A bounding sphere to include in this bounding sphere. */
	void Include(const FUBoundingSphere& boundingSphere);

	/** Extends the bounding sphere to include a bounding box.
		@param boundingBox A bounding box to include in this bounding sphere. */
	void Include(const FUBoundingBox& boundingBox);

	/** Transform the bounding sphere into another space basis.
		@param transform The transformation matrix to go into the other space basis.
			Skews and projections may strangely affect the resulting bounding sphere.
		@return The transformed bounding sphere. */
	FUBoundingSphere Transform(const FMMatrix44& transform) const;
};

#endif // _FU_BOUNDINGSPHERE_H_
