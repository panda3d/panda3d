#ifndef VRPN_FUNCTIONGENERATOR_H
#define VRPN_FUNCTIONGENERATOR_H

#include "vrpn_Shared.h"
#include "vrpn_Analog.h" // for vrpn_CHANNEL_MAX


const vrpn_uint32 vrpn_FUNCTION_CHANNELS_MAX = vrpn_CHANNEL_MAX;

extern const char* vrpn_FUNCTION_MESSAGE_TYPE_CHANNEL;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_CHANNEL_REQUEST;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_ALL_CHANNEL_REQUEST;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_SAMPLE_RATE;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_START;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_STOP;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_CHANNEL_REPLY;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_START_REPLY;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_STOP_REPLY;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_SAMPLE_RATE_REPLY;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_INTERPRETER_REQUEST;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_INTERPRETER_REPLY;
extern const char* vrpn_FUNCTION_MESSAGE_TYPE_ERROR;

class VRPN_API vrpn_FunctionGenerator_channel;

// a base class for all functions that vrpn_FunctionGenerator
// can generate
class VRPN_API vrpn_FunctionGenerator_function
{
public:
	virtual ~vrpn_FunctionGenerator_function() = 0;

	// concrete classes should implement this to generate the appropriate
	// values for the function the class represents.  nValue samples should be
	// generated beginning at time startTime, and these samples should be placed
	// in the provided buffer.  several data members of 'channel' can modify the
	// times for which values are generated.
	// returns the time of the last sample generated.
	virtual vrpn_float32 generateValues( vrpn_float32* buf, vrpn_uint32 nValues,
										 vrpn_float32 startTime, vrpn_float32 sampleRate,
										 vrpn_FunctionGenerator_channel* channel ) const = 0;

	// concrete classes should implement this to encode their 
	// function information into the specified buffer 'buf'.  The
	// remaining length in the buffer is stored in 'len'.  At return,
	// 'len' should be set to the number of characters remaining in the
	// buffer and the number of characters written should be returned,
	// save in case of failure, when negative should be returned.
	virtual vrpn_int32 encode_to( char** buf, vrpn_int32& len ) const = 0;

	// concrete classes should implement this to decode their
	// function information from the specified buffer.  The remaining
	// length in the buffer is stored in 'len'.  At return, 'len' should
	// be set to the number of characters remaining in the the buffer 
	// and the number of characters read should be returned, save in case
	// of failure, when negative should be returned
	virtual vrpn_int32 decode_from( const char** buf, vrpn_int32& len ) = 0;

	virtual vrpn_FunctionGenerator_function* clone( ) const = 0;

	// used when encoding/decoding to specify function type
	enum FunctionCode
	{
		FUNCTION_NULL = 0,
		FUNCTION_SCRIPT = 1
	};

	// concrete classes should implement this to return the
	// appropriate FunctionCode, from above
	virtual FunctionCode getFunctionCode( ) const = 0;


};


// the NULL function:  generate all zeros
class VRPN_API vrpn_FunctionGenerator_function_NULL
: public virtual vrpn_FunctionGenerator_function
{
public:
	vrpn_FunctionGenerator_function_NULL( ) { }
	virtual ~vrpn_FunctionGenerator_function_NULL( ) { }

	vrpn_float32 generateValues( vrpn_float32* buf, vrpn_uint32 nValues,
								vrpn_float32 startTime, vrpn_float32 sampleRate, 
								vrpn_FunctionGenerator_channel* channel ) const;

	vrpn_int32 encode_to( char** buf, vrpn_int32& len ) const;
	vrpn_int32 decode_from( const char** buf, vrpn_int32& len );
	vrpn_FunctionGenerator_function* clone( ) const;
protected:
	FunctionCode getFunctionCode( ) const {  return FUNCTION_NULL;  }

};


class VRPN_API vrpn_FunctionGenerator_function_script
: public virtual vrpn_FunctionGenerator_function
{
public:
	vrpn_FunctionGenerator_function_script( );
	vrpn_FunctionGenerator_function_script( const char* script );
	vrpn_FunctionGenerator_function_script( const vrpn_FunctionGenerator_function_script& );
	virtual ~vrpn_FunctionGenerator_function_script();

	virtual vrpn_float32 generateValues( vrpn_float32* buf, vrpn_uint32 nValues,
								vrpn_float32 startTime, vrpn_float32 sampleRate, 
								vrpn_FunctionGenerator_channel* channel ) const;

	vrpn_int32 encode_to( char** buf, vrpn_int32& len ) const;
	vrpn_int32 decode_from( const char** buf, vrpn_int32& len );
	vrpn_FunctionGenerator_function* clone( ) const;

	// returns a copy of the script.  caller is responsible for 
	// calling 'delete []' to free the returned string.
	char* getScript( ) const;

	const char* getConstScript( ) const
	{ return script; }

	vrpn_bool setScript( char* script );

protected:
	FunctionCode getFunctionCode( ) const {  return FUNCTION_SCRIPT;  }
	char* script;

};


class VRPN_API vrpn_FunctionGenerator_channel
{
	// note:  the channel will delete its function when the function is
	// no longer needed (e.g., when the channel is destroyed or the function changed)
public:
	vrpn_FunctionGenerator_channel( );
	vrpn_FunctionGenerator_channel( vrpn_FunctionGenerator_function* function );
	virtual ~vrpn_FunctionGenerator_channel( );

	const vrpn_FunctionGenerator_function* getFunction( ) const { return function; }
	void setFunction( vrpn_FunctionGenerator_function* function );

	// these return zero on success and negative on some failure.
	vrpn_int32 encode_to( char** buf, vrpn_int32& len ) const;
	vrpn_int32 decode_from( const char** buf, vrpn_int32& len );

protected:
	vrpn_FunctionGenerator_function* function;
	
};


class VRPN_API vrpn_FunctionGenerator : public vrpn_BaseClass
{
public:
	vrpn_FunctionGenerator( const char* name, vrpn_Connection* c = NULL );
	virtual ~vrpn_FunctionGenerator( );

	// returns the requested channel, or null if channelNum is 
	// greater than the maximum number of channels.
	const vrpn_FunctionGenerator_channel* const getChannel( vrpn_uint32 channelNum );

	vrpn_uint32 getNumChannels( ) const { return numChannels; }

	vrpn_float32 getSampleRate( )
	{  return sampleRate;  }

	enum FGError 
	{
		NO_FG_ERROR = 0,
		INTERPRETER_ERROR = 1, // the interpreter (for script) had some problem
		TAKING_TOO_LONG = 2, // samples were not generated quickly enough
		INVALID_RESULT_QUANTITY = 3, // an incorrect number of values was generated
		INVALID_RESULT_RANGE = 4 // generated values were out of range
	};

protected:
	vrpn_float32 sampleRate;  // samples per second
	vrpn_uint32 numChannels;
	vrpn_FunctionGenerator_channel* channels[vrpn_FUNCTION_CHANNELS_MAX];

	vrpn_int32 channelMessageID;             // id for channel message (remote -> server)
	vrpn_int32 requestChannelMessageID;	     // id for messages requesting channel info be sent (remote -> server)
	vrpn_int32 requestAllChannelsMessageID;  // id for messages requesting channel info of all channels be sent (remote -> server)
	vrpn_int32 sampleRateMessageID;		     // id for message to request a sampling rate (remote -> server)
	vrpn_int32 startFunctionMessageID;       // id for message to start generating the function (remote -> server)
	vrpn_int32 stopFunctionMessageID;        // id for message to stop generating the function (remote -> server)
	vrpn_int32 requestInterpreterMessageID;  // id for message to request interpreter description (remote -> server)

	vrpn_int32 channelReplyMessageID;        // id for reply for channel message (server -> remote)
	vrpn_int32 startFunctionReplyMessageID;  // id for reply to start-function message (server -> remote)
	vrpn_int32 stopFunctionReplyMessageID;   // id for reply to stop-function message (server -> remote)
	vrpn_int32 sampleRateReplyMessageID;     // id for reply to request-sample-rate message (server -> remote)
	vrpn_int32 interpreterReplyMessageID;    // id for reply to request-interpreter message (server -> remote)
	vrpn_int32 errorMessageID;				 // id for error reports

	vrpn_int32	gotConnectionMessageID;  // for new-connection message

	virtual int register_types( );

	char msgbuf[vrpn_CONNECTION_TCP_BUFLEN];
	struct timeval timestamp;
}; // end class vrpn_FunctionGenerator


class VRPN_API vrpn_FunctionGenerator_Server : public vrpn_FunctionGenerator
{
public:
	vrpn_FunctionGenerator_Server( const char* name, vrpn_uint32 numChannels = vrpn_FUNCTION_CHANNELS_MAX, vrpn_Connection* c = NULL );
	virtual ~vrpn_FunctionGenerator_Server( );

	virtual void mainloop( );

	// sub-classes should implement these functions.  they will be called when messages 
	// are received for the particular request.  at the end of these functions, servers 
	// should call the appropriate send*Reply function, even (especially!) if the requested 
	// change was rejected.
	virtual void setChannel( vrpn_uint32 channelNum, vrpn_FunctionGenerator_channel* channel ) = 0;
	virtual void start( ) = 0;
	virtual void stop( ) = 0;
	virtual void setSampleRate( vrpn_float32 rate ) = 0;

	vrpn_uint32 setNumChannels( vrpn_uint32 numChannels );

	// sub-classes should implement this function to provide a description of the type
	// of interpreter used to interpret vrpn_FunctionGenerator_function_script
	virtual const char* getInterpreterDescription( ) = 0;

	// sub-classes should not override these methods; these take care of
	// receiving requests
	static int VRPN_CALLBACK handle_channel_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_channelRequest_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_allChannelRequest_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_start_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_stop_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_sample_rate_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_interpreter_request_message( void* userdata, vrpn_HANDLERPARAM p );

protected:
	
	// sub-classes should call these functions to inform the remote side of
	// changes (or of non-changes, when a requested change cannot be accepted).
	// returns 0 on success and negative on failure.
	int sendChannelReply( vrpn_uint32 channelNum );
	int sendSampleRateReply( );
	int sendStartReply( vrpn_bool started );
	int sendStopReply( vrpn_bool stopped );
	int sendInterpreterDescription( );

	// sub-classes should use this function to report an error in function generation
	int sendError( FGError error, vrpn_int32 channel );

	vrpn_int32 decode_channel( const char* buf, const vrpn_int32 len, vrpn_uint32& channelNum,
								vrpn_FunctionGenerator_channel& channel );
	vrpn_int32 decode_channel_request( const char* buf, const vrpn_int32 len, vrpn_uint32& channelNum );
	vrpn_int32 decode_sampleRate_request( const char* buf, const vrpn_int32 len, vrpn_float32& sampleRate );

	vrpn_int32 encode_channel_reply( char** buf, vrpn_int32& len, const vrpn_uint32 channelNum );
	vrpn_int32 encode_start_reply( char** buf, vrpn_int32& len, const vrpn_bool isStarted );
	vrpn_int32 encode_stop_reply( char** buf, vrpn_int32& len, const vrpn_bool isStopped );
	vrpn_int32 encode_sampleRate_reply( char** buf, vrpn_int32& len, const vrpn_float32 sampleRate );
	vrpn_int32 encode_interpreterDescription_reply( char** buf, vrpn_int32& len, const char* desc );
	vrpn_int32 encode_error_report( char** buf, vrpn_int32& len, const FGError err, const vrpn_int32 channel );

}; // end class vrpn_FunctionGenerator_Server


//----------------------------------------------------------
// ************** Users deal with the following *************

// User routine to handle function-generator channel replies.  This
// is called when the function-generator server replies with new
// setting for some channel.
typedef	struct _vrpn_FUNCTION_CHANNEL_REPLY_CB
{
	struct timeval	msg_time;	// Time of the report
	vrpn_uint32	channelNum;		// Which channel is being reported
	vrpn_FunctionGenerator_channel*	channel;
} vrpn_FUNCTION_CHANNEL_REPLY_CB;
typedef void (VRPN_CALLBACK *vrpn_FUNCTION_CHANGE_REPLY_HANDLER)( void *userdata,
					  const vrpn_FUNCTION_CHANNEL_REPLY_CB info );

// User routine to handle function-generator start replies.  This
// is called when the function-generator server reports that it
// has started generating functions.
typedef	struct _vrpn_FUNCTION_START_REPLY_CB
{
	struct timeval	msg_time;	// Time of the report
	vrpn_bool isStarted;		// did the function generation start?
} vrpn_FUNCTION_START_REPLY_CB;
typedef void (VRPN_CALLBACK *vrpn_FUNCTION_START_REPLY_HANDLER)( void *userdata,
					     const vrpn_FUNCTION_START_REPLY_CB info );

// User routine to handle function-generator stop replies.  This
// is called when the function-generator server reports that it
// has stopped generating functions.
typedef	struct _vrpn_FUNCTION_STOP_REPLY_CB
{
	struct timeval	msg_time;	// Time of the report
	vrpn_bool isStopped;		// did the function generation stop?
} vrpn_FUNCTION_STOP_REPLY_CB;
typedef void (VRPN_CALLBACK *vrpn_FUNCTION_STOP_REPLY_HANDLER)( void *userdata,
					     const vrpn_FUNCTION_STOP_REPLY_CB info );

// User routine to handle function-generator sample-rate replies.  
// This is called when the function-generator server reports that 
// the function-generation sample rate has changed.
typedef	struct _vrpn_FUNCTION_SAMPLE_RATE_REPLY_CB
{
	struct timeval	msg_time;	// Time of the report
	vrpn_float32 sampleRate;		
} vrpn_FUNCTION_SAMPLE_RATE_REPLY_CB;
typedef void (VRPN_CALLBACK *vrpn_FUNCTION_SAMPLE_RATE_REPLY_HANDLER)( void *userdata,
					     const vrpn_FUNCTION_SAMPLE_RATE_REPLY_CB info );


// User routine to handle function-generator interpreter-description replies.  
// This is called when the function-generator server reports the description
// of its interpreter.
typedef	struct _vrpn_FUNCTION_INTERPRETER_REPLY_CB
{
	struct timeval	msg_time;	// Time of the report
	char* description;		
} vrpn_FUNCTION_INTERPRETER_REPLY_CB;
typedef void (VRPN_CALLBACK *vrpn_FUNCTION_INTERPRETER_REPLY_HANDLER)( void *userdata,
					     const vrpn_FUNCTION_INTERPRETER_REPLY_CB info );


// User routine to handle function-generator error notifications.  
// This is called when the function-generator server reports some
// error in the generation of a function.
typedef	struct _vrpn_FUNCTION_ERROR_CB
{
	struct timeval	msg_time;	// Time of the report
	vrpn_FunctionGenerator::FGError err;
	vrpn_int32 channel;
} vrpn_FUNCTION_ERROR_CB;
typedef void (VRPN_CALLBACK *vrpn_FUNCTION_ERROR_HANDLER)( void *userdata,
					     const vrpn_FUNCTION_ERROR_CB info );


class VRPN_API vrpn_FunctionGenerator_Remote : public vrpn_FunctionGenerator
{
public:
	vrpn_FunctionGenerator_Remote( const char* name, vrpn_Connection* c = NULL );
	virtual ~vrpn_FunctionGenerator_Remote( ) { }

	int setChannel( const vrpn_uint32 channelNum, const vrpn_FunctionGenerator_channel* channel );
	int requestChannel( const vrpn_uint32 channelNum );
	int requestAllChannels( );
	int requestStart( );
	int requestStop( );
	int requestSampleRate( const vrpn_float32 rate );
	int requestInterpreterDescription( );

	virtual void mainloop( );
	
	// (un)Register a callback handler to handle a channel reply
	virtual int register_channel_reply_handler( void *userdata,
		vrpn_FUNCTION_CHANGE_REPLY_HANDLER handler );
	virtual int unregister_channel_reply_handler( void *userdata,
		vrpn_FUNCTION_CHANGE_REPLY_HANDLER handler );
	
	// (un)Register a callback handler to handle a start reply
	virtual int register_start_reply_handler( void *userdata,
		vrpn_FUNCTION_START_REPLY_HANDLER handler );
	virtual int unregister_start_reply_handler( void *userdata,
		vrpn_FUNCTION_START_REPLY_HANDLER handler );
	
	// (un)Register a callback handler to handle a stop reply
	virtual int register_stop_reply_handler( void *userdata,
		vrpn_FUNCTION_STOP_REPLY_HANDLER handler );
	virtual int unregister_stop_reply_handler( void *userdata,
		vrpn_FUNCTION_STOP_REPLY_HANDLER handler );
	
	// (un)Register a callback handler to handle a sample-rate reply
	virtual int register_sample_rate_reply_handler( void *userdata,
		vrpn_FUNCTION_SAMPLE_RATE_REPLY_HANDLER handler );
	virtual int unregister_sample_rate_reply_handler( void *userdata,
		vrpn_FUNCTION_SAMPLE_RATE_REPLY_HANDLER handler );
	
	// (un)Register a callback handler to handle an interpreter message
	virtual int register_interpreter_reply_handler( void *userdata,
		vrpn_FUNCTION_INTERPRETER_REPLY_HANDLER handler );
	virtual int unregister_interpreter_reply_handler( void *userdata,
		vrpn_FUNCTION_INTERPRETER_REPLY_HANDLER handler );

	virtual int register_error_handler( void* userdata, 
		vrpn_FUNCTION_ERROR_HANDLER handler );
	virtual int unregister_error_handler( void* userdata,
		vrpn_FUNCTION_ERROR_HANDLER handler );
	
	static int VRPN_CALLBACK handle_channelReply_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_startReply_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_stopReply_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_sampleRateReply_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_interpreterReply_message( void* userdata, vrpn_HANDLERPARAM p );
	static int VRPN_CALLBACK handle_error_message( void* userdata, vrpn_HANDLERPARAM p );

protected:
	vrpn_Callback_List<vrpn_FUNCTION_CHANNEL_REPLY_CB> channel_reply_list;
	vrpn_Callback_List<vrpn_FUNCTION_START_REPLY_CB> start_reply_list;
	vrpn_Callback_List<vrpn_FUNCTION_STOP_REPLY_CB> stop_reply_list;
	vrpn_Callback_List<vrpn_FUNCTION_SAMPLE_RATE_REPLY_CB> sample_rate_reply_list;
	vrpn_Callback_List<vrpn_FUNCTION_INTERPRETER_REPLY_CB> interpreter_reply_list;
	vrpn_Callback_List<vrpn_FUNCTION_ERROR_CB> error_list;


	vrpn_int32 decode_channel_reply( const char* buf, const vrpn_int32 len, vrpn_uint32& channelNum );
	vrpn_int32 decode_start_reply( const char* buf, const vrpn_int32 len, vrpn_bool& isStarted );
	vrpn_int32 decode_stop_reply( const char* buf, const vrpn_int32 len, vrpn_bool& isStopped );
	vrpn_int32 decode_sampleRate_reply( const char* buf, const vrpn_int32 len );
	vrpn_int32 decode_interpreterDescription_reply( const char* buf, const vrpn_int32 len, char** desc );
	vrpn_int32 decode_error_reply( const char* buf, const vrpn_int32 len, FGError& error, vrpn_int32& channel );

	vrpn_int32 encode_channel( char** buf, vrpn_int32& len, const vrpn_uint32 channelNum, 
							   const vrpn_FunctionGenerator_channel* channel );
	vrpn_int32 encode_channel_request( char** buf, vrpn_int32& len, const vrpn_uint32 channelNum );
	vrpn_int32 encode_sampleRate_request( char** buf, vrpn_int32& len, const vrpn_float32 sampleRate );

}; // end class vrpn_FunctionGenerator_Remote


#endif // VRPN_FUNCTIONGENERATOR_H
