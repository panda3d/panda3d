////////////////////////////////////////////////////////////////////
// Function name	: Buffered_DatagramReader::GetMessageInplace
// Description	    :  A function that will peal a core message of the input buffer
//  
// Return type		: inline bool 
// Argument         : CoreMessage &inmsg
////////////////////////////////////////////////////////////////////
inline bool Buffered_DatagramReader::GetMessageFromBuffer(Datagram &inmsg)
{
	bool	answer = false;
	size_t DataAvail = FastAmountBeffered();	
	if(DataAvail >= sizeof(short))
	{
		char *ff = FastGetMessageHead();
		unsigned short len=GetUnsignedShort(ff); 
		len += sizeof(unsigned short);
		if(len <= DataAvail)
		{
			inmsg.assign(ff+2,len-2);
			_StartPos += len;				
			answer = true;
		}
	}
	return answer;
}
////////////////////////////////////////////////////////////////////
// Function name	: Buffered_DatagramReader::Buffered_DatagramReader
// Description	    :  constructore .. passes size up to ring buffer
//  
// Return type		: inline 
// Argument         : int in_size
////////////////////////////////////////////////////////////////////
inline Buffered_DatagramReader::Buffered_DatagramReader(int in_size) : RingBuffer(in_size)
{	
	
}
////////////////////////////////////////////////////////////////////
// Function name	: Buffered_DatagramReader::ReSet
// Description	    :  Reaset all read content.. IE zero's out buffer...
//  
//	If you lose framing this will not help
//
// Return type		: inline void 
// Argument         : void
////////////////////////////////////////////////////////////////////
inline void Buffered_DatagramReader::ReSet(void) 
{
	ResetContent();
}
