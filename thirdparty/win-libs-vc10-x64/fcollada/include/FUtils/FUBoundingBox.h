/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUBoundingBox.h
	This file contains the FUBoundingBox class.
*/

#ifndef _FU_BOUNDINGBOX_H_
#define _FU_BOUNDINGBOX_H_

class FUBoundingSphere;

/**
	An axis-aligned bounding box.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUBoundingBox
{
private:
	FMVector3 minimum;
	FMVector3 maximum;

public:
	/** Empty constructor.
		The minimum and maximum bounds are set at the largest and most impossible values. */
	FUBoundingBox();

	/** Constructor.
		@param minimum The minimum bounds of the bounding box.
		@param maximum The maximum bounds of the bounding box. */
	FUBoundingBox(const FMVector3& minimum, const FMVector3& maximum);

	/** Copy constructor.
		@param copy The bounding box to duplicate. */
	FUBoundingBox(const FUBoundingBox& copy);

	/** Destructor. */
	~FUBoundingBox();

	/** Resets the bounding box.
		The minimum and maximum bounds are set at the largest and most impossible values.
		Including a freshly reset bounding box to a valid bounding box will have no effect. */
	void Reset();

	/** Retrieves whether the bounding box contains valid information.
		An invalid bounding box has a minimum value that is greater than the maximum value.
		Reseting the bounding box and the empty constructor generate invalid bounding boxes on purpose.
		@return The validity state of the bounding box. */
	bool IsValid() const;

	/** Retrieves the minimum bounds of the bounding box.
		@return The minimum bounds. */
	inline const FMVector3& GetMin() const { return minimum; }

	/** Retrieves the maximum bounds of the bounding box.
		@return The maximum bounds. */
	inline const FMVector3& GetMax() const { return maximum; }

	/** Sets the minimum bounds of the bounding box.
		@param _min The new minimum bounds of the bounding box. */
	inline void SetMin(const FMVector3& _min) { minimum = _min; }

	/** Sets the maximum bounds of the bounding box.
		@param _max The new maximum bounds of the bounding box. */
	inline void SetMax(const FMVector3& _max) { maximum = _max; }

	/** Retrieves the center of the bounding box.
		@return The center of the bounding box. */
	inline FMVector3 GetCenter() const { return (minimum + maximum) / 2.0f; }

	/** Retrieves whether the bounding box contains a given 3D coordinate.
		@param point A 3D coordinate.
		@return Whether the coordinate is contained by the bounding box. */
	bool Contains(const FMVector3& point) const;

	/** Retrieves whether this bounding box overlaps a given bounding box.
		@param boundingBox A bounding box.
		@param overlapCenter An optional pointer to retrieve the center of the overlap region.
		@return Whether the two bounding boxes overlap. */
	bool Overlaps(const FUBoundingBox& boundingBox, FMVector3* overlapCenter = NULL) const;

	/** Retrieves whether this bounding box overlaps a given bounding sphere.
		@param boundingSphere A bounding sphere.
		@param overlapCenter An optional pointer to retrieve the center of the overlap region.
			For this particular case, there is no guarantee that this is the exact center of the overlap region.
		@return Whether this bounding box overlaps the bounding sphere. */
	bool Overlaps(const FUBoundingSphere& boundingSphere, FMVector3* overlapCenter = NULL) const;

	/** Extends the bounding box to include the given 3D coordinate.
		@param point A 3D coordinate to include in the bounding box. */
	void Include(const FMVector3& point);

	/** Extends the bounding box to include another bounding box.
		@param boundingBox A bounding box to include in this bounding box. */
	void Include(const FUBoundingBox& boundingBox);

	/** Transform the bounding box into another space basis.
		@param transform The transformation matrix to go into the other space basis.
		@return The transformed bounding box. */
	FUBoundingBox Transform(const FMMatrix44& transform) const;

	/** Evaluates if this bounding box is equal to the one at the RHS.
		@param right The bounding box to test against.
		@return True if the two boxes are equivalent.*/
	bool Equals(const FUBoundingBox& right) const;

public:
	/** Represents an infinite bounding box, including all space.*/
	static const FUBoundingBox Infinity;
};

#endif // _FU_BOUNDINGBOX_H_
