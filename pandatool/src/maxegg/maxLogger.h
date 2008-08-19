#ifndef __maxLogger__H
#define __maxLogger__H

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

public:
        Logger() {} 
        Logger( LoggingPipeType toWhere, char *additionalStringInfo ) { }
        ~Logger() { }
        static void             FunctionEntry( char *newFunctionName ) { }
        static void             FunctionExit() { }
        static int              GetHierarchyLevel() { }
        static char *   GetLogString() { return dummy; }
        static void             Log( SystemType whichSystem, SystemAspectType whichErrorKind, char *errorDescription ) { }
        static void             SetCurrentFunctionName( char *newFunctionName ) { }
        static void             SetHierarchyLevel( unsigned int newIndentLevel ) { }
        static void             SetOneErrorMask( SystemType whichType, long int whichErrors ) { }
        static void             SetPipe( LoggingPipeType toWhere, char *additionalStringInfo ) { }

        static char dummy[65536];

};

#endif
