////////////////////////////////////////////////////////////////////
// Function name    : Http_BufferedReader::GetTermedString
// Description      :  a function that will peal a terminated string from the buffer
//  
// Return type      : inline bool 
// Argument         : char * outdata
// Argument         : int maxlen
// Argument         : char termchar1
// Argument         : char termchar2
////////////////////////////////////////////////////////////////////
inline bool Http_BufferedReader::GetTermedString(char * outdata, size_t maxlen,char termchar1, char termchar2)
{
    bool answer = false;
    size_t DataAvail = FastAmountBeffered();
    size_t MaxRead = maxlen;
    if(MaxRead > DataAvail)
        MaxRead = DataAvail;
    
    char     * wrk = FastGetMessageHead();
    for(size_t x=0; x< MaxRead; x++)
    {
        if(wrk[x] == termchar1 || wrk[x] == termchar2)
        {
            memcpy(outdata,wrk,x);
            outdata[x] = '\0';
            _StartPos += x+1;               
            Compress();
            answer = true;
            break;
        }           
    }
    return answer;
}
////////////////////////////////////////////////////////////////////
// Function name    : Http_BufferedReader::GetDoubleTermedString
// Description      : a function that will peal a terminated string from the buffer
//
//          This is the interface for a web request....
//  
// Return type      : inline bool 
// Argument         : char * outdata
// Argument         : int maxlen
// Argument         : char termchar1
// Argument         : char termchar2
////////////////////////////////////////////////////////////////////
inline bool Http_BufferedReader::GetDoubleTermedString(char * outdata, int maxlen,char termchar1, char termchar2)
{
    bool answer = false;
    size_t DataAvail = FastAmountBeffered();
    size_t MaxRead = maxlen;
    if(MaxRead > DataAvail)
        MaxRead = DataAvail;
    
    char     * wrk = FastGetMessageHead();
    for(size_t x=1; x< MaxRead; x++)
    {
        if(
            (wrk[x] == termchar1 && wrk[x-1] == termchar1) ||
            (wrk[x] == termchar2 && wrk[x-1] == termchar2) ||
            ( x >= 3 && wrk[x] == termchar1 && wrk[x-2] == termchar1 &&  wrk[x-1] == termchar2 && wrk[x-3] == termchar2 ) ||
            ( x >= 3 && wrk[x] == termchar2 && wrk[x-2] == termchar2 &&  wrk[x-1] == termchar1 && wrk[x-3] == termchar1 ) 
            )
        {
            memcpy(outdata,wrk,x);
            outdata[x] = '\0';
            _StartPos += x+1;               
            Compress();
            answer = true;
            break;
        }           
    }
    return answer;
}
////////////////////////////////////////////////////////////////////
// Function name    : Http_BufferedReader::GetTermedStringInPLace
// Description      :  Will peal a string inplace for reading
//  
// Return type      : inline bool 
// Argument         : char ** outdata
// Argument         : char termchars
////////////////////////////////////////////////////////////////////
inline bool Http_BufferedReader::GetTermedStringInPLace(char ** outdata,char termchars)
{
    bool answer = false;
    Compress();
    size_t MaxRead = FastAmountBeffered();
    char     * wrk = FastGetMessageHead();
    for(size_t x=0; x< MaxRead; x++)
    {
        if(wrk[x] == termchars)
        {               
            *outdata = wrk;     
            wrk[x] = '\0';
            _StartPos += x+1;               
            answer = true;
            break;
        }           
    }
    return answer;
}


////////////////////////////////////////////////////////////////////
// Function name    : Http_BufferedReader::GetTermedString
// Description      : do a read of a termed string not in place
//  
// Return type      : bool 
// Argument         : char * outdata
// Argument         : int maxlen
// Argument         : char * termchars
////////////////////////////////////////////////////////////////////
bool Http_BufferedReader::GetTermedString(char * outdata, int maxlen,char * termchars)
{
    bool answer = false;
    size_t DataAvail = FastAmountBeffered();
    size_t MaxRead = maxlen;
    if(MaxRead > DataAvail)
        MaxRead = DataAvail;
    int tstrsize = (int)strlen(termchars);
    
    char     * wrk = FastGetMessageHead();
    for(size_t x=0; x< MaxRead; x++)
    {
        if(memcmp(&wrk[x],termchars,tstrsize) == 0)
        {
            memcpy(outdata,wrk,x);
            outdata[x] = '\0';
            _StartPos += x+tstrsize;                
            Compress();
            answer = true;
            break;
        }           
    }
    return answer;
}
////////////////////////////////////////////////////////////////////
// Function name    : Http_BufferedReader::Http_BufferedReader
// Description      :  constructore .. passes size up to ring buffer
//  
// Return type      : inline 
// Argument         : int in_size
////////////////////////////////////////////////////////////////////
inline Http_BufferedReader::Http_BufferedReader(int in_size) : RingBuffer_Slide(in_size)
{   
    
}
////////////////////////////////////////////////////////////////////
// Function name    : Http_BufferedReader::ReSet
// Description      :  Reaset all read content.. IE zero's out buffer...
//  
//  If you lose framing this will not help
//
// Return type      : inline void 
// Argument         : void
////////////////////////////////////////////////////////////////////
inline void Http_BufferedReader::ReSet(void) 
{
    ResetContent();
}
////////////////////////////////////////////////////////////////////
// Function name    : Http_BufferedReader::PumpCRRead
// Description      :  a upcall function to read a CR object off buffer
//  
// Return type      : inline int 
// Argument         : char * data
// Argument         : int   maxdata
// Argument         : Socket_TCP &sck
////////////////////////////////////////////////////////////////////
inline int Http_BufferedReader::PumpCRRead(char * data, int   maxdata, Socket_TCP &sck)
{   
    if(GetTermedString(data,maxdata,'\r','\n') == true)
        return 1;
    
    int rp = ReadPump(sck);
    if(rp == 0)
        return 0;
    
    if(rp < 1)
        return -1;
    
    if(GetTermedString(data,maxdata,'\r','\n') == true)
        return 1;
    
    
    return 0;
}
////////////////////////////////////////////////////////////////////
// Function name    : Http_BufferedReader::PumpHTTPHeaderRead
// Description      :  Will read a HTTP head ,, GET ..... or response from web server
//  
// Return type      : inline int 
// Argument         : char * data
// Argument         : int   maxdata
// Argument         : Socket_TCP &sck
////////////////////////////////////////////////////////////////////
inline int Http_BufferedReader::PumpHTTPHeaderRead(char * data, int   maxdata, Socket_TCP &sck)
{
    
    if(GetDoubleTermedString(data,maxdata,'\r','\n') == true)
        return 1;
    
    
    int rp = ReadPump(sck);
    if(rp == 0)
        return 0;
    
    if(rp < 1)
        return -1;
    
    if(GetDoubleTermedString(data,maxdata,'\r','\n') == true)
        return 1;
    
    return 0;
}

inline int Http_BufferedReader::PumpSizeRead(StrTargetBuffer  & outdata,Socket_TCP &sck)
{
        if(GetSizeString(outdata) == true)
            return 1;   
    
        int rp = ReadPump(sck);
        if(rp == 0)
            return 0;
    
        if(rp < 1)
            return -1;
    
        if(GetSizeString(outdata) == true)
            return 1;
    
        return 0;
}

inline int Http_BufferedReader::PumpEofRead(StrTargetBuffer  & outdata,Socket_TCP &sck)
{   
    // do a quick read
    {       
        size_t MaxRead = FastAmountBeffered();
        if(MaxRead > 0)
        {
            char *ff = FastGetMessageHead();
            outdata.append(ff,MaxRead);
            _StartPos += MaxRead;               
            Compress();
        }
    }       
    
    // pump the reader  
    int rp = ReadPump(sck);
    if(rp == 0)
        return 0;
    
    if(rp== -1) // eof
    {
        // if eof clean the mess
        size_t MaxRead = FastAmountBeffered();
        if(MaxRead > 0)
        {
            char *ff = FastGetMessageHead();
            outdata.append(ff,MaxRead);
            _StartPos += MaxRead;               
            Compress();
        }
        return 1;
    }
        
    
    if(rp < 1)
        return -1;
    
    
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
inline bool Http_BufferedReader::GetSizeString(StrTargetBuffer  & outdata)
{
    size_t DataAvail = FastAmountBeffered();
    size_t MaxRead = outdata.left_to_fill();
    if(MaxRead > DataAvail)
        MaxRead = DataAvail;
    
    char     * wrk = FastGetMessageHead();   
    if(MaxRead > 0)
    {
    
        char *ff = FastGetMessageHead();
        outdata.append(ff,MaxRead);
        _StartPos += MaxRead;               
        return true;
    }   
    
    return false;   
};


