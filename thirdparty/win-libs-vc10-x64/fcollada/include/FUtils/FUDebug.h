/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUDebug.h
	This file contains macros useful to write debugging output.
*/

#ifndef _FU_DEBUG_H_
#define _FU_DEBUG_H_

/**
	A debugging logging facility.
	Do not use this static class directly. Instead, use the macros listed below.
	This class is based on top of the FUError sinks.

	@see DEBUG_OUT WARNING_OUT ERROR_OUT FUError

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUDebug
{
private:
	/**	Block access to the constructors and destructors 
		as only static functions of the class will be used. */
	FUDebug();
	virtual ~FUDebug();

	/**	Outputs a string to the debug monitor.
		@param level The error level: FUError::DEBUG_LEVEL is recommended.
		@param message The formatted message to output. */
	static void DebugOut(uint8 level, const char* message, ...);
#ifdef UNICODE
	static void DebugOut(uint8 level, const fchar* message, ...); /**< See above. */
#endif // UNICODE

	/**	Outputs a string to the debug monitor.
		@param level The error level: FUError::DEBUG_LEVEL is recommended.
		@param message The formatted message to output.
		@param vars A standard variable argument list. */
	static void DebugOutV(uint8 level, const char* message, va_list& vars);
#ifdef UNICODE
	static void DebugOutV(uint8 level, const fchar* message, va_list& vars); /**< See above. */
#endif // UNICODE

	/**	Outputs a string to the debug monitor.
		@param level The error level: FUError::DEBUG_LEVEL is recommended.
		@param filename The filename of the code to trace.
		@param line The line number of the code trace.
		@param message The formatted message to output.
		@param vars A standard variable argument list. */
	static void DebugOutV(uint8 level, const char* filename, uint32 line, const char* message, va_list& vars);
#ifdef UNICODE
	static void DebugOutV(uint8 level, const char* filename, uint32 line, const fchar* message, va_list& vars); /**< See above. */
#endif // UNICODE

public:
	/**	Outputs a string to the debug monitor.
		@param level The error level: FUError::DEBUG_LEVEL is recommended.
		@param filename The filename of the code to trace.
		@param line The line number of the code trace.
		@param message The formatted message to output. */
	static void DebugOut(uint8 level, uint32 line, const char* filename, const char* message, ...);
#ifdef UNICODE
	static void DebugOut(uint8 level, uint32 line, const char* filename, const fchar* message, ...); /**< See above. */
#endif // UNICODE
};

/** Outputs a string to the debug monitor.
	At least one parameter must always be provided: the string to output, in UTF8.
	Any extra parameters will be used as tokens to a sprintf operation. */
#define DEBUG_OUT(...) FUDebug::DebugOut(FUError::DEBUG_LEVEL, __LINE__, __FILE__, __VA_ARGS__);
/** See above. */
#define WARNING_OUT(...) FUDebug::DebugOut(FUError::WARNING_LEVEL, __LINE__, __FILE__, __VA_ARGS__);
/** See above. */
#define ERROR_OUT(...) FUDebug::DebugOut(FUError::ERROR_LEVEL, __LINE__, __FILE__, __VA_ARGS__);

#endif // _FU_DEBUG_H_
