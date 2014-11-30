/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMAngleAxis.h
	This file contains the FMAngleAxis class.
*/

#ifndef _FM_ANGLEAXIS_H_
#define _FM_ANGLEAXIS_H_

/** 
	An axis-angle rotation value.
	@ingroup FUParameter
*/
class FCOLLADA_EXPORT FMAngleAxis
{
public:
	FMVector3 axis; /**< The axis of rotation. */
	float angle; /**< The angle of rotation. */

	/** Default Constructor. */
	FMAngleAxis();

	/** Constructor.
		@param _axis A 3D rotation axis.
		@param _angle The rotation angle, in degrees. */
	FMAngleAxis(const FMVector3& _axis, float _angle);
};

/** Retrieves whether one angle-axis rotation is
	equivalent to a second angle-axis rotation.
	@param first A first angle-axis rotation. 
	@param other A second angle-axis rotation. */
bool operator==(const FMAngleAxis& first, const FMAngleAxis& other);

#endif // _FM_ANGLEAXIS_H_

