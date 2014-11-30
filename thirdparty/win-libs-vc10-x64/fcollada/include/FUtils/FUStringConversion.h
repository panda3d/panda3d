/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUStringConversion.h
	This file contains the FUStringConversion class.
*/

#ifndef _FCU_STRING_CONVERSION_
#define _FCU_STRING_CONVERSION_

#ifndef UINT_MAX
#define UINT_MAX ~(uint32)0
#endif // UINT_MAX

class FUDateTime;

/**
	Common string conversion.

	This static class contains the parsing function for Unicode and 8-bit/UTF-8
	fm::strings into common data types: integers, booleans, floating-point values,
	vectors, matrices, date-time, etc. and dynamically-sized array of these types.

	This class can also convert common data types into an 8-bit or a Unicode string and 
	it contains conversion functions to convert string between 8-bit and Unicode.

	All the functions which return string objects are returning static string objects
	in order for DLLs to work correctly with non-local heaps. If you are interested
	in the value of the conversion: save it into a local string object.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUStringConversion
{
private: FUStringConversion() {}
public:

	/** Converts a 8-bit string to a Unicode string.
		@param value The 8-bit string.
		@return The converted Unicode string. */
	static fstring ToFString(const char* value);
	inline static fstring ToFString(const fm::string& value) { return ToFString(value.c_str()); } /**< See above. */

	/** Converts an Unicode string to a 8-bit string.
		@param value The Unicode string.
		@return The converted 8-bit string. */
	static fm::string ToString(const fchar* value);
	inline static fm::string ToString(const fstring& value) { return ToString(value.c_str()); } /**< See above. */

	/** Parses a string into a boolean value.
		@param value The string.
		@return The parsed boolean value. */
	template <class CH>
	static FCOLLADA_EXPORT bool ToBoolean(const CH* value);
	template <class CH>
	inline static bool ToBoolean(const fm::stringT<CH>& value) { return ToBoolean(value.c_str()); } /**< See above. */

	/** Parses a string into a floating-point value.
		@param value The string. For the string pointer versions of this function,
			the pointer will point to the last processed characters after the parsing.
		@return The parsed floating-point value. */
	template <class CH>
	static FCOLLADA_EXPORT float ToFloat(const CH** value);
	template <class CH>
	inline static float ToFloat(const CH* value) { return ToFloat(&value); } /**< See above. */
	template <class CH>
	inline static float ToFloat(const fm::stringT<CH>& value) { return ToFloat(value.c_str()); } /**< See above. */

	/** Parses a string into a signed integer.
		@param value The string. For the string pointer versions of this function,
			the pointer will point to the last processed characters after the parsing.
		@return The parsed signed integer. */
	template <class CH>
	static FCOLLADA_EXPORT int32 ToInt32(const CH** value);
	template <class CH>
	inline static int32 ToInt32(const CH* value) { return ToInt32(&value); } /**< See above. */
	template <class CH>
	inline static int32 ToInt32(const fm::stringT<CH>& value) { return ToInt32(value.c_str()); } /**< See above. */

	/** Parses a string into an unsigned integer.
		@param value The string. For the string pointer versions of this function,
			the pointer will point to the last processed characters after the parsing.
		@return The parsed unsigned integer. */
	template <class CH>
	static FCOLLADA_EXPORT uint32 ToUInt32(const CH** value);
	template <class CH>
	inline static uint32 ToUInt32(const CH* value) { return ToUInt32(&value); } /**< See above. */
	template <class CH>
	inline static uint32 ToUInt32(const fm::stringT<CH>& value) { return ToUInt32(value.c_str()); } /**< See above. */

	/** Parses a string into an unsigned integer. The string is assumed to have
		an unsigned integer in hexadecimal format.
		@param value The string. For the string pointer versions of this function,
			the pointer will point to the last processed characters after the parsing.
		@param count The maxmimum number of characters to parse.
			For example, a count of 2 will read in an 8-bit character.
		@return The parsed unsigned integer. */
	template <class CH>
	static FCOLLADA_EXPORT uint32 HexToUInt32(const CH** value, uint32 count=UINT_MAX);
	template <class CH>
	inline static uint32 HexToUInt32(const CH* value, uint32 count=UINT_MAX) { return HexToUInt32(&value, count); } /**< See above. */
	template <class CH>
	inline static uint32 HexToUInt32(const fm::stringT<CH>& value, uint32 count=UINT_MAX) { return HexToUInt32(value.c_str(), count); } /**< See above. */

	/** Parses a string into a vector.
		@param value The string. For the string pointer versions of this function,
			the pointer will point to the last processed characters after the parsing.
		@return The parsed vector. */
	template <class CH>
	static FCOLLADA_EXPORT FMVector2 ToVector2(const CH** value);
	template <class CH>
	inline static FMVector2 ToVector2(const CH* value) { return ToVector2(&value); } /**< See above. */
	template <class CH>
	inline static FMVector2 ToVector2(const fm::stringT<CH>& value) { return ToVector2(value.c_str()); } /**< See above. */
	template <class CH>
	static FCOLLADA_EXPORT FMVector3 ToVector3(const CH** value); /**< See above. */
	template <class CH>
	inline static FMVector3 ToVector3(const CH* value) { return ToVector3(&value); } /**< See above. */
	template <class CH>
	inline static FMVector3 ToVector3(const fm::stringT<CH>& value) { return ToVector3(value.c_str()); } /**< See above. */
	template <class CH>
	static FCOLLADA_EXPORT FMVector4 ToVector4(const CH** value); /**< See above. */
	template <class CH>
	inline static FMVector4 ToVector4(const CH* value) { return ToVector4(&value); } /**< See above. */
	template <class CH>
	inline static FMVector4 ToVector4(const fm::stringT<CH>& value) { return ToVector4(value.c_str()); } /**< See above. */

	/** Parses a string into a 4x4 matrix.
		@param value The string. For the string pointer versions of this function,
			the pointer will point to the last processed characters after the parsing.
		@param mx The matrix to be filled in. */
	template <class CH>
	static FCOLLADA_EXPORT void ToMatrix(const CH** value, FMMatrix44& mx);
	template <class CH>
	inline static void ToMatrix(const CH* value, FMMatrix44& mx) { ToMatrix(&value, mx); } /**< See above. */
	template <class CH>
	inline static void ToMatrix(const fm::stringT<CH>& value, FMMatrix44& mx) { ToMatrix(value.c_str(), mx); } /**< See above. */
	template <class CH>
	inline static FMMatrix44 ToMatrix(const CH* value) { FMMatrix44 mx; ToMatrix(&value, mx); return mx; } /**< See above. */
	template <class CH>
	inline static FMMatrix44 ToMatrix(const fm::stringT<CH>& value) { FMMatrix44 mx; ToMatrix(value.c_str(), mx); return mx; } /**< See above. */

	/** Parses a string into a datetime structure.
		@param value The string.
		@param dateTime The datetime structure to fill in. */
	template <class CH>
	static FCOLLADA_EXPORT void ToDateTime(const CH* value, FUDateTime& dateTime);
	template <class CH>
	inline static void ToDateTime(const fm::stringT<CH>& value, FUDateTime& dateTime) { return ToDateTime(value.c_str(), dateTime); } /**< See above. */

#ifdef HAS_VECTORTYPES

	/** Splits a string into multiple substrings.
		The separator used here are the white-spaces.
		@param value The string.
		@param array A list of strings that will be filled in. */
	static void ToFStringList(const fstring& value, FStringList& array);
	static void ToStringList(const char* value, StringList& array); /**< See above. */
#ifdef UNICODE
	static void ToStringList(const fchar* value, StringList& array); /**< See above. */
#endif // UNICODE
	template <class CH>
	inline static void ToStringList(const fm::stringT<CH>& value, StringList& array) { return ToStringList(value.c_str(), array); } /**< See above. */

	/** Parses a string into a list of boolean values.
		@param value The string.
		@param array The list of boolean values to fill in. */
	template <class CH>
	static FCOLLADA_EXPORT void ToBooleanList(const CH* value, BooleanList& array);
	template <class CH>
	inline static void ToBooleanList(const fm::stringT<CH>& value, BooleanList& array) { return ToBooleanList(value.c_str(), array); } /**< See above. */

	/** Parses a string into a list of floating point values.
		@param value The string.
		@param array The list of floating point values to fill in. */
	template <class CH>
	static FCOLLADA_EXPORT void ToFloatList(const CH* value, FloatList& array);
	template <class CH>
	inline static void ToFloatList(const fm::stringT<CH>& value, FloatList& array) { return ToFloatList(value.c_str(), array); } /**< See above. */

	/** Parses a string into a list of signed integers.
		@param value The string.
		@param array The list of signed integers to fill in. */
	template <class CH>
	static FCOLLADA_EXPORT void ToInt32List(const CH* value, Int32List& array);
	template <class CH>
	inline static void ToInt32List(const fm::stringT<CH>& value, Int32List& array) { return ToInt32List(value.c_str(), array); } /**< See above. */

	/** Parses a string into a list of unsigned integers.
		@param value The string.
		@param array The list of unsigned integers to fill in. */
	template <class CH>
	static FCOLLADA_EXPORT void ToUInt32List(const CH* value, UInt32List& array);
	template <class CH>
	inline static void ToUInt32List(const fm::stringT<CH>& value, UInt32List& array) { return ToUInt32List(value.c_str(), array); } /**< See above. */

	/** Parses a string containing interleaved floating-point values.
		The values will be stored in multiple, independent lists.
		@param value The string containing interleaved floating-point values.
		@param arrays The lists of floating-point values to fill in. */
	template <class CH>
	static FCOLLADA_EXPORT void ToInterleavedFloatList(const CH* value, fm::pvector<FloatList>& arrays);
	template <class CH>
	inline static void ToInterleavedFloatList(const fm::stringT<CH>& value, fm::pvector<FloatList>& arrays) { return ToInterleavedFloatList(value.c_str(), arrays); } /**< See above. */

	/** Parses a string containing interleaved unsigned integers.
		The values will be stored in multiple, independent lists.
		@param value The string containing interleaved unsigned integers.
		@param arrays The lists of unsigned integers to fill in. */
	template <class CH>
	static FCOLLADA_EXPORT void ToInterleavedUInt32List(const CH* value, fm::pvector<UInt32List>& arrays);
	template <class CH>
	inline static void ToInterleavedUInt32List(const fm::stringT<CH>& value, fm::pvector<UInt32List>& arrays) { return ToInterleavedFloatList(value.c_str(), arrays); } /**< See above. */

	/** Parses a string into a list of matrices.
		@param value The string.
		@param array The list of matrices to fill in. */
	template <class CH>
	static FCOLLADA_EXPORT void ToMatrixList(const CH* value, FMMatrix44List& array);
	template <class CH>
	inline static void ToMatrixList(const fm::stringT<CH>& value, FMMatrix44List& array) { return ToMatrixList(value.c_str(), array); } /**< See above. */

	/** Parses a string into a list of 2D vectors.
		@param value The string.
		@param array The list of 2D vectors to fill in.*/
	template <class CH>
	static FCOLLADA_EXPORT void ToVector2List(const CH* value, FMVector2List& array);
	template <class CH>
	inline static void ToVector2List(const fm::stringT<CH>& value, FMVector2List& array) { return ToVector2List(value.c_str(), array); } /**< See above. */

	/** Parses a string into a list of 3D vectors.
		@param value The string.
		@param array The list of 3D vectors to fill in.*/
	template <class CH>
	static FCOLLADA_EXPORT void ToVector3List(const CH* value, FMVector3List& array);
	template <class CH>
	inline static void ToVector3List(const fm::stringT<CH>& value, FMVector3List& array) { return ToVector3List(value.c_str(), array); } /**< See above. */

	/** @deprecated Parses a string into a list of 3D points. Now replaced by ToVector3List.
		@param value The string.
		@param array The list of 3D points to fill in.*/
	template <class CH>
	inline static void ToPointList(const CH* value, FMVector3List& array) { return ToVector3List(value, array); }
	template <class CH>
	inline static void ToPointList(const fm::stringT<CH>& value, FMVector3List& array) { return ToVector3List(value.c_str(), array); } /**< See above. */

	/** Parses a string into a list of 4D vectors.
		@param value The string.
		@param array The list of 4D vectors to fill in.*/
	template <class CH>
	static FCOLLADA_EXPORT void ToVector4List(const CH* value, FMVector4List& array);
	template <class CH>
	inline static void ToVector4List(const fm::stringT<CH>& value, FMVector4List& array) { return ToVector4List(value.c_str(), array); } /**< See above. */

	/** Converts a list of floating-point values into a string.
		@param builder The string builder that will contain the list of values.
			This string builder is not cleared of its contents and a space
			character will be added if it is not empty.
		@param values The list of floating-point values to convert. */
	template <class CH>
	static FCOLLADA_EXPORT void ToString(FUStringBuilderT<CH>& builder, const FloatList& values);

	/** Converts a list of signed integers into a string.
		@param builder The string builder that will contain the list of values.
			This string builder is not cleared of its contents and a space
			character will be added if it is not empty.
		@param values The list of signed integers to convert. */
	template <class CH>
	static FCOLLADA_EXPORT void ToString(FUStringBuilderT<CH>& builder, const Int32List& values);

	/** Converts a list of unsigned integers into a string.
		@param builder The string builder that will contain the list of values.
			This string builder is not cleared of its contents and a space
			character will be added if it is not empty.
		@param values The list of unsigned integers to convert. */
	template <class CH>
	inline FCOLLADA_EXPORT static void ToString(FUStringBuilderT<CH>& builder, const UInt32List& values) { return ToString(builder, values.begin(), values.size()); }

	/** Converts a list of unsigned integers into a string.
		@param builder The string builder that will contain the list of values.
			This string builder is not cleared of its contents and a space
			character will be added if it is not empty.
		@param values The contiguous array of unsigned integers to convert.
		@param count The number of values in the contiguous array. */
	template <class CH>
	static FCOLLADA_EXPORT void ToString(FUStringBuilderT<CH>& builder, const uint32* values, size_t count);

#endif // HAS_VECTORTYPES

	/** Converts a 4D vector into a string.
		@param p The 4D vector to convert.
		@return The string containing the converted vector. */
	static fm::string ToString(const FMVector4& p);
	static fstring ToFString(const FMVector4& p); /**< See above. */

	/** Converts a matrix into a string.
		@param value The matrix to convert.
		@return The string containing the converted matrix. */
	static fm::string ToString(const FMMatrix44& value);
	static fstring ToFString(const FMMatrix44& value); /**< See above. */

	/** Converts a 2D vector into a string.
		@param value The 2D vector to convert.
		@return The string containing the converted vector. */
	static fm::string ToString(const FMVector2& value);
	static fstring ToFString(const FMVector2& value); /**< See above. */

	/** Converts a 3D vector into a string.
		@param value The 3D vector to convert.
		@return The string containing the converted vector. */
	static fm::string ToString(const FMVector3& value);
	static fstring ToFString(const FMVector3& value); /**< See above. */

	/** Converts a datetime structure into a string.
		@param dateTime The datetime structure to convert.
		@return The string containing the converted datetime structure. */
	static fm::string ToString(const FUDateTime& dateTime);
	static fstring ToFString(const FUDateTime& dateTime); /**< See above. */

	/** Converts a primitive value into a string.
		This function is templatized to use the global string builders to
		convert most primitive value types, such as signed integers,
		unsigned integers and single floating-point values, into strings.
		@see FUStringBuilderT
		@param value A primitive value.
		@return The string containing the converted primitive value. */
	template <class T> static fm::string ToString(const T& value) { FUSStringBuilder builder; builder.set(value); return builder.ToString(); }
	template <class T> static fstring ToFString(const T& value) { FUStringBuilder builder; builder.set(value); return builder.ToString(); } /**< See above. */

	/** Converts a matrix into a string.
		@param builder The string builder that will contain the matrix.
			This string builder is not cleared of its contents.
		@param value The matrix to convert. */
	template <class CH>
	static FCOLLADA_EXPORT void ToString(FUStringBuilderT<CH>& builder, const FMMatrix44& value);

	/** Converts a 2D vector into a string.
		@param builder The string builder that will contain the 2D vector.
			This string builder is not cleared of its contents.
		@param value The 2D vector to convert. */
	template <class CH>
	static FCOLLADA_EXPORT void ToString(FUStringBuilderT<CH>& builder, const FMVector2& value);

	/** Converts a 3D vector into a string.
		@param builder The string builder that will contain the 3D vector.
			This string builder is not cleared of its contents.
		@param value The 3D vector to convert. */
	template <class CH>
	static FCOLLADA_EXPORT void ToString(FUStringBuilderT<CH>& builder, const FMVector3& value);

	/** Converts a 4D vector into a string.
		@param builder The string builder that will contain the 4D vector.
			This string builder is not cleared of its contents.
		@param p The 4D vector to convert. */
	template <class CH>
	static FCOLLADA_EXPORT void ToString(FUStringBuilderT<CH>& builder, const FMVector4& p);

	/** Split the target string into its pointer and its qualifier(s)
		Used by the animation system to split to find the animation targets */
	static void SplitTarget(const fm::string& target, fm::string& pointer, fm::string& qualifier);
	
	/** Parses the input string for a qualifier for the animation system.
		Basically extracts a number from either [XX] or (XX)
		@param string The input string to parse the number from
		@return The qualifier if found, -1 if not. */
	static int32 ParseQualifier(const char* string);
	static inline int32 ParseQualifier(const fm::string& string) { return ParseQualifier(string.c_str()); }

	/** Count the number of values to expect within this string.
		This function assumes that the tokens are separated by white spaces.
		This function is useful to pre-allocate value arrays.
		@param sz The string whose values to count.
		@return The expected number of values. */
	template <class CH>
	static FCOLLADA_EXPORT size_t CountValues(const CH* sz);
};

#ifdef __APPLE__
#include "FUtils/FUStringConversion.hpp"
#endif // __APPLE__

#endif // _FCU_STRING_CONVERSION_
