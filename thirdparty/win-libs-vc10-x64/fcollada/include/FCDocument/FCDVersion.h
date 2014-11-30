/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDVersion.h
	This file declares the FCDVersion class.
*/

#ifndef _FCD_VERSION_H_
#define _FCD_VERSION_H_

#ifdef major
#undef major
#endif
#ifdef minor
#undef minor
#endif

/**
	A COLLADA document verison.
	Format is major.minor.revision.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDVersion
{
public:
	uint32 major; /**< The major release number of the version. */
	uint32 minor; /**< The minor release number of the version. */
	uint32 revision; /**< The revision number of the release. */

	/** Default Constructor. Leaves all the numbers at zero. */
	FCDVersion();

	/** Constructs a version number structure from a string.
		@param v A string containing the version numbers. */
	FCDVersion(const fm::string& v);

	/** Constructs a version number structure from a series of numbers.
		@param major The major version number.
		@param minor The minor version number.
		@param revision The revision version number. */
	FCDVersion(uint32 major, uint32 minor, uint32 revision);

	/** Destructor. */
	~FCDVersion() {}

	/** Extracts the version number from a string in the form of
		major.minor.revision (ex: 1.4.1)
		@param v A string containing the version numbers */
	void ParseVersionNumbers(const fm::string& v);

	/** Returns whether two versions are equivalent.
		@param a A first version.
		@param b A second version.
		@return Whether the two versions are equivalent. */
	friend FCOLLADA_EXPORT bool IsEquivalent(const FCDVersion& a, const FCDVersion& b);

	/** Returns whether this version is older than a second version.
		@param b A second version.
		@return Whether this version is older than the second version. */
	bool operator< (const FCDVersion& b) const;

	/** Returns whether this version is older than or equal to a second version.
		@param b A second version.
		@return Whether this version is older than or equal to a second version. */
	bool operator<= (const FCDVersion& b) const;

	/** Returns whether this version is newer than a second version.
		@param b A second version.
		@return Whether this version is newer than the second version. */
	inline bool operator> (const FCDVersion& b) const { return b <= *this; }

	/** Returns whether this version is newer or equal to a second version.
		@param b A second version.
		@return Whether this version is newer or equal to a second version. */
	inline bool operator>= (const FCDVersion& b) const { return b < *this; }
};

#endif // _FCD_VERSION_H_
