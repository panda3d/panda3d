#ifndef VRPN_LOG_H
#define VRPN_LOG_H

/**
 * @class vrpn_Log
 * Logs a VRPN stream.
 * Used by vrpn_Endpoint.
 */

class VRPN_API vrpn_Log {

  public:

    vrpn_Log (vrpn_TranslationTable * senders,
              vrpn_TranslationTable * types);
    ~vrpn_Log (void);

    // ACCESSORS
	char* getName( );
	 ///< Allocates a new string and copies the log file name to it.
	///< IMPORTANT:  code calling this function is responsible for freeing the memory.

    // MANIPULATORS
    int open (void);
      ///< Opens the log file.

    int close (void);
      ///< Closes and saves the log file.

    int saveLogSoFar(void);
      ///< Saves any messages logged so far.

    int logIncomingMessage (vrpn_int32 payloadLen, struct timeval time,
                    vrpn_int32 type, vrpn_int32 sender, const char * buffer);
      ///< Should be called with the timeval adjusted by the clock offset
      ///< on the receiving Endpoint.

    int logOutgoingMessage (vrpn_int32 payloadLen, struct timeval time,
                    vrpn_int32 type, vrpn_int32 sender, const char * buffer);

    int logMessage (vrpn_int32 payloadLen, struct timeval time,
                    vrpn_int32 type, vrpn_int32 sender, const char * buffer,
                    vrpn_bool isRemote = VRPN_FALSE);
      ///< We'd like to make this protected, but there's one place it needs
      ///< to be exposed, at least until we get cleverer.


    int setCookie (const char * cookieBuffer);
      ///< The magic cookie is set to the default value of the version of
      ///< VRPN compiled, but a more correct value to write in the logfile
      ///< (if we're logging incoming messages) is that of the version of
      ///< VRPN we're communicating with.


    int setCompoundName (const char * name, int index);
      ///< Takes a name of the form foo.bar and an index <n> and sets the
      ///< name of the log file to be foo-<n>.bar;  if there is no period
      ///< in the name, merely appends -<n>.

    int setName (const char * name);
    int setName (const char * name, int len);

    long & logMode (void);
      ///< Returns a reference so we can |= it.

    int addFilter (vrpn_LOGFILTER filter, void * userdata);

    timeval lastLogTime ();
      ///< Returns the time of the last message that was logged

  protected:

    int checkFilters (vrpn_int32 payloadLen, struct timeval time,
                      vrpn_int32 type, vrpn_int32 sender, const char * buffer);

    char * d_logFileName;
    long d_logmode;

    vrpn_LOGLIST * d_logTail;
    vrpn_LOGLIST * d_firstEntry;

    FILE * d_file;

    char * d_magicCookie;

    vrpn_bool d_wroteMagicCookie;

    vrpnLogFilterEntry * d_filters;

    vrpn_TranslationTable * d_senders;
    vrpn_TranslationTable * d_types;

    timeval d_lastLogTime;
};



#endif  // VRPN_LOG_H

