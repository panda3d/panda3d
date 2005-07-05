#ifndef __maxLogger__H
#define __maxLogger__H

/*	Standard C++ Includes for file and stream output
 */

//For file IO and cmd line output
#include <iostream>
#include <fstream>

//For MessageBox
#include "windows.h"

#define MY_MAX_NUM_SYSTEMS 8
#define LOGGER_STRING_BUFFER_SIZE 128

#include <vector>
using namespace std;

/*	Vector definitions
 */

typedef vector<char *> CharStarVector;
typedef CharStarVector::iterator CharStarVectorIterator;

/*	Class Defintions
 */

class Logger
{
public:
	enum SystemAspectType
	{
		SAT_NONE = 0x0000,
		SAT_NULL_ERROR = 0x0001, 
		SAT_CRITICAL_ERROR = 0x0002, 
		SAT_PARAMETER_INVALID_ERROR = 0x0004, 
		SAT_OTHER_ERROR = 0x0008,
		SAT_HIGH_LEVEL = 0x0010, 
		SAT_MEDIUM_LEVEL = 0x0020, 
		SAT_LOW_LEVEL = 0x0040, 
		SAT_DEBUG_SPAM_LEVEL = 0x0080,
		SAT_ALL = 0x00FF
	};

	enum SystemType
	{
		ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM1 = 0x0000, 
		ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM2 = 0x0001, 
		ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM3 = 0x0002, 
		ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM4 = 0x0003,
		ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM5 = 0x0004, 
		ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM6 = 0x0005, 
		ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM7 = 0x0006, 
		ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM8 = 0x0007		
	};

	enum LoggingPipeType
	{
		PIPE_TO_FILE, PIPE_TO_COUT, PIPE_TO_CERR, PIPE_TO_DIALOG_BOX, PIPE_TO_DEV_NULL
	};

private:
	//Which errors to display
	long int			myErrorMasks[MY_MAX_NUM_SYSTEMS];
	//For use when the the logger is set to pipe to a file.
	ofstream			myFileOutputLog;
	//The stack of called functions.
	CharStarVector		myFunctionNames;
	//For formatting purposes, in the file, cerr, or cout versions, add n whitespace to the front, where n is this.
	unsigned int		myHierarchyLevel;
	//A memory of which state we're in, as far as output is concerned
	LoggingPipeType		myLogDestination;
	//A pre-allocated string to use with sprintf, or when error logging just needs a little scratch space.
	char				myLogString[LOGGER_STRING_BUFFER_SIZE];
	//An integer that keeps track of how far down into the indent hierarchy we've actually written.
	unsigned int		myWrittenHierarchyLevel;
	
public:
	//A static pointer to an active errorLogger that any class can get to.
	static Logger *globalLoggingInstance;

	//Constructors & Destructor
	Logger();
	Logger( LoggingPipeType toWhere, char *additionalStringInfo );
	~Logger();
	//Static functions that constitute the main interface to this class.
	static void		FunctionEntry( char *newFunctionName );
	static void		FunctionExit();
	static int		GetHierarchyLevel();
	static char *	GetLogString();
	static void		Log( SystemType whichSystem, SystemAspectType whichErrorKind, char *errorDescription );
	static void		SetCurrentFunctionName( char *newFunctionName );
	static void		SetHierarchyLevel( unsigned int newIndentLevel );
	static void		SetOneErrorMask( SystemType whichType, long int whichErrors );
	static void		SetPipe( LoggingPipeType toWhere, char *additionalStringInfo );
	
private:
	//Private functions called by the static versions if a globalLogging instance exists.
	void			FunctionEntryInstance( char *newFunctionName );
	void			FunctionExitInstance();
	int				GetHierarchyLevelInstance();
	char *			GetLogStringInstance();
	void			LogInstance( SystemType whichSystem, SystemAspectType whichErrorKind, char *errorDescription );
	void			SetCurrentFunctionNameInstance( char *newFunctionName );
	void			SetHierarchyLevelInstance( unsigned int newIndentLevel );
	void			SetOneErrorMaskInstance( SystemType whichType, long int whichErrors );
	void			SetPipeInstance( LoggingPipeType toWhere, char *additionalStringInfo );
	void			VoidEverything();
	void			WriteToPipe( char *textToPipe );
};

/*	Subsystem defs for logger.
 */

#define LLOGGING Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM1

/*	Externed Globals
 */

#endif
