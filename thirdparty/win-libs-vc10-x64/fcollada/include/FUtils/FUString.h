/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUString.h
	This file includes FUStringBuilder.h and FUStringConversion.h
	and defines important string-related macros and inline functions.
*/

#ifndef _FU_STRING_H_
#define _FU_STRING_H_

#ifndef _FM_ARRAY_H_
#include "FMath/FMArray.h"
#endif // _FM_ARRAY_H_

/** An empty UTF-8 string. This string is returned in many functions when there is an error. */
extern FCOLLADA_EXPORT const char* emptyCharString;
/** An empty Unicode string. This string is returned in many functions when there is an error. */
extern FCOLLADA_EXPORT const fchar* emptyFCharString;

// Already documented elsewhere.
namespace fm
{
	/** A string template.
		Intentionally has an interface similar to STL.
		You should use fm::string for UTF8 strings and
		fstring for UNICODE strings.
		@ingroup FUtils */
	template <class CH>
	class FCOLLADA_EXPORT stringT : public fm::vector<CH, true>
	{
	public:
		typedef fm::vector<CH, true> Parent; /**< The parent class. */

	public:
		/** Constant for infinity length.
			This important constant is used throughout the
			class to indicate infinity or values not found. */
		static const size_t npos = ~(size_t)0;

		/** Default constructor. Defaults to an empty string. */
		stringT() : Parent() {}

		/** Copy constructor.
			@param c The string to clone. */
		stringT(const stringT& c) : Parent(c) {}
		
		/** Copy constructor.
			@param c A NULL-terminated character buffer to clone. */
		stringT(const CH* c) : Parent()
		{
			append(c);
			if (c == NULL || (*c) == 0) push_back((CH) 0);
		}

		/** Copy constructor.
			@param c A character buffer to clone.
			@param length A partial length to copy.
				Use stringT::npos for copying full NULL-terminated strings. */
		stringT(const CH* c, size_t length) : Parent()
		{
			if (c == NULL || length == 0) return;
			
			if (length != npos)
			{
				Parent::resize(length + 1);
				memcpy(Parent::begin(), c, sizeof(CH) * length);
				Parent::back() = 0; // NULL-terminate.
			}
			else
			{
				append(c);
				if (c == NULL || (*c) == 0) push_back((CH) 0);
			}
		}

		/** Constructor.
			@param length The number of characters to repeat in the string.
			@param c The character value to repeat within the string. */
		stringT(size_t length, const CH& c) : Parent()
		{
			Parent::reserve(length + 1);
			Parent::insert(Parent::begin(), length, c);
			Parent::push_back((CH) 0);
		}

		/** Retrieves the length of the string.
			This function is NULL-termination aware.
			@return The length of the string. */
		inline size_t length() const { return Parent::size() > 1 ? Parent::size() - 1 : 0; }
		inline size_t size() const { return Parent::size() > 1 ? Parent::size() - 1 : 0; } /**< See above. */
		
		/** Retrieves the last element of the string.
			This function is NULL-termination aware.
			@return The last element of the string. */
		inline CH& back() { return *(Parent::end() - 2); }
		inline const CH& back() const { return *(Parent::end() - 2); } /**< See above. */

		/** Removes the last character from a string.
			This function is NULL-termination aware. */
		inline void pop_back() { if (Parent::size() > 0) { Parent::pop_back(); Parent::back() = 0; } }

		/** Retrieves whether the string contains useful data.
			This function differs from the parent because it checks for NULL-termination.
			@return Whether the string contains useful data. */
		inline bool empty() const { return Parent::size() <= 1; }

		/** Retrieves a segment of the string.
			@param start The index of the first character to extract.
			@param count The number of characters to extract. When the count
				is 'npos', all the remaining characters are extracted.
			@return The partial string. */
		stringT substr(size_t start, size_t count = npos) const
		{
			if (start >= length()) return stringT();
			if (count == npos || count + start > length()) count = length() - start;
			return stringT(c_str() + start, count);
		}

		/** Appends a string to this string.
			@param str A second string. */
		inline void append(const stringT& str)
		{
			insert(npos, str);
		}

		/** Appends a NULL-terminated character buffer to this string.
			@param str A NULL-terminated character buffer.
			@param count The number of characters to append. If the count
				is 'npos', all available characters are appended. */
		inline void append(const CH* str, size_t count=npos)
		{
			insert(npos, str, count);
		}

		/** Appends one character to this string.
			@param c A character. */
		void append(const CH& c)
		{
			if (c != 0)
			{
				size_t originalSize = length();
				Parent::resize(originalSize + 2);
				*(Parent::end() - 2) = c;
				Parent::back() = 0; // NULL-terminate
			}
		}

		/** Inserts a character buffer in this string.
			@param offset The position at which to insert the character buffer.
			@param str A NULL-terminated character buffer.
			@param count The number of characters to append. If the count
				is 'npos', all available characters are appended. */
		void insert(size_t offset, const CH* str, size_t count=npos)
		{
			if (str != NULL && (*str != 0))
			{
				size_t originalSize = length();
				offset = min(offset, originalSize);

				size_t str_length = 0;
				const CH* s = str;
				while(*s != 0 && str_length < count) { ++s; ++str_length; }
				resize(originalSize + str_length);
				if (offset < originalSize)
				{
					memmove(Parent::begin() + offset + str_length, Parent::begin() + offset, (originalSize - offset) * sizeof(CH));
				}
				memcpy(Parent::begin() + offset, str, sizeof(CH) * str_length);
				Parent::back() = 0; // NULL-terminate
			}
		}

		/** Inserts a string in this string.
			@param offset The position at which to insert the string.
			@param str A second string. */
		void insert(size_t offset, const stringT& str)
		{
			size_t str_length = str.length();
			if (str_length > 0)
			{
				size_t originalSize = length();
				offset = min(offset, originalSize);
				resize(originalSize + str_length);
				if (offset < originalSize)
				{
					memmove(Parent::begin() + offset + str_length, Parent::begin() + offset, (originalSize - offset) * sizeof(CH));
				}
				memcpy(Parent::begin() + offset, str.c_str(), sizeof(CH) * str_length);
			}
		}

		/** Retrieves the character buffer attached to this string.
			@return The NULL-terminated character buffer for this string. */
		const CH* c_str() const
		{
			static CH empty = 0;
			if (Parent::size() == 0) return &empty;
			return Parent::begin();
		}
		inline operator const CH*() const { return c_str(); } /**< See above. */

		/** Retrieves the position of the first matching character found within the string.
			@param character The character to match.
			@param offset An offset at which to start searching. Defaults to zero.
			@return The position of the first matching character. If 'npos' is returned, the
				character was not matched within the given (sub)string. */
		size_t find(const CH& character, size_t offset=0) const
		{
			if (character > 0)
			{
				for (const CH* it = Parent::begin() + offset; it < Parent::end(); ++it)
				{
					if ((*it) == character) return it - Parent::begin();
				}
			}
			return npos;
		}

		/** Retrieves the position of the first matching string found within the string.
			@param str The string to match. If the string is zero-terminated, the null
				character will not be included in the search.
			@param offset An offset at which to start searching. Defaults to zero.
			@return The position of the first matching string. If 'npos' is returned, the
				string was not matched within the given (sub)string. */
		size_t find(const stringT& str, size_t offset=0) const
		{
			if (str.length() > 0 && length() >= str.length())
			{
				CH firstMatch = str.front();
				const CH* end_fence = Parent::end() - str.size() + 1;
				for (const CH* it = Parent::begin() + offset; it < end_fence; ++it)
				{
					if ((*it) == firstMatch)
					{
						const CH* it2 = it;
						const CH* sit = str.begin();
						const CH* endIt = (*(str.end() - 1) == 0) ? str.end() - 1 : str.end();
						for (; it2 != Parent::end() && sit != endIt; ++sit, ++it2)
						{
							if ((*sit) != (*it2)) break;
						}
						if (sit == endIt) return it - Parent::begin();
					}
				}
			}
			return npos;
		}

		/** Retrieves the position of the first matching character buffer found within the string.
			@param c The character buffer to match.
			@param offset An offset at which to start searching. Defaults to zero.
			@return The position of the first matching character buffer. If 'npos' is returned, the
				character buffer was not matched within the given (sub)string. */
		size_t find(const CH* c, size_t offset=0) const
		{
			size_t length = 0; const CH* d = c; while (*d != 0) { ++length; ++d; }
			if (length > 0 && Parent::size() >= length)
			{
				const CH* end_fence = Parent::end() - length + 1;
				for (const CH* it = Parent::begin() + offset; it < end_fence; ++it)
				{
					if ((*it) == (*c))
					{
						const CH* it2 = it;
						for (d = c; it2 != Parent::end() && (*d) != 0; ++it2, ++d)
						{
							if ((*it2) != (*d)) break;
						}
						if (*d == 0) return it - Parent::begin();
					}
				}
			}
			return npos;
		}

		/** Retrieves the position of the last matching character found within the string.
			@param character The character to match.
			@param offset An offset at which to start searching. Defaults to zero.
			@return The position of the last matching character. If 'npos' is returned, the
				character was not matched within the given (sub)string. */
		size_t rfind(const CH& character, size_t offset=0) const
		{
			size_t ret = npos;
			if (character > 0)
			{
				for (const CH* it = Parent::begin() + offset; it < Parent::end(); ++it)
				{
					if ((*it) == character) ret = it - Parent::begin();
				}
			}
			return ret;
		}

		/** Retrieves the position of the last matching string found within the string.
			@param str The string to match. If the string is zero-terminated, the null
				character will not be included in the search.
			@param offset An offset at which to start searching. Defaults to zero.
			@return The position of the last matching string. If 'npos' is returned, the
				string was not matched within the given (sub)string. */
		size_t rfind(const stringT& str, size_t offset=0) const
		{
			size_t ret = npos;
			if (str.length() > 0 && length() >= str.length())
			{
				CH firstMatch = str.front();
				const CH* end_fence = Parent::end() - str.size() + 1;
				for (const CH* it = Parent::begin() + offset; it < end_fence; ++it)
				{
					if ((*it) == firstMatch)
					{
						const CH* it2 = it;
						const CH* sit = str.begin();
						const CH* endIt = (*(str.end() - 1) == 0) ? str.end() - 1 : str.end();
						for (; it2 != Parent::end() && sit != endIt; ++sit, ++it2)
						{
							if ((*sit) != (*it2)) break;
						}
						if (sit == endIt) ret = it - Parent::begin();
					}
				}
			}
			return ret;
		}

		/** Retrieves the position of the last matching character buffer found within the string.
			@param c The character buffer to match.
			@param offset An offset at which to start searching. Defaults to zero.
			@return The position of the last matching character buffer. If 'npos' is returned, the
				character buffer was not matched within the given (sub)string. */
		size_t rfind(const CH* c, size_t offset=0) const
		{
			size_t ret = npos;
			size_t length = 0; const CH* d = c; while (*d != 0) { ++length; ++d; }
			if (length > 0 && Parent::size() >= length)
			{
				const CH* end_fence = Parent::end() - length + 1;
				for (const CH* it = Parent::begin() + offset; it < end_fence; ++it)
				{
					if ((*it) == (*c))
					{
						const CH* it2 = it;
						for (d = c; it2 != Parent::end() && (*d) != 0; ++it2, ++d)
						{
							if ((*it2) != (*d)) break;
						}
						if (*d == 0) ret = it - Parent::begin();
					}
				}
			}
			return ret;
		}

		/** Retrieves the position of the first matching character from a list of possible characters.
			@param c A list of possible characters to match.
			@param offset An offset at which to start searching. Defaults to zero.
			@return The position of the first matching character within the list of possible characters.
				If 'npos' is returned, none of the possible characters were found within the (sub)string. */
		size_t find_first_of(const CH* c, size_t offset=0) const
		{
			size_t length = 0; const CH* d = c; while (*d != 0) { ++length; ++d; }
			if (length > 0 && Parent::size() >= length)
			{
				for (const CH* it = Parent::begin() + offset; it < Parent::end(); ++it)
				{
					d = c;
					while (*d != 0 && *d != *it) { ++d; }
					if (*d != 0) return it - Parent::begin();
				}
			}
			return npos;
		}

		/** Retrieves the position of the last matching character from a list of possible characters.
			@param c A list of possible characters to match.
			@param offset An offset (from the end of the string) at which to start searching. Defaults to zero.
			@return The position of the last matching character within the list of possible characters.
				If 'npos' is returned, none of the possible characters were found within the (sub)string. */
		size_t find_last_of(const CH* c, size_t offset=0) const
		{
			size_t ret = length() - offset; // UNOPTIMIZE. Didn't feel like this is an important-enough function.
			size_t length = 0; const CH* d = c; while (*d != 0) { ++length; ++d; }
			if (length > 0 && Parent::size() >= length)
			{
				const CH* end = Parent::end() - (length + offset);
				const CH* begin = Parent::begin();
				for (const CH* it = end; it >= begin; --it)
				{
					d = c;
					while (*d != 0) 
					{
						if (*d == *it) return ret;
						++d;
					}
					--ret;
				}
			}
			return npos;
		}
		
		/** Removes a range of characters from a string.
			@param start The start of the range of characters to remove.
				Defaults to zero.
			@param end The end of the range of characters to remove.
				The character at this position will not be removed so
				that if (start==end), no operation is done.
				Defaults to npos, which indicates that all characters from
				the start of the range should be removed. */
		void erase(size_t start=0, size_t end=npos)
		{
			if (start < length() && start < end) // covers size() == 0.
			{
				if (end > length()) end = length();
				Parent::erase(Parent::begin() + start, Parent::begin() + end);
			}
		}

		/** Override the fm::vector resize.  That call
			does not handle the assumption this class makes that
			every string is null terminated.
			
			The fm::string resize command will automatically create
			a null terminated string of length size
			@param size Length of resulting string */
		void resize(size_t size)
		{
			Parent::resize(size + 1);
			Parent::back() = 0; // NULL-terminate
		}

		/** Override the fm::vector resize.  That call
			does not handle the assumption this class makes that
			every string is null terminated.
			
			Sets the number of values contained in the list.
			@param count The new number of values contained in the list.
			@param value The value to assign to the new entries in the list. */
		void resize(size_t count, const CH& value)
		{
			Parent::resize(size + 1, value);
			Parent::back() = 0; // NULL-terminate
		}
	};

	/** A string of UTF8 characters. */
	typedef stringT<char> string;

	/** Concatenates two strings.
		@param A A first string.
		@param B A second string.
		@return The concatenation of the two strings. */
	template <class CharT> stringT<CharT> operator+(const stringT<CharT>& A, const stringT<CharT>& B) { stringT<CharT> C = A; C.append(B); return C; }
	template <class CharT> stringT<CharT> operator+(const CharT* A, const stringT<CharT>& B) { stringT<CharT> C = A; C.append(B); return C; } /**< See above. */
	template <class CharT> stringT<CharT> operator+(const stringT<CharT>& A, const CharT* B) { stringT<CharT> C = A; C.append(B); return C; } /**< See above. */
	template <class CharT> stringT<CharT>& operator+=(stringT<CharT>& A, const stringT<CharT>& B) { A.append(B); return A; } /**< See above. */
	template <class CharT> stringT<CharT>& operator+=(stringT<CharT>& A, const CharT* B) { A.append(B); return A; } /**< See above. */

	/** Appends a character to a string.
		@param A A string.
		@param B A character.
		@return The concatenation of the string with the character. */
	template <class CharT> stringT<CharT>& operator+=(stringT<CharT>& A, const CharT& B) { A.append(B); return A; }

	/** Retrieves whether a first string is lesser than a second string.
		This comparison is done solely on the character buffers and not the lengths.
		@param A A first string.
		@param B A second string.
		@return Whether the first string is lesser than the second string. */
	template <class CharT> bool operator<(const stringT<CharT>& A, const stringT<CharT>& B)
	{
		const CharT* a = A.c_str(); const CharT* b = B.c_str();
		while ((*a) != 0 && (*b) != 0 && (*a) == (*b)) { ++a; ++b; }
		return (*a) < (*b);
	}

	/** Retrieves whether a first string is equal to a second string.
		@param A A first string.
		@param B A second string.
		@return Whether the first string is equal to the second string. */
	template <class CharT> bool operator==(const stringT<CharT>& A, const stringT<CharT>& B)
	{
		if (A.length() != B.length()) return false;
		const CharT* a = A.c_str(); const CharT* b = B.c_str();
		while ((*a) != 0 && (*a) == (*b)) { ++a; ++b; }
		return (*a) == (*b);
	}

	/** Retrieves whether a first string differs from a second string.
		@param A A first string.
		@param B A second string.
		@return Whether the first string differs from the second string. */
	template <class CharT> bool operator!=(const stringT<CharT>& A, const stringT<CharT>& B)
	{
		if (A.length() != B.length()) return true;
		const CharT* a = A.c_str(); const CharT* b = B.c_str();
		while ((*a) != 0 && (*a) == (*b)) { ++a; ++b; }
		return (*a) != (*b);
	}

	/** Retrieves whether a first string differs from a second string.
		@param A A first string.
		@param B A second string.
		@return Whether the first string differs from the second string. */
	template <class CharT> bool operator!=(const stringT<CharT>& A, const CharT* B)
	{
		if (B == NULL) return true;
		size_t B_length = 0; { const CharT* b = B; while (*b != 0) { ++b; ++B_length; } }
		if (A.length() != B_length) return true;
		const CharT* a = A.c_str(); const CharT* b = B;
		while ((*a) != 0 && (*a) == (*b)) { ++a; ++b; }
		return (*a) != (*b);
	}
};

/** A string of UNICODE characters. */
typedef fm::stringT<fchar> fstring;

/** A dynamically-sized array of Unicode strings. */
typedef fm::vector<fstring> FStringList;

/** A dynamically-sized array of simple strings. */
typedef fm::vector<fm::string> StringList;

/** Returns whether two 8-bit strings are equivalent. This is a case-sensitive comparison.
	@param sz1 The first 8-bit string to compare.
	@param sz2 The second 8-bit string to compare.
	@return Whether the two 8-bit strings are equivalent. */
inline bool IsEquivalent(const char* sz1, const char* sz2) { return strcmp(sz1, sz2) == 0; }
inline bool IsEquivalent(const fm::string& sz1, const char* sz2) { return strcmp(sz1.c_str(), sz2) == 0; } /**< See above. */
inline bool IsEquivalent(const char* sz1, const fm::string& sz2) { return strcmp(sz1, sz2.c_str()) == 0; } /**< See above. */
inline bool IsEquivalent(const fm::string& sz1, const fm::string& sz2) { return strcmp(sz1.c_str(), sz2.c_str()) == 0; } /**< See above. */

/** Returns whether two 8-bit strings are equivalent. This is a case-insensitive comparison.
	@param sz1 The first 8-bit string to compare.
	@param sz2 The second 8-bit string to compare.
	@return Whether the two 8-bit strings are equivalent. */
inline bool IsEquivalentI(const char* sz1, const char* sz2) { return _stricmp(sz1, sz2) == 0; }
inline bool IsEquivalentI(const fm::string& sz1, const char* sz2) { return _stricmp(sz1.c_str(), sz2) == 0; } /**< See above. */
inline bool IsEquivalentI(const char* sz1, const fm::string& sz2) { return _stricmp(sz1, sz2.c_str()) == 0; } /**< See above. */
inline bool IsEquivalentI(const fm::string& sz1, const fm::string& sz2) { return _stricmp(sz1.c_str(), sz2.c_str()) == 0; } /**< See above. */
#ifdef UNICODE
inline bool IsEquivalentI(const fchar* sz1, const fchar* sz2) { return fstricmp(sz1, sz2) == 0; } /**< See above. */
inline bool IsEquivalentI(const fstring& sz1, const fchar* sz2) { return fstricmp(sz1.c_str(), sz2) == 0; } /**< See above. */
inline bool IsEquivalentI(const fchar* sz1, const fstring& sz2) { return fstricmp(sz1, sz2.c_str()) == 0; } /**< See above. */
inline bool IsEquivalentI(const fstring& sz1, const fstring& sz2) { return fstricmp(sz1.c_str(), sz2.c_str()) == 0; } /**< See above. */
#endif // UNICODE

/** Returns whether two 8-bit strings are equivalent. This is a case-sensitive comparison.
	@param sz1 The first 8-bit string to compare.
	@param sz2 The second 8-bit string to compare.
	@return Whether the two 8-bit strings are equivalent. */
inline bool operator==(const fm::string& sz1, const char* sz2) { return strcmp(sz1.c_str(), sz2) == 0; }

#ifdef UNICODE
/** Returns whether two Unicode strings are equivalent. This is a case-sensitive comparison.
	@param sz1 The first Unicode string to compare.
	@param sz2 The second Unicode string to compare.
	@return Whether the two Unicode strings are equivalent. */
inline bool IsEquivalent(const fchar* sz1, const fchar* sz2) 
{ 
	return (sz1 == sz2) ? true : // ptrs same, are equivalent
		(sz1 == NULL || sz2 == NULL) ? // either ptr null, not equivalent
			false : fstrcmp(sz1, sz2) == 0; // do actual test
}
inline bool IsEquivalent(const fstring& sz1, const fchar* sz2) { return IsEquivalent(sz1.c_str(), sz2); } /**< See above. */
inline bool IsEquivalent(const fchar* sz1, const fstring& sz2) { return IsEquivalent(sz1, sz2.c_str()); } /**< See above. */
inline bool IsEquivalent(const fstring& sz1, const fstring& sz2) { return IsEquivalent(sz1.c_str(), sz2.c_str()); } /**< See above. */

/** Returns whether two Unicode strings are equivalent. This is a case-sensitive comparison.
	@param sz1 The first Unicode string to compare.
	@param sz2 The second Unicode string to compare.
	@return Whether the two Unicode strings are equivalent. */
inline bool operator==(const fstring& sz1, const fchar* sz2) { return IsEquivalent(sz1.c_str(), sz2); }
#endif // UNICODE

// Include the main string modification classes.
#include "FUtils/FUStringBuilder.h"
#include "FUtils/FUStringConversion.h"

/** A Unicode string from a constant 8-bit string. */
#define FS(a) fstring(FC(a))
/** A Unicode string from any convertable value: string, vector-type or simple numeric. */
#define TO_FSTRING(a) FUStringConversion::ToFString(a)
/** An 8-bit string from any convertable value: Unicode string, vector-type or simple numeric. */
#define TO_STRING(a) FUStringConversion::ToString(a)

/** An empty UTF-8 string. This string is returned in many functions when there is an error. */
extern FCOLLADA_EXPORT const fm::string emptyString;
/** An empty Unicode string. This string is returned in many functions when there is an error. */
extern FCOLLADA_EXPORT const fstring emptyFString;

/** Returns whether a string builder and a string are equivalent. This is a case-sensitive comparison.
	@param builder The string builder to compare.
	@param sz The string to compare.
	@return Whether the two strings are equivalent. */
inline bool IsEquivalent(FUSStringBuilder& builder, const char* sz) { return IsEquivalent(builder.ToCharPtr(), sz); }
inline bool IsEquivalent(FUSStringBuilder& builder, const fm::string& sz) { return IsEquivalent(builder.ToCharPtr(), sz.c_str()); } /**< See above. */
#ifdef UNICODE
inline bool IsEquivalent(FUStringBuilder& builder, const fchar* sz) { return IsEquivalent(builder.ToCharPtr(), sz); } /**< See above. */
inline bool IsEquivalent(FUStringBuilder& builder, const fstring& sz) { return IsEquivalent(builder.ToCharPtr(), sz.c_str()); } /**< See above. */
#endif

#endif // _FU_STRING_H_
