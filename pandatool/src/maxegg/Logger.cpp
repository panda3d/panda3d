#include "Logger.h"

/*	Globals & Static Members
 */

Logger *Logger::globalLoggingInstance = 0;

/*	Error Logger Member Functions 
 */

Logger::Logger()
{
	VoidEverything();
	LogInstance( LLOGGING, SAT_MEDIUM_LEVEL, "A new, void Logging instance has been created." );
}

Logger::Logger( LoggingPipeType toWhere, char *additionalStringInfo )
{
	VoidEverything();
	SetPipeInstance( toWhere, additionalStringInfo );
	
	sprintf( GetLogString(), "A new, piped logging instance has been created with data '%s'.", additionalStringInfo );
	LogInstance( LLOGGING, SAT_MEDIUM_LEVEL, GetLogString() );
}

Logger::~Logger()
{
	//Send message telling everyone we're going away.
	LogInstance( LLOGGING, SAT_MEDIUM_LEVEL, "Error logger shutting down!" );
	//If we've got an open file, close that muthafugga!
	if ( ( myLogDestination == PIPE_TO_FILE ) && myFileOutputLog.is_open() )
		myFileOutputLog.close();
	VoidEverything();
}

/*	Public, Static Member Functions
 */

void Logger::FunctionEntry( char *newFunctionName )
{
	if ( !globalLoggingInstance )
		globalLoggingInstance = new Logger();
	if ( globalLoggingInstance )
		globalLoggingInstance->FunctionEntryInstance( newFunctionName );
}

void Logger::FunctionExit()
{
	if ( !globalLoggingInstance )
		globalLoggingInstance = new Logger();
	if ( globalLoggingInstance )
		globalLoggingInstance->FunctionExitInstance();
}

int Logger::GetHierarchyLevel()
{
	if ( !globalLoggingInstance )
		globalLoggingInstance = new Logger();
	if ( globalLoggingInstance )
		return globalLoggingInstance->GetHierarchyLevelInstance();
	else return 0;
}

char * Logger::GetLogString()
{
	if ( !globalLoggingInstance )
		globalLoggingInstance = new Logger();
	if ( globalLoggingInstance )
		return globalLoggingInstance->GetLogStringInstance();
	else return 0;
}

void Logger::Log( SystemType whichSystem, SystemAspectType whichErrorKind, char *errorDescription )
{
	if ( !globalLoggingInstance )
		globalLoggingInstance = new Logger();
	if ( globalLoggingInstance )
		globalLoggingInstance->LogInstance( whichSystem, whichErrorKind, errorDescription );
}

void Logger::SetCurrentFunctionName( char *newFunctionName )
{
	if ( !globalLoggingInstance )
		globalLoggingInstance = new Logger();
	if ( globalLoggingInstance )
		globalLoggingInstance->SetCurrentFunctionNameInstance( newFunctionName );
}

void Logger::SetHierarchyLevel( unsigned int newIndentLevel )
{
	if ( !globalLoggingInstance )
		globalLoggingInstance = new Logger();
	if ( globalLoggingInstance )
		globalLoggingInstance->SetHierarchyLevelInstance( newIndentLevel );
}

void Logger::SetOneErrorMask( SystemType whichType, long int whichErrors )
{
	if ( !globalLoggingInstance )
		globalLoggingInstance = new Logger();
	if ( globalLoggingInstance )
		globalLoggingInstance->SetOneErrorMaskInstance( whichType, whichErrors );
}

void Logger::SetPipe( LoggingPipeType toWhere, char *additionalStringInfo )
{
	if ( !globalLoggingInstance )
		globalLoggingInstance = new Logger();
	if ( globalLoggingInstance )
		globalLoggingInstance->SetPipeInstance( toWhere, additionalStringInfo );
}

/*	Private Member Functions
 */

void Logger::FunctionEntryInstance( char *newFunctionName )
{
	SetCurrentFunctionNameInstance( newFunctionName );
	SetHierarchyLevelInstance( GetHierarchyLevelInstance() + 1 );
}

void Logger::FunctionExitInstance()
{
	char endMsg[64];

	SetHierarchyLevelInstance( GetHierarchyLevelInstance() - 1 );
	if ( myFunctionNames.back() )
	{
		if ( myWrittenHierarchyLevel >= myHierarchyLevel )
		{
			sprintf( endMsg, "#END {%s}", myFunctionNames.back() );
			WriteToPipe( endMsg );
			//LogInstance( LLOGGING, this->SAT_HIGH_LEVEL, GetLogStringInstance());
			--myWrittenHierarchyLevel;
		}
		free( (void *)myFunctionNames.back() );
		myFunctionNames.pop_back();
	}
}

int Logger::GetHierarchyLevelInstance()
{
	return myHierarchyLevel;
}

char * Logger::GetLogStringInstance()
{
	return myLogString;
}

void Logger::LogInstance( SystemType whichSystem, SystemAspectType whichErrorKind, char *errorDescription )
{
	unsigned int i;
	char *typeBuf;
	char beginMsg[64];

	if ( !errorDescription )
		return;
	if ( !( (int)whichErrorKind & myErrorMasks[(int)whichSystem] ) )
		return;
	typeBuf = (char *)malloc( strlen( errorDescription ) + 64 );
	if ( !typeBuf )
		return;
	typeBuf = strcpy( typeBuf, errorDescription );
	switch( whichErrorKind )
	{
	case SAT_NONE:
		break;
	case SAT_NULL_ERROR:
		strcat( typeBuf, " - (***!!!NULL ERROR!!!***, " );
		break;
	case SAT_CRITICAL_ERROR: 
		strcat( typeBuf, " - (***!!!CRITICAL ERROR!!!***, " );
		break;
	case SAT_PARAMETER_INVALID_ERROR:
		strcat( typeBuf, " - (***PARAMETER ERROR***, " );
		break;
	case SAT_OTHER_ERROR:
		strcat( typeBuf, " - (***OTHER ERROR***, " );
		break;
	case SAT_HIGH_LEVEL:
		strcat( typeBuf, " - (---HIGH LEVEL---, " );
		break;
	case SAT_MEDIUM_LEVEL:
		strcat( typeBuf, " - (MEDIUM LEVEL, " );
		break;
	case SAT_LOW_LEVEL: 
		strcat( typeBuf, " - (LOW LEVEL, " );
		break;
	case SAT_DEBUG_SPAM_LEVEL:
		strcat( typeBuf, " - (SPAM LEVEL, " );
		break;
	case SAT_ALL:
		strcat( typeBuf, " - (ALL INCLUSIVE, " );
		break;
	}
	switch( whichSystem )
	{
	case ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM1: 
		strcat( typeBuf, "SYS_ONE)" );
		break;
	case ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM2: 
		strcat( typeBuf, "SYS_TWO)" );
		break;
	case ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM3:  
		strcat( typeBuf, "SYS_THREE)" );
		break;
	case ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM4: 
		strcat( typeBuf, "SYS_FOUR)" );
		break;
	case ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM5: 
		strcat( typeBuf, "SYS_FIVE)" );
		break;
	case ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM6: 
		strcat( typeBuf, "SYS_SIX)" );
		break;
	case ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM7: 
		strcat( typeBuf, "SYS_SEVEN)" );
		break;
	case ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM8: 
		strcat( typeBuf, "SYS_EIGHT)" );
		break;
	}
	
	//Now that we've created the correct logging line to print, we need to worry
	//about function entries and exits. Only do this if we're not writing to a dialog box.
	if ( myLogDestination != PIPE_TO_DIALOG_BOX )
	{
		unsigned int tempHierarchyLevel = myHierarchyLevel;

		i = 0;
		for( CharStarVectorIterator hierarchyDepthIterator = myFunctionNames.begin(); 
			hierarchyDepthIterator != myFunctionNames.end(); 
			++hierarchyDepthIterator )
		{
			++i;
			//Since we're writing some output, we need to print all function headings
			//leading up to this output, bumping up our written hierarchy level to match
			//the "actual" level. 
			//If we've reached a function level that's deeper than what we've written out already...
			if ( i > myWrittenHierarchyLevel )
			{
				myHierarchyLevel = myWrittenHierarchyLevel;
				sprintf( beginMsg, "#BEGIN {%s}", *hierarchyDepthIterator );
				WriteToPipe( beginMsg );	
				myHierarchyLevel = tempHierarchyLevel;
				++myWrittenHierarchyLevel;
			}
		}
	}
	WriteToPipe( typeBuf );
	free(typeBuf);
}

void Logger::SetCurrentFunctionNameInstance( char *newFunctionName )
{
	char *newBuf;

	//No FunctionEntry Instance allowed...that function uses this!
	if ( !newFunctionName )
	{
		LogInstance( LLOGGING, SAT_NULL_ERROR, "newFunctionName is null!" );
		return;
	}
	newBuf = strdup( newFunctionName );
	myFunctionNames.push_back( newBuf );
}

void Logger::SetHierarchyLevelInstance( unsigned int newIndentLevel )
{
	myHierarchyLevel = newIndentLevel;
}

void Logger::SetOneErrorMaskInstance( SystemType whichType, long int whichErrors )
{
	if ( ( (int)whichType < 0 ) || ( (int)whichType >= MY_MAX_NUM_SYSTEMS ) )
	{
		LogInstance( LLOGGING, SAT_PARAMETER_INVALID_ERROR, "whichType is out of bounds!" );
		return;
	}
	//Now that the sanity check is out of the way, let us change the error mask!
	myErrorMasks[(int)whichType] = whichErrors;
	sprintf( GetLogStringInstance(), "Set error mask for system with ID %x to %x.", (int)whichType, whichErrors );
	LogInstance( LLOGGING, SAT_LOW_LEVEL, GetLogStringInstance() );
}

void Logger::SetPipeInstance( LoggingPipeType toWhere, char *additionalStringInfo )
{
	myLogDestination = toWhere;
	switch( myLogDestination )
	{
	case PIPE_TO_FILE:
		if ( myFileOutputLog.is_open() )
			myFileOutputLog.close();
		if ( additionalStringInfo )
			myFileOutputLog.open( additionalStringInfo, ofstream::out | ofstream::trunc );
		else
			myFileOutputLog.open( "Kens_Logger_Log_File.txt", ofstream::out | ofstream::trunc );
		LogInstance( LLOGGING, SAT_LOW_LEVEL, "Error output piped to file." );
		break;
	case PIPE_TO_COUT:
		LogInstance( LLOGGING, SAT_LOW_LEVEL, "Error output piped to cout." );
		break;
	case PIPE_TO_CERR:
		LogInstance( LLOGGING, SAT_LOW_LEVEL, "Error output piped to cerr." );
		break;
	case PIPE_TO_DIALOG_BOX:
		LogInstance( LLOGGING, SAT_LOW_LEVEL, "Error output piped to dialog box." );
		break;
	case PIPE_TO_DEV_NULL:
		LogInstance( LLOGGING, SAT_LOW_LEVEL, "Error output piped to dev null." );
		break;
	}
}

void Logger::VoidEverything()
{
	myLogDestination = PIPE_TO_DEV_NULL;
	myHierarchyLevel = 0;
	myWrittenHierarchyLevel = 0;
	//(Get rid of the stack of called functions.)
	for (CharStarVectorIterator it = myFunctionNames.begin(); it < myFunctionNames.end(); it++) {
	  if (*it)
		 free ((void *)(*it));
	}
	myFunctionNames.erase( myFunctionNames.begin(), myFunctionNames.end() );
	//Make it so that our logger blindly accepts all logs...
	for( int i = 0; i < MY_MAX_NUM_SYSTEMS; ++i )
		SetOneErrorMaskInstance( (SystemType)i, (long int)SAT_ALL );
	//...but pipes them all to /dev/null. Mwahaha! The irony!
	strncpy( myLogString, "No Error", LOGGER_STRING_BUFFER_SIZE - 1 );
}

void Logger::WriteToPipe( char *textToPipe )
{
	switch( myLogDestination )
	{
	case PIPE_TO_FILE:
		if ( myFileOutputLog.is_open() )
		{
			for ( int i = 0; i < myHierarchyLevel; ++i )
				myFileOutputLog << "  ";
			myFileOutputLog << textToPipe << endl;
			myFileOutputLog.flush();
		}
		break;
	case PIPE_TO_COUT:
		cout << "("  << textToPipe << ")" << endl;
		break;
	case PIPE_TO_CERR:
		cerr << "(" << textToPipe << ")" << endl;
		break;
	case PIPE_TO_DIALOG_BOX:
		MessageBox( NULL, textToPipe, "Logger", MB_OK );
		break;
	case PIPE_TO_DEV_NULL:
		break;
	}
}










