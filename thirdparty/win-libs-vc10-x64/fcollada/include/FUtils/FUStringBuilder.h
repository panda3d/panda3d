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
	@file FUStringBuilder.h
	This file contains the FUStringBuilderT template class,
	its defined template classes and its helper classes.
*/

#ifndef _FCU_STRING_BUILDER_
#define _FCU_STRING_BUILDER_

/**
	A dynamically-sized string object.
	The template has two arguments: the character definition and
	the sprintf() functor class for float to string conversions.

	This class should be used for all the string operations, as it contains a
	dynamically-resized buffer that is not directly tied to its content's length.

	@ingroup FUtils
*/
template <class Char>
class FCOLLADA_EXPORT FUStringBuilderT
{
private:
	Char* buffer;
	size_t reserved;
	size_t size;

public:
	/** The standard string object which correspond to the builder. */
	typedef fm::stringT<Char> String;

	/** Creates a new builder with the content of the given string.
		@param sz A string. Its content will be copied within the builder. */
	FUStringBuilderT(const String& sz);

	/** Creates a new builder with the content of the given character array.
		@param sz A character array. Its content will be copied within the builder.
			It must terminate with an element containing the 'zero' value. */
	FUStringBuilderT(const Char* sz);

	/** Creates a new builder with the given character repeated multiple times over the array.
		@param ch A character to repeat.
		@param count The number of times to repeat the given character. */
	FUStringBuilderT(Char ch, size_t count);

	/** Creates a new builder with an empty buffer. 
		@see reserve
		@param reserved The number of character slots to reserve within the empty buffer. */
	FUStringBuilderT(size_t reserved);

	/** Creates a new builder with an empty buffer. */
	FUStringBuilderT();

	/** Deletes the builder. Its buffer will be cleared. 
		Any pointers to its data will be dangling. */
	~FUStringBuilderT();

	/** Reserves a given number of character slots.
		If the builder has a buffer with a different number of character slots, a new
		buffer will be allocated. If the builder has contents, it will be copied within
		the new buffer. If there is more content than the new buffer can handle, it will
		be discarded.
		@param length The number of character slots to reserve. */
	void reserve(size_t length);

	/** Retrieves the length of the content within the builder.
		@return The length of the string. */
	inline size_t length() { return size; }

	/** Clears the content of the builder.
		This does not re-allocate a new buffer. */
	void clear();

	/** Retrieves whether the builder is empty.
		A builder is considered empty when it has no content, regardless of
		the size or allocation status of its buffer.
		@return Whether the builder is empty. */
	inline bool empty() const { return size == 0; }

	/** Appends a character to the content of the builder.
		@param c A character. May not be the 'zero' value. */
	void append(Char c);

	/** Appends a string to the content of the builder.
		@param sz A string. */
	void append(const String& sz);

	/** Appends a character array to the content of the builder.
		@param sz A character array. It must terminate with an
			element containing the 'zero' value. */
	void append(const Char* sz);

	/** Appends a character array to the content of the builder.
		@param sz A character array. It should not contain any 'zero'
			characters before 'len'
		@param len The number of characters to read from sz. */
	void append(const Char* sz, size_t len);

	/** Appends the content of a builder to the content of this builder.
		@param b A string builder. */
	void append(const FUStringBuilderT& b);

	/** Appends the integer value, after converting it to a string,
		to the content of the builder.
		@param i An integer value. */
	void append(int32 i);
	void append(uint32 i); /**< See above. */
	void append(uint64 i); /**< See above. */

	/** Appends the integer value, after converting it to a
		fm::string, in hexadecimal, to the content of the builder.
		The size of the integer will determine the number of characters used.
		@param i An unsigned integer value. */
	void appendHex(uint8 i);
	template <class T> inline void appendHex(const T& i) { for (size_t j = 0; j < sizeof(T); ++j) appendHex(*(((uint8*)&i) + j)); } /**< See above. */

#if defined(WIN32)
	inline void append(int i) { append((int32) i); } /**< See above. */
#ifdef _W64
	inline void append(_W64 unsigned int i) { append((uint32) i); } /**< See above. */
#else
	inline void append(unsigned int i) { append((uint32) i); } /**< See above. */
#endif
#else
	inline void append(unsigned long i) { append((uint32) i); } /**< See above. */
	inline void append(long i) { append((int32) i); } /**< See above. */
#endif // platform-switch.

	/** Appends the floating-point value, after converting it to a string,
		to the content of the builder. If the floating-point value is the special token
		that represents infinity, the string "INF" is appended. If it represents
		the negative infinity, the string "-INF" is appended. If it represents the
		impossibility, the string "NaN" is appended.
		@param f A floating-point value. */
	void append(float f);
	void append(double f); /**< See above. */

	/** Appends a vector to the content of the builder.
		@param v A vector. */
	void append(const FMVector2& v);
	void append(const FMVector3& v); /**< See above. */
	void append(const FMVector4& v); /**< See above. */

	/** Appends a value to the content of the builder.
		This is a shortcut for the append function.
		@see append
		@param val A value. This may be numerical, a character, a character array or a string. */
	template<typename TYPE> inline FUStringBuilderT& operator+=(const TYPE& val) { append(val); return *this; }

	/** Appends a character array to the content of the builder.
		A newline character will be appended after the character array.
		@param sz A character array. It must terminate with an
			element containing the 'zero' value. */
	void appendLine(const Char* sz);

	/** Removes a section of the content of the builder.
		Every character that occurs after the given index will be removed,
		resulting in a shrunk string.
		@param start An index within the content of the builder. */
	void remove(int32 start);

	/** Removes a section of the content of the builder.
		The substring defined by the 'start' and 'end' indices will be
		removed. The 'start' character is removed and is replaced by
		the 'end' character.
		@param start The index of the first character of the substring to remove.
		@param end The index of the first character after the removed substring. */
	void remove(int32 start, int32 end);

	/** Removes the last character of the content of the builder. */
	inline void pop_back() { if (size > 0) --size; }

	/** Sets the content of the builder to a given value.
		This clears the builder of all its content and appends the given value.
		@param val A value. This may be numerical, a character, a character array or a string. */
	template<typename TYPE> inline void set(const TYPE& val) { clear(); append(val); }
	template<typename TYPE> inline FUStringBuilderT& operator=(const TYPE& val) { clear(); append(val); return *this; } /**< See above. */

	/** Converts the content of the builder to a standard string.
		@return A string with the content of the builder. */
	String ToString() const;

	/** Converts the content of the builder to a character array.
		@return A character array with the content of the builder.
			This pointer is valid for the lifetime of the buffer of the builder, so
			do not keep it around. This character array should not be modified. */
	const Char* ToCharPtr() const;
	inline operator const Char*() const { return ToCharPtr(); } /**< See above. */

	/** Retrieves the index of the first character within the content of the builder
		that is equivalent to the given character.
		@param c The character to match.
		@return The index of the first equivalent character. -1 is returned if no
			character matches the given character. */
	int32 index(Char c) const;

	/** Retrieves the index of the last character within the content of the builder
		that is equivalent to the given character.
		@param c The character to match.
		@return The index of the last equivalent character. -1 is returned if no
			character matches the given character. */
	int32 rindex(Char c) const;

	/** Retrieves the last character within the content of the builder.
		@return The last character of the builder. */
	Char back() const { FUAssert(size > 0, return (Char) 0); return buffer[size-1]; }

private:
	void enlarge(size_t minimum);
};

typedef FUStringBuilderT<fchar> FUStringBuilder; /**< A Unicode string builder. */
typedef FUStringBuilderT<char> FUSStringBuilder;  /**< A 8-bit string builder. */

#if defined(__APPLE__)
#include "FUtils/FUStringBuilder.hpp"
#endif // __APPLE__

#endif // _FCU_STRING_BUILDER_

