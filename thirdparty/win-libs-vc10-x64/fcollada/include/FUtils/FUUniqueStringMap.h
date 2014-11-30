/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUUniqueStringMap.h
	This file contains the FUUniqueStringMapT template.
*/

#ifndef _FU_UNIQUE_ID_MAP_H_
#define _FU_UNIQUE_ID_MAP_H_

/**
	A set of unique strings.
	This class adds three functions to the STL map in order to
	keep the strings inside unique: AddUniqueString, Exists and Erase.
	
	@ingroup FUtils
*/
template <class CH>
class FCOLLADA_EXPORT FUUniqueStringMapT
{
private:
	typedef fm::map<uint32, uint32> NumberMap; // This is really a set and the second uint32 is not used.
	typedef fm::map<fm::stringT<CH>, NumberMap> StringMap;

	StringMap values;

public:
	/** Adds a string to the map.
		If the string isn't unique, it will be modified in order to make it unique.
		@param wantedStr The string to add. This reference will be directly
			modified to hold the actual unique string added to the map. */
	void insert(fm::stringT<CH>& wantedStr);
	void insert(const fm::stringT<CH>& wantedStr) { fm::stringT<CH> a = wantedStr; insert(a); } /**< See above. */

	/** Retrieves whether a given string is contained within the map.
		@param str The string. */
	bool contains(const fm::stringT<CH>& str) const;

	/** Erases a string from the map.
		@param str A string contained within the map. */
	void erase(const fm::stringT<CH>& str);
};

typedef FUUniqueStringMapT<char> FUSUniqueStringMap; /**< A map of unique UTF-8 strings. */
typedef FUUniqueStringMapT<fchar> FUUniqueStringMap; /**< A map of unique Unicode strings. */

#endif // _FU_UNIQUE_ID_MAP_H_

