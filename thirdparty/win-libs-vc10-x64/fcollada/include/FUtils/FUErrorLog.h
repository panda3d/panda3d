/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUErrorLog.h
	This file contains the FUErrorLog class.
*/

#ifndef _FU_ERROR_LOG_H_
#define _FU_ERROR_LOG_H_

#ifndef _FU_ERROR_H_
#include "FUtils/FUError.h"
#endif // _FU_ERROR_H_

class FULogFile;

/**
	An error log.
	This class monitors the FUError sinks and outputs the wanted messages into a text file.
	@see FUError
	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUErrorLog
{
private:
	FULogFile* logFile;
	uint32 counts[FUError::LEVEL_COUNT];
	FUError::Level minimumLevel;

public:
	/** Constructor.
		@see FUError::Level
		@param logFilename The filename in which to output the error messages
			that cover at least the given errorLevel.
		@param errorLevel The minimum required error level. Any error messages
			lower that this level will be discarded. */
	FUErrorLog(const fchar* logFilename, FUError::Level errorLevel);

	/** Destructor.
		Closes the log file. */
	~FUErrorLog();

	/** Retrieves the log file handler.
		This can be useful when writing out long, custom messages.
		@return The log file handler. */
	FULogFile* GetLogFile() { return logFile; }

	/**	Allows to determine if new messages have been generated since the last query.
		@param debug The number of new debug-level messages. 
		@param warnings The number of new warning-level messages. 
		@param errors The number of new error-level messages. */
	void QueryNewMessages(uint32& debug, uint32& warnings, uint32& errors);

private:
	void OnErrorCallback(FUError::Level level, uint32 errorCode, uint32 argument);
};

#endif // _FU_ERROR_LOG_H_

