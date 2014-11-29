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
	@file FUCrc32.h
	This file contains the CRC-32 hashing functions.
*/

#ifndef _FU_CRC32_H_
#define _FU_CRC32_H_

/**
	CRC-32 hashing functions.
	CRC-32 is a commonly used hashing mechanism for strings.

	@ingroup FUtils
*/
namespace FUCrc32
{
	/** A CRC32 hash value. */
	typedef uint32 crc32;

	/** Hashes a string.
		@param text The string to hash.
		@return The 32-bit hash value. */
	FCOLLADA_EXPORT crc32 CRC32(const char* text);
#ifdef UNICODE
	FCOLLADA_EXPORT crc32 CRC32(const fchar* text); /**< See above. */
#endif // UNICODE
};

#endif // _FU_CRC32_H_

