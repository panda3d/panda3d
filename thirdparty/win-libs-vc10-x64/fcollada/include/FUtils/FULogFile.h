/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FULogFile.h
	This file contains the FULogFile class.
*/

#ifndef _FU_LOG_FILE_H_
#define _FU_LOG_FILE_H_

class FUFile;

/**
	A log file.
	This class simply makes writing out text file easier.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FULogFile
{
private:
	FUFile* file;

public:
	/** Constructor.
		This opens, for writing, the file with the given filename.
		Relative filepaths are recommended for log files.
		@param filename The log filename. */
	FULogFile(const fchar* filename);

	/** Destructor. */
	~FULogFile();

	/** Returns the underlying FUFile.
		@return The FUFile.*/
	inline FUFile* GetFile() { return file; }
	inline const FUFile* GetFile() const { return file; } /**< See above.*/

	/** Writes out a log message.
		This version of this function is useful for code-base assertions and
		error messages aimed at programmers.
		@param filename A filename.
		@param linenum A line number.
		@param message The message to write out. */
	void WriteLine(const char* filename, uint32 linenum, const char* message, ...);
#ifdef UNICODE
	void WriteLine(const char* filename, uint32 line, const fchar* message, ...); /**< See above. */
#endif // UNICODE

	/** Writes out a log message.
		This version of this function is useful for messages aimed at users.
		@param message The message to write out. */
	void WriteLine(const char* message, ...);
#ifdef UNICODE
	void WriteLine(const fchar* message, ...); /**< See above. */
#endif // UNICODE

	/** Writes out a log message.
		The variable-list version of this function.
		@param message The message to write out.
		@param vars A variable list object. */
	void WriteLineV(const char* message, va_list& vars);
#ifdef UNICODE
	void WriteLineV(const fchar* message, va_list& vars); /**< See above. */
#endif // UNICODE

	/** Flushes the log file. */
	void Flush();
};

#endif // _FU_LOG_FILE_H_
