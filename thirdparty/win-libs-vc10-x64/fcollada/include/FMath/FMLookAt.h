/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMLookAt.h
	This file contains the FMLookAt class.
*/

#ifndef _FM_LOOKAT_H_
#define _FM_LOOKAT_H_

/**
	A look-at value.
	@ingroup FMath
*/
class FCOLLADA_EXPORT FMLookAt
{
public:
	FMVector3 position; /**< The position of the viewer.
						     Defines the translation. */
	FMVector3 target; /**< The target of the viewer.
					       Defines the pitch and the yaw of the transform. */
	FMVector3 up; /**< The up-axis of the viewer.
				       Defines the roll of the transform. */

	/** Default Constructor. */
	FMLookAt();

	/** Constructor.
		@param _position The position of the viewer.
		@param _target The target of the viewer.
		@param _up The up-axis of the viewer. */
	FMLookAt(const FMVector3& _position, const FMVector3& _target, const FMVector3& _up);
};

/** Retrieves whether one look-at value is equivalent to a second look-at value.
	@param first A first look-at value. 
	@param other A second look-at value. */
bool operator==(const FMLookAt& first, const FMLookAt& other);

#endif // _FM_LOOKAT_H_

