/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMSkew.h
	This file contains the FMSkew class.
*/

#ifndef _FM_SKEW_H_
#define _FM_SKEW_H_

/** 
	A skew value.
	@ingroup FUParameter
*/
class FCOLLADA_EXPORT FMSkew
{
public:
	FMVector3 rotateAxis; /**< The axis which is rotated. */
	FMVector3 aroundAxis; /**< The axis around which to rotate. */
	float angle; /**< The angle of rotation. */

	/** Default Constructor. */
	FMSkew();

	/** Constructor.
		@param rotateAxis The axis which is rotated.
		@param aroundAxis The axis around which to rotate.
		@param angle The rotation angle, in degrees. */
	FMSkew(const FMVector3& rotateAxis, const FMVector3& aroundAxis, float angle);
};

/** Retrieves whether one skew is equivalent to a second skew.
	@param first A first skew. 
	@param other A second skew. */
bool operator==(const FMSkew& first, const FMSkew& other);

#endif // _FM_SKEW_H_

