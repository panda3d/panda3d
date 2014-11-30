/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	This file was mostly inspired by source code found at:
	http://www.codeguru.com/cpp/cpp/algorithms/article.php/c5099/
*/

/**
	@file FUBase64.h
	This file contains the FUBase64 namespace, exposing methods to encode and
	decode string to and from the Base64 format.
*/

#ifndef _FUTILS_BASE64_H_
#define _FUTILS_BASE64_H_

namespace FUBase64
{
	/** Encodes the given input string in Base64, and stores the result in
		the output string.
		@param input The string to encode.
		@param output The encoded output.*/
	FCOLLADA_EXPORT void encode(const UInt8List& input, UInt8List& output);

	/** Decodes the given input string from the Base64 format, and stores
		the result in the output string. This method will decode the input string
		as long as it is in Base64 format.
		@param input The string to decode.
		@param output The decoded output.*/
	FCOLLADA_EXPORT void decode(const UInt8List& input, UInt8List& output);

} // namespace FUBase64

#endif // _FUTILS_BASE64_H_
