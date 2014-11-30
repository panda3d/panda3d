/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMInteger.h
	The file containing functions and constants for integer values.
 */

#ifndef _FM_INTEGER_H_
#define _FM_INTEGER_H_

/** A dynamically-sized array of 32-bit signed integer values. */
typedef fm::vector<int32, true> Int32List;
/** A dynamically-sized array of 32-bit unsigned integer values. */
typedef fm::vector<uint32, true> UInt32List;
/** A dynamically-sized array of 16-bit unsigned integer values. */
typedef fm::vector<uint16, true> UInt16List;
/** A dynamically-sized array of 8-bit unsigned integer values. */
typedef fm::vector<uint8, true> UInt8List;
/** A dynamically-sized array of 8-bit signed integer values. */
typedef fm::vector<int8, true> Int8List;
/** A dynamically-sized array of boolean values. */
typedef fm::vector<bool, true> BooleanList;

/** Returns whether two signed or unsigned integers are equivalent.
	For integers, this function simply wraps around the operator==.
	@param i1 A first integer.
	@param i2 A second integer.
	@return Whether the two integers are equivalent. */
inline bool IsEquivalent(int8 i1, int8 i2) { return i1 == i2; }
inline bool IsEquivalent(uint8 i1, uint8 i2) { return i1 == i2; } /**< See above. */
inline bool IsEquivalent(int16 i1, int16 i2) { return i1 == i2; } /**< See above. */
inline bool IsEquivalent(uint16 i1, uint16 i2) { return i1 == i2; } /**< See above. */
inline bool IsEquivalent(int32 i1, int32 i2) { return i1 == i2; } /**< See above. */
inline bool IsEquivalent(uint32 i1, uint32 i2) { return i1 == i2; } /**< See above. */
inline bool IsEquivalent(int64 i1, int64 i2) { return i1 == i2; } /**< See above. */
inline bool IsEquivalent(uint64 i1, uint64 i2) { return i1 == i2; } /**< See above. */

/** Returns whether two Booleans are equivalent.
	@param b1 A first Boolean.
	@param b2 A second Boolean.
	@return Whether the two Booleans are equivalent. */
inline bool IsEquivalent(bool b1, bool b2) { return b1 == b2; }

#endif // _FM_INTEGER_H_

