/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDAnimationKey.h
	This file contains the FCDAnimationKey, FCDAnimationKeyBezier and FCDAnimationKeyTCB classes.
*/	

#ifndef _FCD_ANIMATION_KEY_H_
#define _FCD_ANIMATION_KEY_H_

/**
	A simple animation key.
	This class is the base for the more complex one-dimensional keys
	and it is used directly for linear and step keys.

	Do not create directly.
	Instead call FCDAnimationCurve::AddKey(FUDaeInterpolation::LINEAR)
	or FCDAnimationCurve::AddKey(FUDaeInterpolation::STEP).
*/
class FCDAnimationKey
{
public:
	/** The key input. Typically, this will be a time value, in seconds.
		For driven curves, the dimension of this value will depend on the driver. */
	float input;

	/** The key output. */
	float output;

	/** The key interpolation type.
		@see FUDaeInterpolation::Interpolation */
	uint32 interpolation;
};

/**
	An animation key with tangents values.
	This class is used for bezier keys and soon: for hermite keys as well.
	
	Do not create directly.
	Instead call FCDAnimationCurve::AddKey(FUDaeInterpolation::BEZIER).
*/
class FCDAnimationKeyBezier : public FCDAnimationKey
{
public:
	FMVector2 inTangent; /**< The incoming tangent value. */
	FMVector2 outTangent; /**< The outcoming tangent value. */
};

/**
	An animation key with tension, continuity and bias values.
	This class is used for 3dsMax TCB keys.
	
	Do not create directly.
	Instead call FCDAnimationCurve::AddKey(FUDaeInterpolation::TCB).
*/
class FCDAnimationKeyTCB : public FCDAnimationKey
{
public:
	float tension; /**< The tension. */
	float continuity; /**< The continuity. */
	float bias; /**< The bias. */

	float easeIn; /**< The ease-in factor. */
	float easeOut; /**< The ease-out factor. */
};

/**
	A simple multi-dimensional animation key.
	This class is the base for the more complex multi-dimensional keys
	and it is used directly for linear and step multi-dimensional keys.
*/
class FCDAnimationMKey
{
private:
	uint32 dimension;

public:
	/** Constructor. Do not use directly.
		Instead call FCDAnimationMultiCurve::AddKey(FUDaeInterpolation::LINEAR)
		or FCDAnimationMultiCurve::AddKey(FUDaeInterpolation::STEP).
		@param dimension The number of dimension to the key output. */
	FCDAnimationMKey(uint32 dimension);

	/** Destructor. */
	virtual ~FCDAnimationMKey();

	/** Retrieves the number of dimensions for this key.
		@return The number of dimensions. */
	uint32 GetDimension() const { return dimension; };

	/** The key input. Typically, this will be a time value, in seconds.
		For driven curves, the dimension of this value will depend on the driver. */
	float input;

	/** The key interpolation type.
		@see FUDaeInterpolation::Interpolation */
	uint32 interpolation;

	/** The multi-dimensional key output. */
	float* output;
};

/**
	A multi-dimensional animation key with tangents values.
	This class is used for bezier keys and soon: for hermite keys as well.
*/
class FCDAnimationMKeyBezier : public FCDAnimationMKey
{
public:
	/** Constructor: do not use directly.
		Instead call FCDAnimationCurve::AddKey(FUDaeInterpolation::BEZIER).
		@param dimension The number of dimension to the key output. */
	FCDAnimationMKeyBezier(uint32 dimension);

	/** Destructor. */
	virtual ~FCDAnimationMKeyBezier();

	FMVector2* inTangent; /**< The incoming tangent value. */
	FMVector2* outTangent; /**< The outcoming tangent value. */
};

/**
	An animation key with tension, continuity and bias values.
	This class is used for 3dsMax TCB keys.
*/
class FCDAnimationMKeyTCB : public FCDAnimationMKey
{
public:
	/** Constructor: do not use directly.
		Instead call FCDAnimationMultiCurve::AddKey(FUDaeInterpolation::TCB).
		@param dimension The number of dimension to the key output. */
	FCDAnimationMKeyTCB(uint32 dimension);

	/** Destructor. */
	virtual ~FCDAnimationMKeyTCB();

	float* tension; /**< The multi-dimensional tensions. */
	float* continuity; /**< The multi-dimensional continuities. */
	float* bias; /**< The multi-dimensional biases. */

	float* easeIn; /**< The multi-dimensional ease-in factors. */
	float* easeOut; /**< The multi-dimensional ease-out factors. */
};

#endif // _FCD_ANIMATION_KEY_H_

