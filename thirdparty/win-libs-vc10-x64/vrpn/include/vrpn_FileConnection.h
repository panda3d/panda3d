#ifndef VRPN_FILE_CONNECTION_H
#define VRPN_FILE_CONNECTION_H

// {{{ vrpn_File_Connection
//
// Tom Hudson, June 1998

// This class *reads* a file written out by vrpn_Connection's logging hooks.

// The interface exactly matches that of vrpn_Connection.  To do things that
// are meaningful on log replay but not on live networks, create a
// vrpn_File_Controller and pass your vrpn_File_Connection to its constructor,
// or just ask the Connection for its file connection pointer and do the
// operations directly on the FileConnection if the pointer is non-NULL.

// Logfiles are recorded as *sent*, not as translated by the receiver,
// so we still need to have all the correct names for senders and types
// registered.

// September 1998:  by default preloads the entire log file on startup.
// This causes a delay (nontrivial for large logs) but should help smooth
// playback.
// }}}

#include "vrpn_Connection.h"

// Global variable used to indicate whether File Connections should
// pre-load all of their records into memory when opened.  This is the
// default behavior, but fails on very large files that eat up all
// of the memory.  This defaults to "true".  User code should set this
// to "false" before calling vrpn_get_connection_by_name() or creating
// a new vrpn_File_Connection object if it wants that file connection
// to not preload.  The value is only checked at connection creation time;
// the connection behaves consistently once created.  This operation is
// useful for applications that load large data files and don't want to
// wait for them to pre-load.

extern VRPN_API bool vrpn_FILE_CONNECTIONS_SHOULD_PRELOAD;

// Global variable used to indicate whether File Connections should
// keep already-read messages stored in memory.  If not, then we have
// to re-load the file starting at the beginning on rewind.  This
// defaults to "true".  User code should set this
// to "false" before calling vrpn_get_connection_by_name() or creating
// a new vrpn_File_Connection object if it wants that file connection
// to not preload.  The value is only checked at connection creation time;
// the connection behaves consistently once created.  This operation is
// useful for applications that read through large data files and
// don't have enough memory to keep them in memory at once, or for applications
// that read through only once and have no need to go back and check.

extern VRPN_API bool vrpn_FILE_CONNECTIONS_SHOULD_ACCUMULATE;

// Global variable used to indicate whether File Connections should
// play through all system messages and get to the first user message
// when opened or reset to the beginning.  This defaults to "true". 
// User code should set this
// to "false" before calling vrpn_get_connection_by_name() or creating
// a new vrpn_File_Connection object if it wants that file connection
// to not preload.  The value is only checked at connection creation time;
// the connection behaves consistently once created.  Leaving this true
// can help with offsets in time that happen at the beginning of files.

extern VRPN_API bool vrpn_FILE_CONNECTIONS_SHOULD_SKIP_TO_USER_MESSAGES;

class VRPN_API vrpn_File_Connection : public vrpn_Connection
{
public:
    vrpn_File_Connection (const char * station_name, 
                         const char * local_in_logfile_name = NULL,
                         const char * local_out_logfile_name = NULL);
    virtual ~vrpn_File_Connection (void);
    
    virtual int mainloop (const timeval * timeout = NULL);

    // returns the elapsed time in the file
    virtual int time_since_connection_open (timeval * elapsed_time);

	// returns the current time in the file since the epoch (UTC time).
	virtual timeval get_time( ) {  return d_time;  }

    virtual vrpn_File_Connection * get_File_Connection (void);

    // Pretend to send pending report, really just clear the buffer.
    virtual int     send_pending_reports (void);

    // {{{ fileconnections-specific methods (playback control)
public:
    // XXX the following should not be public if we want vrpn_File_Connection
    //     to have the same interface as vrpn_Connection
    //
    //     If so handler functions for messages for these operations 
    //     should be made, and functions added to vrpn_File_Controller which
    //     generate the messages.  This seemed like it would be messy
    //     since most of these functions have return values

    // rate of 0.0 is paused, 1.0 is normal speed
    void set_replay_rate(vrpn_float32 rate) {
        d_filetime_accum.set_replay_rate( rate );
    }

	vrpn_float32 get_replay_rate( )
	{  return d_filetime_accum.replay_rate( );  }
    
    // resets to the beginning of the file
	// returns 0 on success
    int reset (void);      

    // returns 1 if we're at the end of file
    int eof();

    // end_time for play_to_time() is an elapsed time
    // returns -1 on error or EOF, 0 on success
    int play_to_time (vrpn_float64 end_time);
    int play_to_time (timeval end_time);

    // end_filetime is an absolute time, corresponding to the
    // timestamps of the entries in the file,
    // returns -1 on error or EOF, 0 on success
    int play_to_filetime(const timeval end_filetime);

    // plays the next entry, returns -1 or error or EOF, 0 otherwise
    int playone();

    // plays at most one entry, but won't play past end_filetime
    // returns 0 on success, 1 if at end_filetime, -1 on error or EOF
    int playone_to_filetime(timeval end_filetime);

    // returns the elapsed time of the file
    timeval get_length();
    double get_length_secs();

    // returns the timestamp of the earliest in time user message
    timeval get_lowest_user_timestamp();

    // returns the timestamp of the greatest-in-time user message
    timeval get_highest_user_timestamp();

    // returns the name of the file
    const char *get_filename();

    // jump_to_time sets the current position to the given elapsed time
	// return 1 if we got to the specified time and 0 if we didn't
    int jump_to_time(vrpn_float64 newtime);
    int jump_to_time(timeval newtime);

    // jump_to_filetime sets the current position to the given absolute time
    // return 1 if we got to the specified time and 0 if we didn't
    int jump_to_filetime( timeval absolute_time );

    // Not very useful.
    // Limits the number of messages played out on any one call to mainloop.
    // 0 => no limit.
    void limit_messages_played_back (vrpn_uint32 max_playback) {
      Jane_stop_this_crazy_thing(max_playback);\
    };

    // }}}
    // {{{ tokens for VRPN control messages (data members)
protected:
    vrpn_int32 d_controllerId;

    vrpn_int32 d_set_replay_rate_type;
    vrpn_int32 d_reset_type;
    vrpn_int32 d_play_to_time_type;
    //long d_jump_to_time_type;

    // }}}
    // {{{ time-keeping
protected:
    timeval d_time;  // current time in file
    timeval d_start_time;  // time of first record in file
    timeval d_earliest_user_time;  // time of first user message
    vrpn_bool d_earliest_user_time_valid;
    timeval d_highest_user_time;  // time of last user message
    vrpn_bool d_highest_user_time_valid;

    // finds the timestamps of the earliest and highest-time user messages
	void find_superlative_user_times( );  
	
	// these are to be used internally when jumping around in the
	// stream (e.g., for finding the earliest and latest timed
	// user messages).  They assume 
	//   1) that only functions such as advance_currentLogEntry, 
	//      read_entry and manual traversal of d_logHead/d_logTail 
	//      will be used.
	// the functions return false if they don't save or restore the bookmark
	class VRPN_API vrpn_FileBookmark
	{
	public:
		vrpn_FileBookmark( );
		~vrpn_FileBookmark( );
		bool valid;
		timeval oldTime;
		long int file_pos;  // ftell result
		vrpn_LOGLIST* oldCurrentLogEntryPtr;  // just a pointer, useful for accum or preload
		vrpn_LOGLIST* oldCurrentLogEntryCopy;  // a deep copy, useful for no-accum, no-preload
	};
	bool store_stream_bookmark( );
	bool return_to_bookmark( );
	vrpn_FileBookmark d_bookmark;

    // wallclock time at the (beginning of the) last call
    // to mainloop that played back an event
    timeval d_last_time;  // XXX remove

    class VRPN_API FileTime_Accumulator
    {
        // accumulates the amount of time that we will advance
        // filetime by when we next play back messages.
        timeval d_filetime_accum_since_last_playback;  
        
        // wallclock time when d_filetime_accum_since_last_playback
        // was last updated
        timeval d_time_of_last_accum;
        
        // scale factor between stream time and wallclock time
        vrpn_float32 d_replay_rate;

    public:
        FileTime_Accumulator();
        
        // return accumulated time since last reset
        const timeval & accumulated (void) {
            return d_filetime_accum_since_last_playback;
        }

        // return last time accumulate_to was called
        const timeval & time_of_last_accum (void) {
            return d_time_of_last_accum;
        }

        vrpn_float32 replay_rate (void) { return d_replay_rate; }
        
        // add (d_replay_rate * (now_time - d_time_of_last_accum))
        // to d_filetime_accum_since_last_playback
        // then set d_time_of_last_accum to now_time
        void accumulate_to (const timeval & now_time);

        // if current rate is non-zero, then time is accumulated
        // before d_replay_rate is set to new_rate
        void set_replay_rate (vrpn_float32 new_rate);
        
        // set d_time_of_last_accum to now_time
        // and set d_filetime_accum_since_last_playback to zero
        void reset_at_time (const timeval & now_time);
    };
    FileTime_Accumulator d_filetime_accum;
    
    // }}}
    // {{{ actual mechanics of the logfile
protected:
    char *d_fileName;
    FILE * d_file;

    void play_to_user_message();

    // helper function for mainloop()
    int need_to_play(timeval filetime);

    // checks the cookie at
    // the head of the log file;
    //  exit on error!
    virtual int read_cookie (void);  

    virtual int read_entry (void);  // appends entry to d_logTail
      // returns 0 on success, 1 on EOF, -1 on error

    // Steps the currentLogEntry pointer forward one.
    // It handles both cases of preload and non-preload.
    // returns 0 on success, 1 on EOF, -1 on error
    virtual int advance_currentLogEntry(void);

    virtual int close_file (void);

    // }}}
    // {{{ handlers for VRPN control messages that might come from
    //     a File Controller object that wants to control this
    //     File Connection.
protected:
    static int VRPN_CALLBACK handle_set_replay_rate (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_reset (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_play_to_time (void *, vrpn_HANDLERPARAM);

    // }}}
    // {{{ Maintains a doubly-linked list structure that keeps
    //     copies of the messages from the file in memory.  If
    //     d_accumulate is false, then there is only ever one entry
    //     in memory (d_currentLogEntry == d_logHead == d_logTail).
    //     If d_preload is true, then all of the records from the file
    //     are read into the list in the constructor and we merely step
    //     through memory when playing the streamfile.  If d_preload is
    //     false and d_accumulate is true, then we have all of the
    //     records up the d_currentLogEntry in memory (d_logTail points
    //     to d_currentLogEntry but not to the last entry in the file
    //     until we get to the end of the file).
    //     The d_currentLogEntry should always be non-NULL unless we are
    //     past the end of all messages... we will either have preloaded
    //     all of them or else the read routine will attempt to load the
    //     next message each time one is played.  The constructor fills it
    //     in with the first message, which makes it non-NULL initially.
protected:
    vrpn_LOGLIST * d_logHead;  // the first read-in record
    vrpn_LOGLIST * d_logTail;  // the last read-in record
    vrpn_LOGLIST * d_currentLogEntry;  // Message that we've just loaded, or are at right now
    vrpn_LOGLIST * d_startEntry;  // potentially after initial system messages
    bool	   d_preload;	  // Should THIS File Connection pre-load?
    bool	   d_accumulate;  // Should THIS File Connection accumulate?
    // }}}
};


#endif  // VRPN_FILE_CONNECTION_H
