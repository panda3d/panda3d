/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMRandom.h
	This file contains functions related to generating pseudo-random values.
 */

#ifndef _FM_RANDOM_H_
#define _FM_RANDOM_H_

/** Contains pseudo-random number generation functions. */
namespace FMRandom
{
	/** Seeds the pseudo-random number generator.
		@param seed The new pseudo-random number generator seed. */
	FCOLLADA_EXPORT void Seed(uint32 seed);

	/** Retrieves one random unsigned integer.
		The expected valid range is [0,0x7FFF].
		@return A random unsigned integer. */
	FCOLLADA_EXPORT uint32 GetUInt32();

	/** Retrieves one random unsigned integer within a given range.
		The generated values will be in the range: [0, rangeEnd[.
		@param rangeEnd The end of the wanted range.
		@return A random unsigned integer. */
	inline uint32 GetUInt32(uint32 rangeEnd) { return GetUInt32() % rangeEnd; }

	/** Retrieves one random unsigned integer within a given range.
		The generated values will be in the range: [rangeStart, rangeEnd[.
		@param rangeStart The start of the wanted range.
		@param rangeEnd The end of the wanted range.
		@return A random unsigned integer. */
	inline uint32 GetUInt32(uint32 rangeStart, uint32 rangeEnd) { return (GetUInt32() % (rangeEnd - rangeStart)) + rangeStart; }

	/** Retrieves one random boolean value.
		There should be equal probability that you get true or false values.
		@return A random boolean. */
	inline bool GetBoolean() { return (bool) (GetUInt32() & 0x1); }

	/** Retrieves one random signed integer.
		The expected valid range is [-0x3FFF,0x4000].
		@return A random signed integer. */
	FCOLLADA_EXPORT int32 GetInt32();

	/** Retrieves one random signed integer.
		The generated values will be in the range: [rangeStart, rangeEnd[.
		@param rangeStart The start of the wanted range.
		@param rangeEnd The end of the wanted range.
		@return A random signed integer. */
	inline int32 GetInt32(int32 rangeStart, int32 rangeEnd) { return (((int32) GetUInt32()) % (rangeEnd - rangeStart)) + rangeStart; }

	/** Retrieves one random floating-point value.
		The expected valid range is [0,1].
		@return A random floating-point value. */
	FCOLLADA_EXPORT float GetFloat();

	/** Retrieves one random floating-point value within a given range.
		The generated values will be in the range [0, rangeEnd].
		@param rangeEnd The end of the wanted range.
		@return A random floating-point value. */
	inline float GetFloat(float rangeEnd) { return GetFloat() * rangeEnd; }

	/** Retrieves one random floating-point value within a given range.
		The generated values will be in the range [rangeStart, rangeEnd].
		@param rangeStart The start of the wanted range.
		@param rangeEnd The end of the wanted range.
		@return A random floating-point value. */
	inline float GetFloat(float rangeStart, float rangeEnd) { return GetFloat() * (rangeEnd - rangeStart) + rangeStart; }
};

#endif // _FM_RANDOM_H_

